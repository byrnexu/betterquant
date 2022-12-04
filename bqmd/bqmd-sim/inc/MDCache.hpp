/*!
 * \file MDCache.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/27
 *
 * \brief
 */

#pragma once

#include "def/BQConst.hpp"
#include "util/Pch.hpp"

namespace bq {
struct MarketDataCond;
using MarketDataCondSPtr = std::shared_ptr<MarketDataCond>;
}  // namespace bq

namespace bq::md {

struct MarketDataOfSim;
using MarketDataOfSimSPtr = std::shared_ptr<MarketDataOfSim>;

using Ts2MarketDataOfSimGroup =
    std::multimap<std::uint64_t, MarketDataOfSimSPtr>;
using Ts2MarketDataOfSimGroupSPtr = std::shared_ptr<Ts2MarketDataOfSimGroup>;

using Ts2HisMDGroup = std::multimap<std::uint64_t, std::string>;
using Ts2HisMDGroupSPtr = std::shared_ptr<Ts2HisMDGroup>;

class MDSim;

class MDCache;
using MDCacheSPtr = std::shared_ptr<MDCache>;

class MDCache {
 public:
  MDCache(const MDCache&) = delete;
  MDCache& operator=(const MDCache&) = delete;
  MDCache(const MDCache&&) = delete;
  MDCache& operator=(const MDCache&&) = delete;

  explicit MDCache(MDSim* mdSim) : mdSim_(mdSim) {}

 public:
  int start();
  void stop();

 public:
  Ts2MarketDataOfSimGroupSPtr pop();

 private:
  void cacheMDHis();
  void cache1BatchOfHisMD();

  Ts2MarketDataOfSimGroupSPtr makeMDCacheOfCurTopic(
      const MarketDataCondSPtr& marketDataCond,
      const Ts2HisMDGroupSPtr& ts2HisMDGroup);

  MarketDataOfSimSPtr makeMarketDataOfTrades(const std::string& line);
  MarketDataOfSimSPtr makeMarketDataOfBooks(const std::string& line);
  MarketDataOfSimSPtr makeMarketDataOfCandle(const std::string& line);
  MarketDataOfSimSPtr makeMarketDataOfTickers(const std::string& line);

  void calcDelayBetweenAdjacentMD(
      Ts2MarketDataOfSimGroupSPtr& mdCacheOfCurBatch);

 private:
  MDSim* mdSim_{nullptr};

  std::atomic_bool keepRunning_{false};
  std::unique_ptr<std::thread> threadCacheMDHis_{nullptr};

  std::deque<Ts2MarketDataOfSimGroupSPtr> mdCache_;
  mutable std::mutex mtxMDCache_;

  std::uint32_t secOfCacheMD_;

  std::uint64_t tsStart_;
  std::uint64_t tsEnd_;

  std::uint64_t tsStartOfCurCache_;

  std::string mdRootPath_;
};

}  // namespace bq::md
