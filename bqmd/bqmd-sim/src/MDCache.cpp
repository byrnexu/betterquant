/*!
 * \file MDCache.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/27
 *
 * \brief
 */

#include "MDCache.hpp"

#include "Config.hpp"
#include "MDSim.hpp"
#include "MDSimDef.hpp"
#include "def/DataStruOfMD.hpp"
#include "def/Def.hpp"
#include "def/StatusCode.hpp"
#include "util/BQMDHis.hpp"
#include "util/Datetime.hpp"
#include "util/Logger.hpp"
#include "util/MarketDataCond.hpp"

namespace bq::md {

int MDCache::start() {
  secOfCacheMD_ = CONFIG["secOfCacheMD"].as<std::uint32_t>(60);
  if (secOfCacheMD_ > MAX_SEC_OF_CACHE_MD) {
    LOG_W(
        "Cache a batch of his market data failed "
        "because of secOfCacheMD greater than {}.",
        MAX_SEC_OF_CACHE_MD);
    return -1;
  }

  int statusCode = 0;

  const auto playbackDateTimeStart =
      CONFIG["playbackDateTimeStart"].as<std::string>("");
  std::tie(statusCode, tsStart_) =
      ConvertISODatetimeToTs(playbackDateTimeStart);
  if (statusCode != 0) {
    LOG_W(
        "Cache a batch of his market data failed "
        "because of invalid playbackDateTimeStart {} in config.",
        playbackDateTimeStart);
    return -1;
  }

  const auto playbackDateTimeEnd =
      CONFIG["playbackDateTimeEnd"].as<std::string>("");
  std::tie(statusCode, tsEnd_) = ConvertISODatetimeToTs(playbackDateTimeEnd);
  if (statusCode != 0) {
    LOG_W(
        "Cache a batch of his market data failed "
        "because of invalid playbackDateTimeEnd {} in config.",
        playbackDateTimeEnd);
    return -1;
  }

  if (tsEnd_ <= tsStart_) {
    LOG_W(
        "Cache a batch of his market data failed because of invalid "
        "playbackDateTimeEnd {} <= playbackDateTimeStart {} in config.",
        playbackDateTimeEnd, playbackDateTimeStart);
    return -1;
  }

  tsStartOfCurCache_ = tsStart_;
  mdRootPath_ = CONFIG["mdRootPath"].as<std::string>("data");

  keepRunning_.store(true);
  threadCacheMDHis_ = std::make_unique<std::thread>([this]() { cacheMDHis(); });

  return 0;
}

void MDCache::stop() {
  keepRunning_.store(false);
  if (threadCacheMDHis_->joinable()) {
    threadCacheMDHis_->join();
  }
}

Ts2MarketDataOfSimGroupSPtr MDCache::pop() {
  Ts2MarketDataOfSimGroupSPtr ret{nullptr};
  {
    std::lock_guard<std::mutex> guard(mtxMDCache_);
    if (!mdCache_.empty()) {
      ret = mdCache_.front();
      mdCache_.pop_front();
    }
  }
  return ret;
}

void MDCache::cacheMDHis() {
  const auto intervalBetweenCacheCheck =
      CONFIG["milliSecIntervalBetweenCacheCheck"].as<std::uint32_t>(1);
  const auto numOfCacheMD = CONFIG["numOfCacheMD"].as<std::uint32_t>(5);

  while (keepRunning_.load()) {
    while (true) {
      if (tsStartOfCurCache_ >= tsEnd_) {
        LOG_I("Has been cached to the md of {} in config, stop caching.",
              ConvertTsToPtime(tsEnd_));
        keepRunning_.store(false);
        break;
      }

      std::size_t cacheSize = 0;
      {
        std::lock_guard<std::mutex> guard(mtxMDCache_);
        cacheSize = mdCache_.size();
      }

      if (cacheSize >= numOfCacheMD) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(intervalBetweenCacheCheck));
        break;
      }

      cache1BatchOfHisMD();
    }
  }
}

void MDCache::cache1BatchOfHisMD() {
  auto tsEnd = tsStartOfCurCache_ + secOfCacheMD_ * 1000 * 1000;
  if (tsEnd > tsEnd_) {
    tsEnd = tsEnd_;
  }

  Ts2MarketDataOfSimGroupSPtr mdCacheOfCurBatch =
      std::make_shared<Ts2MarketDataOfSimGroup>();

  const auto topicGroup = Config::get_const_instance().getTopicGroup();
  for (const auto& topic : topicGroup) {
    const auto maxNumOfHisMDCanBeQeuryEachTime = UINT32_MAX;
    const auto [statusCodeOfLoad, ts2HisMDGroup] = MDHis::LoadHisMDBetweenTs(
        mdRootPath_, topic, tsStartOfCurCache_, tsEnd, IndexType::ByLocalTs,
        maxNumOfHisMDCanBeQeuryEachTime);
    if (statusCodeOfLoad != 0) {
      LOG_W("Load {} his md of topic {} between {} - {} failed. [{} - {}]",
            ts2HisMDGroup->size(), topic, ConvertTsToPtime(tsStartOfCurCache_),
            ConvertTsToPtime(tsEnd), statusCodeOfLoad,
            GetStatusMsg(statusCodeOfLoad));
      continue;
    }
    LOG_D("Load {} his md of topic {} between {} - {} success. [{} - {}]",
          ts2HisMDGroup->size(), topic, ConvertTsToPtime(tsStartOfCurCache_),
          ConvertTsToPtime(tsEnd), statusCodeOfLoad,
          GetStatusMsg(statusCodeOfLoad));

    const auto [statusCodeOfGet, marketDataCond] =
        GetMarketDataCondFromTopic(topic);
    if (statusCodeOfGet != 0) {
      LOG_W("Load {} his md of topic {} between {} - {} failed. [{} - {}]",
            ts2HisMDGroup->size(), topic, ConvertTsToPtime(tsStartOfCurCache_),
            ConvertTsToPtime(tsEnd), statusCodeOfLoad,
            GetStatusMsg(statusCodeOfLoad));
      continue;
    }

    LOG_D("Make md cache of topic {} between {} - {} success. [{} - {}]", topic,
          ConvertTsToPtime(tsStartOfCurCache_), ConvertTsToPtime(tsEnd),
          statusCodeOfLoad, GetStatusMsg(statusCodeOfLoad));

    auto mdCacheOfCurTopic =
        makeMDCacheOfCurTopic(marketDataCond, ts2HisMDGroup);

    mdCacheOfCurBatch->merge(*mdCacheOfCurTopic);
  }

  calcDelayBetweenAdjacentMD(mdCacheOfCurBatch);

  LOG_I("Cache {} of market data between {} - {} success. ",
        mdCacheOfCurBatch->size(), ConvertTsToPtime(tsStartOfCurCache_),
        ConvertTsToPtime(tsEnd));

  if (!mdCacheOfCurBatch->empty()) {
    std::lock_guard<std::mutex> guard(mtxMDCache_);
    mdCache_.emplace_back(mdCacheOfCurBatch);
  }

  tsStartOfCurCache_ = tsEnd;
}

Ts2MarketDataOfSimGroupSPtr MDCache::makeMDCacheOfCurTopic(
    const MarketDataCondSPtr& marketDataCond,
    const Ts2HisMDGroupSPtr& ts2HisMDGroup) {
  auto ret = std::make_shared<Ts2MarketDataOfSimGroup>();

  for (const auto& rec : *ts2HisMDGroup) {
    switch (marketDataCond->mdType_) {
      case MDType::Trades: {
        auto marketDataOfSim = makeMarketDataOfTrades(rec.second);
        if (marketDataOfSim != nullptr)
          ret->emplace(marketDataOfSim->localTs_, marketDataOfSim);
      } break;

      case MDType::Candle: {
        auto marketDataOfSim = makeMarketDataOfCandle(rec.second);
        if (marketDataOfSim != nullptr)
          ret->emplace(marketDataOfSim->localTs_, marketDataOfSim);
      } break;

      case MDType::Books: {
        auto marketDataOfSim = makeMarketDataOfBooks(rec.second);
        if (marketDataOfSim != nullptr)
          ret->emplace(marketDataOfSim->localTs_, marketDataOfSim);
      } break;

      case MDType::Tickers: {
        auto marketDataOfSim = makeMarketDataOfTickers(rec.second);
        if (marketDataOfSim != nullptr)
          ret->emplace(marketDataOfSim->localTs_, marketDataOfSim);
      } break;

      default: {
        LOG_W("Unhandled market data type {}.",
              magic_enum::enum_name(marketDataCond->mdType_));
      } break;
    }
  }

  return ret;
}

MarketDataOfSimSPtr MDCache::makeMarketDataOfTrades(const std::string& line) {
  auto ret = std::make_shared<MarketDataOfSim>();
  const auto [statusCode, trades] = MakeTrades(line);
  if (statusCode != 0) {
    LOG_W("Make market data of sim trades by line failed. {}", line);
    return nullptr;
  }
  ret->localTs_ = trades.mdHeader_.localTs_;
  ret->data_ = malloc(sizeof(Trades));
  ret->dataLen_ = sizeof(Trades);
  memcpy(ret->data_, &trades, sizeof(Trades));
  return ret;
}

MarketDataOfSimSPtr MDCache::makeMarketDataOfBooks(const std::string& line) {
  auto ret = std::make_shared<MarketDataOfSim>();
  const auto [statusCode, books] = MakeBooks(line);
  if (statusCode != 0) {
    LOG_W("Make market data of sim books by line failed. {}", line);
    return nullptr;
  }
  ret->localTs_ = books.mdHeader_.localTs_;
  ret->data_ = malloc(sizeof(Books));
  ret->dataLen_ = sizeof(Books);
  memcpy(ret->data_, &books, sizeof(Books));
  return ret;
}

MarketDataOfSimSPtr MDCache::makeMarketDataOfCandle(const std::string& line) {
  auto ret = std::make_shared<MarketDataOfSim>();
  const auto [statusCode, candle] = MakeCandle(line);
  if (statusCode != 0) {
    LOG_W("Make market data of sim candle by line failed. {}", line);
    return nullptr;
  }
  ret->localTs_ = candle.mdHeader_.localTs_;
  ret->data_ = malloc(sizeof(Candle));
  ret->dataLen_ = sizeof(Candle);
  memcpy(ret->data_, &candle, sizeof(Candle));
  return ret;
}

MarketDataOfSimSPtr MDCache::makeMarketDataOfTickers(const std::string& line) {
  auto ret = std::make_shared<MarketDataOfSim>();
  const auto [statusCode, tickers] = MakeTickers(line);
  if (statusCode != 0) {
    LOG_W("Make market data of sim tickers by line failed. {}", line);
    return nullptr;
  }
  ret->localTs_ = tickers.mdHeader_.localTs_;
  ret->data_ = malloc(sizeof(Tickers));
  ret->dataLen_ = sizeof(Tickers);
  memcpy(ret->data_, &tickers, sizeof(Tickers));
  return ret;
}

void MDCache::calcDelayBetweenAdjacentMD(
    Ts2MarketDataOfSimGroupSPtr& mdCacheOfCurBatch) {
  for (auto iter = std::begin(*mdCacheOfCurBatch);
       iter != std::end(*mdCacheOfCurBatch); ++iter) {
    const auto localTs = iter->first;
    auto& marketDataOfSim = iter->second;
    auto nextIter = std::next(iter, 1);
    if (nextIter != std::end(*mdCacheOfCurBatch)) {
      marketDataOfSim->delay_ = nextIter->second->localTs_ - localTs;
    } else {
      marketDataOfSim->delay_ = 0;
    }
  }
}

}  // namespace bq::md
