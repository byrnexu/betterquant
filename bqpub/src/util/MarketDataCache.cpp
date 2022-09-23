/*!
 * \file MarketDataCache.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "util/MarketDataCache.hpp"

#include "SHMIPC.hpp"
#include "def/DataStruOfMD.hpp"
#include "def/StatusCode.hpp"
#include "def/SymbolInfoIF.hpp"
#include "util/Datetime.hpp"
#include "util/Float.hpp"
#include "util/Logger.hpp"

namespace bq {

MarketDataCache::MarketDataCache() {}

void MarketDataCache::cache(const TradesSPtr& trades) {
#ifndef NDEBUG
  LOG_T("Recv market data. {}", trades->toStr());
#endif
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxTopic2LastTradesGroup_);
    topic2LastTradesGroup_[trades->shmHeader_.topicHash_] = trades;
    if (++timesOfRecvTrades_ % 10000 == 0) {
      LOG_I("===== TRADES CACHE ===== {} \n{}", timesOfRecvTrades_,
            trades2Str());
    }
  }
}

TradesSPtr MarketDataCache::getLastTrades(const std::string& topic) {
  TradesSPtr ret;
  const auto topicHash = XXH3_64bits(topic.data(), topic.size());
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxTopic2LastTradesGroup_);
    const auto iter = topic2LastTradesGroup_.find(topicHash);
    if (iter != std::end(topic2LastTradesGroup_)) {
      ret = iter->second;
    }
  }
  return ret;
}

TradesSPtr MarketDataCache::getLastTrades(MarketCode marketCode,
                                          SymbolType symbolType,
                                          const std::string& symbolCode) {
  const auto topic =
      fmt::format("{}{}{}{}{}{}{}{}{}", TOPIC_PREFIX_OF_MARKET_DATA,
                  SEP_OF_TOPIC, magic_enum::enum_name(marketCode), SEP_OF_TOPIC,
                  magic_enum::enum_name(symbolType), SEP_OF_TOPIC, symbolCode,
                  SEP_OF_TOPIC, magic_enum::enum_name(MDType::Trades));
  return getLastTrades(topic);
}

std::string MarketDataCache::trades2Str() const {
  std::string ret;
  for (const auto& rec : topic2LastTradesGroup_) {
    ret = ret + rec.second->toStr() + "\n";
  }
  return ret;
}

std::tuple<int, std::vector<SymbolInfoSPtr>, std::uint64_t, Decimal> CalcPrice(
    const MarketDataCacheSPtr& marketDataCache, MarketCode marketCode,
    const std::string& baseCurrency, const std::string& quoteCurrency,
    const std::string& quoteCurrencyForCalc,
    const std::string& quoteCurrencyForConv) {

  const auto makeSymbolGroupForCalc = [&]() {
    const auto s1 = fmt::format("{}{}{}", quoteCurrencyForCalc,
                                SEP_OF_SYMBOL_SPOT, quoteCurrency);
    const auto s2 = fmt::format("{}{}{}", quoteCurrency, SEP_OF_SYMBOL_SPOT,
                                quoteCurrencyForCalc);
    const auto s3 = fmt::format("{}{}{}", quoteCurrencyForCalc,
                                SEP_OF_SYMBOL_SPOT, quoteCurrencyForConv);
    const auto s4 = fmt::format("{}{}{}", quoteCurrency, SEP_OF_SYMBOL_SPOT,
                                quoteCurrencyForConv);

    std::vector<SymbolInfoSPtr> ret;
    ret.emplace_back(
        std::make_shared<SymbolInfo>(marketCode, SymbolType::Spot, s1));
    ret.emplace_back(
        std::make_shared<SymbolInfo>(marketCode, SymbolType::Spot, s2));
    ret.emplace_back(
        std::make_shared<SymbolInfo>(marketCode, SymbolType::Spot, s3));
    ret.emplace_back(
        std::make_shared<SymbolInfo>(marketCode, SymbolType::Spot, s4));

    return ret;
  };

  std::vector<SymbolInfoSPtr> symbolGroupForCalc;
  if (quoteCurrencyForCalc == quoteCurrency) {
    return {SCODE_SUCCESS, symbolGroupForCalc, GetTotalUSSince1970(), 1.0};

  } else {
    const auto lastTrades = marketDataCache->getLastTrades(
        marketCode, SymbolType::Spot,
        fmt::format("{}{}{}", quoteCurrencyForCalc, SEP_OF_SYMBOL_SPOT,
                    quoteCurrency));
    if (lastTrades && !isApproximatelyZero(lastTrades->price_)) {
      return {SCODE_SUCCESS, symbolGroupForCalc, lastTrades->tradeTs_,
              lastTrades->price_};

    } else {
      const auto lastTrades = marketDataCache->getLastTrades(
          marketCode, SymbolType::Spot,
          fmt::format("{}{}{}", quoteCurrency, SEP_OF_SYMBOL_SPOT,
                      quoteCurrencyForCalc));
      if (lastTrades && !isApproximatelyZero(lastTrades->price_)) {
        return {SCODE_SUCCESS, symbolGroupForCalc, lastTrades->tradeTs_,
                1.0 / lastTrades->price_};
      }
    }
  }

  TradesSPtr lastTradesOfCalc = nullptr;
  if (quoteCurrencyForCalc == quoteCurrencyForConv) {
    lastTradesOfCalc = std::make_shared<Trades>();
    lastTradesOfCalc->tradeTs_ = GetTotalUSSince1970();
    lastTradesOfCalc->price_ = 1.0;
  } else {
    lastTradesOfCalc = marketDataCache->getLastTrades(
        marketCode, SymbolType::Spot,
        fmt::format("{}{}{}", quoteCurrencyForCalc, SEP_OF_SYMBOL_SPOT,
                    quoteCurrencyForConv));
  }

  TradesSPtr lastTrades = nullptr;
  if (quoteCurrency == quoteCurrencyForConv) {
    lastTrades = std::make_shared<Trades>();
    lastTrades->tradeTs_ = GetTotalUSSince1970();
    lastTrades->price_ = 1.0;
  } else {
    lastTrades = marketDataCache->getLastTrades(
        marketCode, SymbolType::Spot,
        fmt::format("{}{}{}", quoteCurrency, SEP_OF_SYMBOL_SPOT,
                    quoteCurrencyForConv));
  }

  if (lastTrades && lastTradesOfCalc &&
      !isApproximatelyZero(lastTrades->price_) &&
      !isApproximatelyZero(lastTradesOfCalc->price_)) {
    const auto lastPrice = lastTradesOfCalc->price_ / lastTrades->price_;
    const auto updateTime = lastTrades->tradeTs_ < lastTradesOfCalc->tradeTs_
                                ? lastTrades->tradeTs_
                                : lastTradesOfCalc->tradeTs_;
    return {SCODE_SUCCESS, symbolGroupForCalc, updateTime, lastPrice};
  }

  symbolGroupForCalc = makeSymbolGroupForCalc();
  return {SCODE_BQPUB_CALC_PRICE_FAILED, symbolGroupForCalc,
          GetTotalUSSince1970(), 1};
}

}  // namespace bq
