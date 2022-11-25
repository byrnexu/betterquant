/*!
 * \file WSCliOfExchBinance.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "WSCliOfExchBinance.hpp"

#include "BooksCache.hpp"
#include "Config.hpp"
#include "MDSvc.hpp"
#include "MDSvcDef.hpp"
#include "MDSvcOfBinanceConst.hpp"
#include "MDSvcOfBinanceUtil.hpp"
#include "MDSvcUtil.hpp"
#include "SHMIPCConst.hpp"
#include "SHMIPCMsgId.hpp"
#include "SHMIPCUtil.hpp"
#include "SHMSrv.hpp"
#include "TopicGroupMustSubMaint.hpp"
#include "WSTask.hpp"
#include "db/TBLMonitorOfSymbolInfo.hpp"
#include "def/DataStruOfMD.hpp"
#include "def/MDWSCliAsyncTaskArg.hpp"
#include "util/Json.hpp"
#include "util/String.hpp"
#include "util/Util.hpp"

namespace bq::md::svc::binance {

WSCliOfExchBinance::WSCliOfExchBinance(MDSvc* mdSvc)
    : WSCliOfExch(mdSvc), booksCache_(std::make_shared<BooksCache>(mdSvc)) {}

void WSCliOfExchBinance::onBeforeOpen(
    web::WSCli* wsCli, const web::ConnMetadataSPtr& connMetadata) {
  booksCache_->reset();
}

WSCliAsyncTaskArgSPtr WSCliOfExchBinance::MakeWSCliAsyncTaskArg(
    const web::TaskFromSrvSPtr& task) const {
  auto ret = std::make_shared<WSCliAsyncTaskArg>();

  ret->doc_ = yyjson_read(task->msg_->get_payload().data(),
                          task->msg_->get_payload().size(), 0);
  ret->root_ = yyjson_doc_get_root(ret->doc_);

  const auto e = yyjson_obj_get(ret->root_, "e");
  if (yyjson_equals_str(e, EXCH_MD_TYPE_IN_QUOTE_OF_TRADES)) {
    ret->wsMsgType_ = WSMsgType::Trades;
  } else if (yyjson_equals_str(e, EXCH_MD_TYPE_IN_QUOTE_OF_BOOKS)) {
    ret->wsMsgType_ = WSMsgType::Books;
  } else if (yyjson_equals_str(e, EXCH_MD_TYPE_IN_QUOTE_OF_TICKERS)) {
    ret->wsMsgType_ = WSMsgType::Tickers;
  } else if (yyjson_equals_str(e, EXCH_MD_TYPE_IN_QUOTE_OF_CANDLE)) {
    ret->wsMsgType_ = WSMsgType::Candle;
  } else {
    ret->wsMsgType_ = WSMsgType::Others;
  }
  return ret;
}

/* trades
 *
 * {
 * "e": "aggTrade",
 * "E": 123456789,
 * "s": "BNBBTC",
 * "a": 12345,
 * "p": "0.001",
 * "q": "100",
 * "f": 100,
 * "l": 105,
 * "T": 123456785,
 * "m": true,
 * "M": true
 * }
 *
 */
std::string WSCliOfExchBinance::handleMDTrades(WSCliAsyncTaskSPtr& asyncTask) {
  auto arg = std::any_cast<WSCliAsyncTaskArgSPtr>(asyncTask->arg_);
  const auto& marketCode = mdSvc_->getMarketCode();
  const auto& symbolType = mdSvc_->getSymbolType();

  yyjson_val* vals = nullptr;
  yyjson_val* valExchTs = nullptr;
  yyjson_val* vala = nullptr;
  yyjson_val* valPrice = nullptr;
  yyjson_val* valSize = nullptr;
  yyjson_val* valf = nullptr;
  yyjson_val* vall = nullptr;
  yyjson_val* valTradeTs = nullptr;
  yyjson_val* valExchSide = nullptr;

  yyjson_val *valFieldName, *valFieldValue;
  yyjson_obj_iter iter;
  yyjson_obj_iter_init(arg->root_, &iter);
  while ((valFieldName = yyjson_obj_iter_next(&iter))) {
    if (yyjson_equals_str(valFieldName, "s")) {
      vals = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "E")) {
      valExchTs = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "a")) {
      vala = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "p")) {
      valPrice = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "q")) {
      valSize = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "f")) {
      valf = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "l")) {
      vall = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "T")) {
      valTradeTs = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "m")) {
      valExchSide = yyjson_obj_iter_get_val(valFieldName);
    }
  }

  std::string exchSymbolCode = yyjson_get_str(vals);
  boost::to_lower(exchSymbolCode);
  const auto [ret, symbolCode] =
      mdSvc_->getTBLMonitorOfSymbolInfo()->getSymbolCode(marketCode, symbolType,
                                                         exchSymbolCode);
  if (ret != 0) {
    const auto statusMsg = fmt::format(
        "Handle market data of trades failed because of "
        "get symbol code failed. [marketCode = {}, exchSymbolCode = {}]",
        marketCode, exchSymbolCode);
    LOG_W(statusMsg);
    return "";
  }

  const auto exchSide = yyjson_get_bool(valExchSide);
  const auto a = yyjson_get_uint(vala);
  const auto f = yyjson_get_uint(valf);
  const auto l = yyjson_get_uint(vall);

  const auto exchTs = yyjson_get_uint(valExchTs) * 1000;
  const auto tradeTs = yyjson_get_uint(valTradeTs);
  const auto price = yyjson_get_str(valPrice);
  const auto size = yyjson_get_str(valSize);

  const auto [topic, topicHash] =
      MakeTopicInfo(marketCode, symbolType, symbolCode, MDType::Trades);

  mdSvc_->getSHMSrv()->pushMsgWithZeroCopy(
      [&](void* shmBuf) {
        auto trades = static_cast<Trades*>(shmBuf);
        trades->shmHeader_.topicHash_ = topicHash;
        trades->mdHeader_.exchTs_ = exchTs;
        trades->mdHeader_.localTs_ = asyncTask->task_->localTs_;
        trades->mdHeader_.marketCode_ = mdSvc_->getMarketCodeEnum();
        trades->mdHeader_.symbolType_ = mdSvc_->getSymbolTypeEnum();
        strncpy(trades->mdHeader_.symbolCode_, symbolCode.c_str(),
                sizeof(trades->mdHeader_.symbolCode_) - 1);
        trades->mdHeader_.mdType_ = MDType::Trades;
        trades->tradeTs_ = tradeTs * 1000;
        snprintf(trades->tradeId_, sizeof(trades->tradeId_) - 1,
                 "%" PRIu64 "-%" PRIu64 "-%" PRIu64 "", a, f, l);
        trades->price_ = CONV(Decimal, price);
        trades->size_ = CONV(Decimal, size);
        trades->side_ = GetSide(exchSide);
        if (mdSvc_->saveMarketData()) {
          arg->topic_ = topic;
          arg->exchTs_ = exchTs;
          arg->marketDataOfUnifiedFmt_ = trades->dataOfUnifiedFmt();
        }
      },
      PUB_CHANNEL, MSG_ID_ON_MD_TRADES, sizeof(Trades));

#ifdef PERF_TEST
  EXEC_PERF_TEST("Trades", asyncTask->task_->localTs_, 10000, 100);
#endif
  return topic;
}

/* tickers
{
  "e": "24hrMiniTicker",
  "E": 123456789,
  "s": "BNBBTC",
  "c": "0.0025",
  "o": "0.0010",
  "h": "0.0025",
  "l": "0.0010",
  "v": "10000",
  "q": "18"
}
 */
std::string WSCliOfExchBinance::handleMDTickers(WSCliAsyncTaskSPtr& asyncTask) {
  auto arg = std::any_cast<WSCliAsyncTaskArgSPtr>(asyncTask->arg_);
  const auto& marketCode = mdSvc_->getMarketCode();
  const auto& symbolType = mdSvc_->getSymbolType();

  yyjson_val* vals = nullptr;
  yyjson_val* valExchTs = nullptr;
  yyjson_val* valLastPrice = nullptr;
  yyjson_val* valOpen24h = nullptr;
  yyjson_val* valHigh24h = nullptr;
  yyjson_val* valLow24h = nullptr;
  yyjson_val* valVol24h = nullptr;
  yyjson_val* valAmt24h = nullptr;

  yyjson_val *valFieldName, *valFieldValue;
  yyjson_obj_iter iter;
  yyjson_obj_iter_init(arg->root_, &iter);
  while ((valFieldName = yyjson_obj_iter_next(&iter))) {
    if (yyjson_equals_str(valFieldName, "s")) {
      vals = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "E")) {
      valExchTs = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "c")) {
      valLastPrice = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "o")) {
      valOpen24h = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "h")) {
      valHigh24h = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "l")) {
      valLow24h = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "v")) {
      valVol24h = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "q")) {
      valAmt24h = yyjson_obj_iter_get_val(valFieldName);
    }
  }

  std::string exchSymbolCode = yyjson_get_str(vals);
  boost::to_lower(exchSymbolCode);
  const auto [ret, symbolCode] =
      mdSvc_->getTBLMonitorOfSymbolInfo()->getSymbolCode(marketCode, symbolType,
                                                         exchSymbolCode);
  if (ret != 0) {
    const auto statusMsg = fmt::format(
        "Handle market data of tickers failed because of "
        "get symbol code failed. [marketCode = {}, exchSymbolCode = {}]",
        marketCode, exchSymbolCode);
    LOG_W(statusMsg);
    return "";
  }

  const auto exchTs = yyjson_get_uint(valExchTs) * 1000;
  const auto lastPrice = yyjson_get_str(valLastPrice);
  const auto open24h = yyjson_get_str(valOpen24h);
  const auto high24h = yyjson_get_str(valHigh24h);
  const auto low24h = yyjson_get_str(valLow24h);
  const auto vol24h = yyjson_get_str(valVol24h);
  const auto amt24h = yyjson_get_str(valAmt24h);

  const auto [topic, topicHash] =
      MakeTopicInfo(marketCode, symbolType, symbolCode, MDType::Tickers);

  mdSvc_->getSHMSrv()->pushMsgWithZeroCopy(
      [&](void* shmBuf) {
        auto tickers = static_cast<Tickers*>(shmBuf);
        tickers->shmHeader_.topicHash_ = topicHash;
        tickers->mdHeader_.exchTs_ = exchTs;
        tickers->mdHeader_.localTs_ = asyncTask->task_->localTs_;
        tickers->mdHeader_.marketCode_ = mdSvc_->getMarketCodeEnum();
        tickers->mdHeader_.symbolType_ = mdSvc_->getSymbolTypeEnum();
        strncpy(tickers->mdHeader_.symbolCode_, symbolCode.c_str(),
                sizeof(tickers->mdHeader_.symbolCode_) - 1);
        tickers->mdHeader_.mdType_ = MDType::Tickers;
        tickers->lastPrice_ = CONV(Decimal, lastPrice);
        tickers->open24h_ = CONV(Decimal, open24h);
        tickers->high24h_ = CONV(Decimal, high24h);
        tickers->low24h_ = CONV(Decimal, low24h);
        tickers->vol24h_ = CONV(Decimal, vol24h);
        tickers->amt24h_ = CONV(Decimal, amt24h);
        if (mdSvc_->saveMarketData()) {
          arg->topic_ = topic;
          arg->exchTs_ = exchTs;
          arg->marketDataOfUnifiedFmt_ = tickers->dataOfUnifiedFmt();
        }
      },
      PUB_CHANNEL, MSG_ID_ON_MD_TICKERS, sizeof(Tickers));

#ifdef PERF_TEST
  EXEC_PERF_TEST("Tickers", asyncTask->task_->localTs_, 10000, 100);
#endif
  return topic;
}

/*
 * candle
 *
 * {
 *  "e": "kline",
 *  "E": 123456789,
 *  "s": "BNBBTC",
 *  "k":
 *  {
 *  "t": 123400000,
 *  "T": 123460000,
 *  "s": "BNBBTC",
 *  "i": "1m",
 *  "f": 100,
 *  "L": 200,
 *  "o": "0.0010",
 *  "c": "0.0020",
 *  "h": "0.0025",
 *  "l": "0.0015",
 *  "v": "1000",
 *  "n": 100,
 *  "x": false,
 *  "q": "1.0000",
 *  "V": "500",
 *  "Q": "0.500",
 *  "B": "123456"
 *  }
 * }
 *
 */
std::string WSCliOfExchBinance::handleMDCandle(WSCliAsyncTaskSPtr& asyncTask) {
  auto arg = std::any_cast<WSCliAsyncTaskArgSPtr>(asyncTask->arg_);
  const auto& marketCode = mdSvc_->getMarketCode();
  const auto& symbolType = mdSvc_->getSymbolType();

  yyjson_val* valExchTs = yyjson_obj_get(arg->root_, "E");
  const auto exchTs = yyjson_get_uint(valExchTs) * 1000;

  yyjson_val* vals = nullptr;
  yyjson_val* valStartTs = nullptr;
  yyjson_val* valOpen = nullptr;
  yyjson_val* valClose = nullptr;
  yyjson_val* valHigh = nullptr;
  yyjson_val* valLow = nullptr;
  yyjson_val* valAmt = nullptr;
  yyjson_val* valVol = nullptr;

  const auto valk = yyjson_obj_get(arg->root_, "k");

  yyjson_val *valFieldName, *valFieldValue;
  yyjson_obj_iter iter;
  yyjson_obj_iter_init(valk, &iter);
  while ((valFieldName = yyjson_obj_iter_next(&iter))) {
    if (yyjson_equals_str(valFieldName, "s")) {
      vals = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "t")) {
      valStartTs = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "o")) {
      valOpen = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "c")) {
      valClose = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "h")) {
      valHigh = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "l")) {
      valLow = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "q")) {
      valAmt = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "v")) {
      valVol = yyjson_obj_iter_get_val(valFieldName);
    }
  }

  std::string exchSymbolCode = yyjson_get_str(vals);
  boost::to_lower(exchSymbolCode);
  const auto [ret, symbolCode] =
      mdSvc_->getTBLMonitorOfSymbolInfo()->getSymbolCode(marketCode, symbolType,
                                                         exchSymbolCode);
  if (ret) {
    const auto statusMsg = fmt::format(
        "Handle market data of candle failed because of "
        "get symbol code failed. [marketCode = {}, exchSymbolCode = {}]",
        marketCode, exchSymbolCode);
    LOG_W(statusMsg);
    return "";
  }

  const auto [topic, topicHash] =
      MakeTopicInfo(marketCode, symbolType, symbolCode, MDType::Candle);

  mdSvc_->getSHMSrv()->pushMsgWithZeroCopy(
      [&](void* shmBuf) {
        auto candle = static_cast<Candle*>(shmBuf);
        candle->shmHeader_.topicHash_ = topicHash;
        candle->mdHeader_.exchTs_ = exchTs;
        candle->mdHeader_.localTs_ = asyncTask->task_->localTs_;
        candle->mdHeader_.marketCode_ = mdSvc_->getMarketCodeEnum();
        candle->mdHeader_.symbolType_ = mdSvc_->getSymbolTypeEnum();
        strncpy(candle->mdHeader_.symbolCode_, symbolCode.c_str(),
                sizeof(candle->mdHeader_.symbolCode_) - 1);
        candle->mdHeader_.mdType_ = MDType::Candle;
        candle->startTs_ = yyjson_get_uint(valStartTs) * 1000;
        candle->open_ = CONV(Decimal, yyjson_get_str(valOpen));
        candle->high_ = CONV(Decimal, yyjson_get_str(valHigh));
        candle->low_ = CONV(Decimal, yyjson_get_str(valLow));
        candle->close_ = CONV(Decimal, yyjson_get_str(valClose));
        candle->vol_ = CONV(Decimal, yyjson_get_str(valVol));
        candle->amt_ = CONV(Decimal, yyjson_get_str(valAmt));
        if (mdSvc_->saveMarketData()) {
          arg->topic_ = topic;
          arg->exchTs_ = exchTs;
          arg->marketDataOfUnifiedFmt_ = candle->dataOfUnifiedFmt();
        }
      },
      PUB_CHANNEL, MSG_ID_ON_MD_CANDLE, sizeof(Candle));

#ifdef PERF_TEST
  EXEC_PERF_TEST("Candle", asyncTask->task_->localTs_, 10000, 100);
#endif
  return topic;
}

/*
{
  "e": "depthUpdate",
  "E": 123456789,
  "s": "BNBBTC",
  "U": 157,
  "u": 160,
  "b": [
    [
      "0.0024",
      "10"
    ]
  ],
  "a": [
    [
      "0.0026",
      "100"
    ]
  ]
}
*/
std::string WSCliOfExchBinance::handleMDBooks(WSCliAsyncTaskSPtr& asyncTask) {
  auto arg = std::any_cast<WSCliAsyncTaskArgSPtr>(asyncTask->arg_);
  const auto& marketCode = mdSvc_->getMarketCode();
  const auto& symbolType = mdSvc_->getSymbolType();

  const auto s = yyjson_obj_get(arg->root_, "s");
  std::string exchSymbolCode = yyjson_get_str(s);
  boost::to_lower(exchSymbolCode);
  const auto [ret, symbolCode] =
      mdSvc_->getTBLMonitorOfSymbolInfo()->getSymbolCode(marketCode, symbolType,
                                                         exchSymbolCode);
  if (ret != 0) {
    const auto statusMsg = fmt::format(
        "Handle market data of books failed because of "
        "get symbol code failed. [marketCode = {}, exchSymbolCode = {}]",
        marketCode, exchSymbolCode);
    LOG_W(statusMsg);
    return "";
  }

  auto [retOfHandle, snapshot] =
      booksCache_->handle(symbolCode, exchSymbolCode, arg->root_);
  if (retOfHandle != 0) {
    LOG_D("Handle market data of books snapshot for {} failed.", symbolCode);
    return "";
  }
  const auto valExchTs = yyjson_obj_get(arg->root_, "E");
  const auto exchTs = yyjson_get_uint(valExchTs) * 1000;

  const auto [topic, topicHash] =
      MakeTopicInfo(marketCode, symbolType, symbolCode, MDType::Books,
                    Int2StrInCompileTime<MAX_DEPTH_LEVEL>::type::value);

  mdSvc_->getSHMSrv()->pushMsgWithZeroCopy(
      [&](void* shmBuf) {
        auto books = static_cast<Books*>(shmBuf);
        books->shmHeader_.topicHash_ = topicHash;
        books->mdHeader_.exchTs_ = exchTs;
        books->mdHeader_.localTs_ = asyncTask->task_->localTs_;
        books->mdHeader_.marketCode_ = mdSvc_->getMarketCodeEnum();
        books->mdHeader_.symbolType_ = mdSvc_->getSymbolTypeEnum();
        strncpy(books->mdHeader_.symbolCode_, symbolCode.c_str(),
                sizeof(books->mdHeader_.symbolCode_) - 1);
        books->mdHeader_.mdType_ = MDType::Books;

        std::uint32_t asksLvl = 0;
        for (auto iter = std::begin(*snapshot->asks_);
             iter != std::end(*snapshot->asks_); ++iter, ++asksLvl) {
          if (asksLvl >= MAX_DEPTH_LEVEL) break;
          const auto& depthData = iter->second;
          books->asks_[asksLvl].price_ = depthData->price_;
          books->asks_[asksLvl].size_ = depthData->size_;
        }

        std::uint32_t bidsLvl = 0;
        for (auto iter = std::begin(*snapshot->bids_);
             iter != std::end(*snapshot->bids_); ++iter, ++bidsLvl) {
          if (bidsLvl >= MAX_DEPTH_LEVEL) break;
          const auto& depthData = iter->second;
          books->bids_[bidsLvl].price_ = depthData->price_;
          books->bids_[bidsLvl].size_ = depthData->size_;
        }
        if (mdSvc_->saveMarketData()) {
          arg->topic_ = topic;
          arg->exchTs_ = exchTs;
          arg->marketDataOfUnifiedFmt_ =
              books->dataOfUnifiedFmt(mdSvc_->getBooksDepthLevelOfSave());
        }
      },
      PUB_CHANNEL, MSG_ID_ON_MD_BOOKS, sizeof(Books));

#ifdef PERF_TEST
  EXEC_PERF_TEST("Books", asyncTask->task_->localTs_, 10000, 100);
#endif
  return topic;
}

/*
 * {
 *   "result": null,
 *   "id": 1
 * }
 *
 * {"code": 1, "msg": "Invalid value type: expected Boolean", "id": '%s'}
 *
 */
bool WSCliOfExchBinance::isSubOrUnSubRet(WSCliAsyncTaskSPtr& asyncTask) {
  const auto arg = std::any_cast<WSCliAsyncTaskArgSPtr>(asyncTask->arg_);
  const auto result = yyjson_obj_get(arg->root_, "result");
  const auto id = yyjson_obj_get(arg->root_, "id");
  if (result != nullptr && id != nullptr) {
    return true;
  }

  const auto code = yyjson_obj_get(arg->root_, "code");
  const auto msg = yyjson_obj_get(arg->root_, "msg");
  if (code != nullptr && msg != nullptr) {
    return true;
  }

  return false;
}

}  // namespace bq::md::svc::binance
