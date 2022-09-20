/*!
 * \file MarketDataCache.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "def/BQConstIF.hpp"
#include "def/BQDefIF.hpp"
#include "def/DefIF.hpp"
#include "util/PchBase.hpp"
#include "util/StdExt.hpp"

namespace bq {
struct Trades;
using TradesSPtr = std::shared_ptr<Trades>;
}  // namespace bq

namespace bq {

class RiskMgr;

class MarketDataCache {
 public:
  MarketDataCache(const MarketDataCache&) = delete;
  MarketDataCache& operator=(const MarketDataCache&) = delete;
  MarketDataCache(const MarketDataCache&&) = delete;
  MarketDataCache& operator=(const MarketDataCache&&) = delete;

  explicit MarketDataCache();

 public:
  void cache(const TradesSPtr& trades);
  TradesSPtr getLastTrades(const std::string& topic);
  TradesSPtr getLastTrades(MarketCode marketCode, SymbolType symbolType,
                           const std::string& symbolCode);

  std::string trades2Str() const;

 private:
  std::map<TopicHash, TradesSPtr> topic2LastTradesGroup_;
  std::ext::spin_mutex mtxTopic2LastTradesGroup_;

  std::uint64_t timesOfRecvTrades_{0};
};

using MarketDataCacheSPtr = std::shared_ptr<MarketDataCache>;

struct SymbolInfo;
using SymbolInfoSPtr = std::shared_ptr<SymbolInfo>;

std::tuple<int, std::vector<SymbolInfoSPtr>, std::uint64_t, Decimal> CalcPrice(
    const MarketDataCacheSPtr& marketDataCache, MarketCode marketCode,
    const std::string& baseCurrency, const std::string& quoteCurrency,
    const std::string& quoteCurrencyForCalc,
    const std::string& quoteCurrencyForConv);

}  // namespace bq
