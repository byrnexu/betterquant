#include "BooksCache.hpp"

#include "Config.hpp"
#include "MDSvc.hpp"
#include "MDSvcOfBinanceConst.hpp"
#include "def/BQConst.hpp"
#include "def/BQDef.hpp"
#include "def/StatusCode.hpp"
#include "util/Json.hpp"
#include "util/Logger.hpp"
#include "util/StdExt.hpp"
#include "util/String.hpp"
#include "util/Util.hpp"

namespace bq::md::svc::binance {

std::tuple<int, BooksDataSPtr> BooksCache::handle(
    const std::string& symbolCode, const std::string& exchSymbolCode,
    yyjson_val* root) {
  auto retOfCache = cacheUpdateData(symbolCode, root);
  if (retOfCache == SCODE_MD_SVC_UPDATE_DATA_DISCONTINUOUS) {
    removeSnapshotInOrderToRegenThem(symbolCode);
    LOG_W("Handle {} failed.", symbolCode);
    return {retOfCache, nullptr};
  }

  auto [retOfMerge, booksSnapshot] = mergeUpdateDataToSnapshot(symbolCode);
  if (retOfMerge == SCODE_MD_SVC_SNAPSHOT_NOT_EXISTS) {
    createSnapshot(symbolCode, exchSymbolCode);
    LOG_D("Handle {} failed.", symbolCode);
    return {retOfMerge, nullptr};

  } else if (retOfMerge == SCODE_MD_SVC_FINAL_UPDATE_ID_TOO_SMALL) {
    LOG_D("Handle {} failed, continue to recv update data.", symbolCode);
    return {retOfMerge, nullptr};

  } else if (retOfMerge == SCODE_MD_SVC_FIRST_UPDATE_ID_TOO_LARGE) {
    removeSnapshotInOrderToRegenThem(symbolCode);
    LOG_D("Handle {} failed, continue to recv snapshot data.", symbolCode);
    return {retOfMerge, nullptr};
  }

  return {0, booksSnapshot};
}

/*
 *
 * {
 *  "e": "depthUpdate",
 *  "E": 123456789,
 *  "s": "BNBBTC",
 *  "U": 157,
 *  "u": 160,
 *  "b":
 *  [
 *   ["0.0024", "10"]
 *  ],
 *  "a":
 *  [
 *   ["0.0026", "100"]
 *  ]
 * }
 *
 */
int BooksCache::cacheUpdateData(const std::string& symbolCode,
                                yyjson_val* root) {
  UpdateId2BooksDataSPtr updateId2BooksData;

  const auto fieldNameOfFirstUpdateId = getFieldNameOfFirstUpdateId();
  const auto fieldNameOfFinalUpdateId = "u";
  const auto booksData =
      makeBooksData(symbolCode, root, "a", "b",
                    fieldNameOfFirstUpdateId.c_str(), fieldNameOfFinalUpdateId);

  const auto iter =
      symbolCode2UpdateId2BooksDataOfUpdateData_->find(symbolCode);
  if (iter != std::end(*symbolCode2UpdateId2BooksDataOfUpdateData_)) {
    updateId2BooksData = iter->second;
  } else {
    updateId2BooksData = std::make_shared<UpdateId2BooksData>();
    (*symbolCode2UpdateId2BooksDataOfUpdateData_)[symbolCode] =
        updateId2BooksData;
  }

  if (!updateId2BooksData->empty()) {
    const auto& prevBooksData = std::rbegin(*updateId2BooksData)->second;
    if (prevBooksData->finalUpdateId_ + 1 != booksData->firstUpdateId_) {
      const auto statusMsg = fmt::format(
          "The updateId of {} is found to be discontinuous, resub required. "
          "[finalUpdateIdOfPrevBook = {}; firstUpdateIdOfCurBook = {}]",
          symbolCode, prevBooksData->finalUpdateId_, booksData->firstUpdateId_);
      LOG_W(statusMsg);
      updateId2BooksData->clear();
      updateId2BooksData->emplace(booksData->finalUpdateId_, booksData);
      return SCODE_MD_SVC_UPDATE_DATA_DISCONTINUOUS;
    }
  }
  updateId2BooksData->emplace(booksData->finalUpdateId_, booksData);
  LOG_T("===== {} Cache {}. [size = {}]", symbolCode, booksData->toShortStr(),
        updateId2BooksData->size());

  if (updateId2BooksData->size() % 1000 == 0) {
    LOG_W("Too many unprocessed update data. [num = {}]",
          updateId2BooksData->size());
  }

  if (updateId2BooksData->size() > MAX_BOOKS_CACHE_NUM_OF_EACH_SYM) {
    const auto eraseNum =
        updateId2BooksData->size() - MAX_BOOKS_CACHE_NUM_OF_EACH_SYM;
    const auto iter = std::next(std::begin(*updateId2BooksData), eraseNum);
    updateId2BooksData->erase(std::begin(*updateId2BooksData), iter);
  }

  return 0;
}

void BooksCache::removeSnapshotInOrderToRegenThem(
    const std::string& symbolCode) {
  symbolCode2BooksDataOfSnapshot_->erase(symbolCode);
}

std::tuple<int, BooksDataSPtr> BooksCache::mergeUpdateDataToSnapshot(
    const std::string& symbolCode) {
  auto printDebugInfo = [](const auto& symbolCode, auto finalUpdateIdInSnapshot,
                           const auto& updateId2BooksData) {
    const auto firstRec = *updateId2BooksData->begin();
    const auto finalRec = *updateId2BooksData->rbegin();
    LOG_T("===== {}: Try to merge UPD_DATA to SNAHSHOT", symbolCode);
    LOG_T("===== SNAPSHOT: {} ", finalUpdateIdInSnapshot);
    LOG_T("===== UPD_DATA: {} [size = {}]", firstRec.second->toShortStr(),
          updateId2BooksData->size());
    LOG_T("===== UPD_DATA: {} [size = {}]", finalRec.second->toShortStr(),
          updateId2BooksData->size());
  };

  auto [retOfGetSnapshot, booksSnapshot] = getSnapshot(symbolCode);
  if (retOfGetSnapshot != 0) {
    LOG_D("Merge update data of {} to snapshot failed.", symbolCode);
    return {retOfGetSnapshot, nullptr};
  }
  const auto finalUpdateIdInSnapshot = booksSnapshot->finalUpdateId_ + 1;

  auto iterUpdateId2BooksData =
      symbolCode2UpdateId2BooksDataOfUpdateData_->find(symbolCode);
  auto updateId2BooksData = iterUpdateId2BooksData->second;

#ifndef NDEBUG
  if (!updateId2BooksData->empty()) {
    printDebugInfo(symbolCode, finalUpdateIdInSnapshot, updateId2BooksData);
  }
#endif

  const auto maxFinalUpdateId =
      std::rbegin(*updateId2BooksData)->second->finalUpdateId_;
  if (maxFinalUpdateId < finalUpdateIdInSnapshot) {
    LOG_D(
        "Max final update id of update data in cache {} < {} "
        "is too small. [{}]",
        maxFinalUpdateId, finalUpdateIdInSnapshot, symbolCode);
    updateId2BooksData->clear();
    return {SCODE_MD_SVC_FINAL_UPDATE_ID_TOO_SMALL, nullptr};
  }

  const auto minFirstUpdateId =
      std::begin(*updateId2BooksData)->second->firstUpdateId_;
  if (minFirstUpdateId > finalUpdateIdInSnapshot) {
    LOG_W(
        "Min first update id of update data in cache {} > {} "
        "is too large. [{}]",
        minFirstUpdateId, finalUpdateIdInSnapshot, symbolCode);
    return {SCODE_MD_SVC_FIRST_UPDATE_ID_TOO_LARGE, nullptr};
  }

  for (auto iter = std::begin(*updateId2BooksData);
       iter != std::end(*updateId2BooksData); ++iter) {
    const auto& booksDataUpdate = iter->second;
    if (finalUpdateIdInSnapshot >= booksDataUpdate->firstUpdateId_ &&
        finalUpdateIdInSnapshot <= booksDataUpdate->finalUpdateId_) {
      booksSnapshot->merge(booksDataUpdate);
      LOG_T("===== {}: Merge {} to {}. ", symbolCode,
            booksDataUpdate->toShortStr(), finalUpdateIdInSnapshot);
    }
  }

  // Keep the last record for judging whether the update data is continuous
  updateId2BooksData->erase(std::begin(*updateId2BooksData),
                            std::prev(std::end(*updateId2BooksData), 1));

#ifndef NDEBUG
  static std::uint64_t times = 0;
  if (++times % 1000 == 0) {
    LOG_D("{} {} asks lvl size: {}; bids lvl size {}", times, symbolCode,
          booksSnapshot->asks_->size(), booksSnapshot->bids_->size());
  }
#endif

  return {0, booksSnapshot};
}

/*
 * {
 *  "lastUpdateId": 19071881594,
 *  "bids": [
 *    ["31603.76000000", "0.32990000"],
 *    ["31603.75000000", "0.00090000"],
 *    ["31603.63000000", "0.01221000"],
 *    ["31603.26000000", "0.00063000"],
 *    ["31600.53000000", "0.00278000"],
 *    ["31598.82000000", "0.47444000"],
 *    ["31598.34000000", "0.01961000"],
 *    ["31598.18000000", "0.85000000"],
 *    ["31596.82000000", "0.00195000"],
 *    ["31595.53000000", "0.15824000"]
 *  ],
 *  "asks": [
 *    ["31603.77000000", "3.22509000"],
 *    ["31605.12000000", "0.06935000"],
 *    ["31605.61000000", "0.85000000"],
 *    ["31605.85000000", "0.85000000"],
 *    ["31605.90000000", "0.31862000"],
 *    ["31605.98000000", "0.02969000"],
 *    ["31605.99000000", "0.00033000"],
 *    ["31606.74000000", "2.81988000"],
 *    ["31606.75000000", "0.30765000"],
 *    ["31607.03000000", "0.42719000"]
 *  ]
 * }
 */
int BooksCache::createSnapshot(const std::string& symbolCode,
                               const std::string& exchSymbolCode) {
  auto addrOfSnapshot = CONFIG["addrOfSnapshot"].as<std::string>();
  boost::replace_first(addrOfSnapshot, "symbolCode",
                       boost::to_upper_copy(exchSymbolCode));
  const auto timeoutOfQuerySnapshot =
      CONFIG["timeoutOfQuerySnapshot"].as<std::uint32_t>();

  LOG_D("Begin to query snapshot of {} from exch. ", symbolCode);
  cpr::Response rsp =
      cpr::Get(cpr::Url{addrOfSnapshot}, cpr::Timeout(timeoutOfQuerySnapshot));
  if (rsp.status_code != cpr::status::HTTP_OK) {
    const auto statusMsg =
        fmt::format("Query snapshot of {} from exch failed. [{}:{}] [{}]",
                    symbolCode, rsp.status_code, rsp.reason, rsp.url.str());
    LOG_W(statusMsg);
    return -1;
  }

  std::unique_ptr<yyjson_doc, AutoFreeYYDoc> doc(
      yyjson_read(rsp.text.data(), rsp.text.size(), 0));
  if (doc.get() == nullptr) {
    LOG_W("Create snapshot of {} failed. ", symbolCode);
    return -1;
  }

  yyjson_val* root = yyjson_doc_get_root(doc.get());
  if (root == nullptr) {
    LOG_W("Create snapshot of {} failed. ", symbolCode);
    return -1;
  }

  const auto fieldNameOfFirstUpdateId = "";
  const auto fieldNameOfFinalUpdateId = "lastUpdateId";
  const auto booksSnapshot =
      makeBooksData(symbolCode, root, "asks", "bids", fieldNameOfFirstUpdateId,
                    fieldNameOfFinalUpdateId);
  setSnapshot(symbolCode, booksSnapshot);

  LOG_D("Query snapshot of {} from exch success. [update id = {}]", symbolCode,
        booksSnapshot->finalUpdateId_);

  return 0;
}

std::string BooksCache::getFieldNameOfFirstUpdateId() const {
  if (mdSvc_->getSymbolType() == magic_enum::enum_name(SymbolType::Spot)) {
    return "U";
  } else {
    return "pu";
  }
};

BooksDataSPtr BooksCache::makeBooksData(const std::string& symbolCode,
                                        yyjson_val* root,
                                        const char* fieldNameOfAsk,
                                        const char* fieldNameOfBid,
                                        const char* fieldNameOfFirstUpdateId,
                                        const char* fieldNameOfFinalUpdateId) {
  auto asks = std::make_shared<Asks<Decimal>>();
  const auto arrayAsk = yyjson_obj_get(root, fieldNameOfAsk);
  yyjson_val* valAsk;
  yyjson_arr_iter iterAsk;
  yyjson_arr_iter_init(arrayAsk, &iterAsk);
  while ((valAsk = yyjson_arr_iter_next(&iterAsk))) {
    size_t idxAskField, maxAskField;
    yyjson_val* valAskField;
    Decimal price = 0;
    Decimal size = 0;
    yyjson_arr_foreach(valAsk, idxAskField, maxAskField, valAskField) {
      if (idxAskField == 0) {
        price = CONV(Decimal, yyjson_get_str(valAskField));
      } else if (idxAskField == 1) {
        size = CONV(Decimal, yyjson_get_str(valAskField));
      }
    }
    const auto depthData = std::make_shared<DepthData<Decimal>>(price, size);
    const std::uint64_t priceMult = price * DBL_TO_INT_MULTI;
    asks->emplace(priceMult, depthData);
  }

  auto bids = std::make_shared<Bids<Decimal>>();
  const auto arrayBid = yyjson_obj_get(root, fieldNameOfBid);
  yyjson_val* valBid;
  yyjson_arr_iter iterBid;
  yyjson_arr_iter_init(arrayBid, &iterBid);
  while ((valBid = yyjson_arr_iter_next(&iterBid))) {
    size_t idxBidField, maxBidField;
    yyjson_val* valBidField;
    Decimal price = 0;
    Decimal size = 0;
    yyjson_arr_foreach(valBid, idxBidField, maxBidField, valBidField) {
      if (idxBidField == 0) {
        price = CONV(Decimal, yyjson_get_str(valBidField));
      } else if (idxBidField == 1) {
        size = CONV(Decimal, yyjson_get_str(valBidField));
      }
    }
    const auto depthData = std::make_shared<DepthData<Decimal>>(price, size);
    const std::uint64_t priceMult = price * DBL_TO_INT_MULTI;
    bids->emplace(priceMult, depthData);
  }

  std::uint64_t firstUpdateId = 0;
  if (std::strcmp(fieldNameOfFirstUpdateId, "") != 0) {
    const auto valFirstUpdateId =
        yyjson_obj_get(root, fieldNameOfFirstUpdateId);
    firstUpdateId = yyjson_get_uint(valFirstUpdateId);
    if (mdSvc_->getSymbolType() != magic_enum::enum_name(SymbolType::Spot)) {
      firstUpdateId += 1;
    }
  }
  const auto valFinalUpdateId = yyjson_obj_get(root, fieldNameOfFinalUpdateId);
  const auto finalUpdateId = yyjson_get_uint(valFinalUpdateId);
  const auto ret = std::make_shared<BooksData>(symbolCode, asks, bids,
                                               firstUpdateId, finalUpdateId);
  return ret;
}

void BooksCache::setSnapshot(const std::string& symbolCode,
                             const BooksDataSPtr& booksData) {
  (*symbolCode2BooksDataOfSnapshot_)[symbolCode] = booksData;
}

std::tuple<int, BooksDataSPtr> BooksCache::getSnapshot(
    const std::string& symbolCode) {
  const auto iter = symbolCode2BooksDataOfSnapshot_->find(symbolCode);
  if (iter != std::end(*symbolCode2BooksDataOfSnapshot_)) {
    return {0, iter->second};
  }
  LOG_D("The snapshot of {} does not exist.", symbolCode);
  return {SCODE_MD_SVC_SNAPSHOT_NOT_EXISTS, nullptr};
}

void BooksCache::reset() {
  symbolCode2UpdateId2BooksDataOfUpdateData_->clear();
  symbolCode2BooksDataOfSnapshot_->clear();
}

}  // namespace bq::md::svc::binance
