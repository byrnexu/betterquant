#pragma once

#include "def/BQDef.hpp"
#include "def/BQMDDef.hpp"
#include "def/Def.hpp"
#include "util/Float.hpp"
#include "util/Pch.hpp"
#include "util/StdExt.hpp"

namespace bq::md::svc {
class MDSvc;
}

namespace bq::md::svc::binance {

using Price = std::uint64_t;
using UpdateId = std::uint64_t;

struct BooksData;
using BooksDataSPtr = std::shared_ptr<BooksData>;

struct BooksData {
  BooksData(const std::string symbolCode, const AsksSPtr<Decimal>& asks,
            const BidsSPtr<Decimal>& bids, UpdateId firstUpdateId,
            UpdateId finalUpdateId)
      : symbolCode_(symbolCode),
        asks_(asks),
        bids_(bids),
        firstUpdateId_(firstUpdateId),
        finalUpdateId_(finalUpdateId) {}

  std::string symbolCode_;
  AsksSPtr<Decimal> asks_{nullptr};
  BidsSPtr<Decimal> bids_{nullptr};
  UpdateId firstUpdateId_{0};
  UpdateId finalUpdateId_{0};

  std::string toShortStr() const {
    const auto ret =
        fmt::format("{} {} - {}", symbolCode_, firstUpdateId_, finalUpdateId_);
    return ret;
  }

  void merge(const BooksDataSPtr& booksDataUpdate) {
    assert(symbolCode_ == booksDataUpdate->symbolCode_ &&
           "symbolCode_ == booksDataUpdate->symbolCode_");

    firstUpdateId_ = booksDataUpdate->firstUpdateId_;
    finalUpdateId_ = booksDataUpdate->finalUpdateId_;

    for (const auto& rec : *booksDataUpdate->asks_) {
      const auto priceInUpdateData = rec.first;
      const auto& depthDataInUpdateData = rec.second;
      if (!isApproximatelyZero(depthDataInUpdateData->size_)) {
        (*asks_)[priceInUpdateData] = depthDataInUpdateData;
      } else {
        asks_->erase(priceInUpdateData);
      }
    }

    for (const auto& rec : *booksDataUpdate->bids_) {
      const auto priceInUpdateData = rec.first;
      const auto& depthDataInUpdateData = rec.second;
      if (!isApproximatelyZero(depthDataInUpdateData->size_)) {
        (*bids_)[priceInUpdateData] = depthDataInUpdateData;
      } else {
        bids_->erase(priceInUpdateData);
      }
    }
  }
};

using SymbolCode2BooksData = std::map<std::string, BooksDataSPtr>;
using SymbolCode2BooksDataSPtr = std::shared_ptr<SymbolCode2BooksData>;

using UpdateId2BooksData = std::multimap<UpdateId, BooksDataSPtr>;
using UpdateId2BooksDataSPtr = std::shared_ptr<UpdateId2BooksData>;

using SymbolCode2UpdateId2BooksData =
    std::map<std::string, UpdateId2BooksDataSPtr>;
using SymbolCode2UpdateId2BooksDataSPtr =
    std::shared_ptr<SymbolCode2UpdateId2BooksData>;

class BooksCache;
using BooksCacheSPtr = std::shared_ptr<BooksCache>;

class BooksCache {
 public:
  BooksCache(const BooksCache&) = delete;
  BooksCache& operator=(const BooksCache&) = delete;
  BooksCache(const BooksCache&&) = delete;
  BooksCache& operator=(const BooksCache&&) = delete;

  explicit BooksCache(MDSvc* mdSvc) : mdSvc_(mdSvc) {}

  std::tuple<int, BooksDataSPtr> handle(const std::string& symbolCode,
                                        const std::string& exchSymbolCode,
                                        yyjson_val* root);

 private:
  int cacheUpdateData(const std::string& symbolCode, yyjson_val* root);

  void removeSnapshotInOrderToRegenThem(const std::string& symbolCode);

  std::tuple<int, BooksDataSPtr> mergeUpdateDataToSnapshot(
      const std::string& symbolCode);

  int createSnapshot(const std::string& symbolCode,
                     const std::string& exchSymbolCode);

  std::string getFieldNameOfFirstUpdateId() const;

  BooksDataSPtr makeBooksData(const std::string& symbolCode, yyjson_val* root,
                              const char* fieldNameOfAsk,
                              const char* fieldNameOfBid,
                              const char* fieldNameOfFirstUpdateId,
                              const char* fieldNameOfFinalUpdateId);

  void setSnapshot(const std::string& symbolCode,
                   const BooksDataSPtr& booksData);
  std::tuple<int, BooksDataSPtr> getSnapshot(const std::string& symbolCode);

 public:
  void reset();

 private:
  MDSvc* mdSvc_;

  SymbolCode2UpdateId2BooksDataSPtr symbolCode2UpdateId2BooksDataOfUpdateData_{
      std::make_shared<SymbolCode2UpdateId2BooksData>()};
  SymbolCode2BooksDataSPtr symbolCode2BooksDataOfSnapshot_{
      std::make_shared<SymbolCode2BooksData>()};
};

}  // namespace bq::md::svc::binance
