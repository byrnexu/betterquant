/*!
 * \file MarketDataCond.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/25
 *
 * \brief
 */

#include "util/MarketDataCond.hpp"

#include "def/StatusCode.hpp"
#include "util/String.hpp"

namespace bq {

// topic = MD@Binance@Spot@BTC-USDT@Books@20
std::tuple<int, MarketDataCondSPtr> GetMarketDataCondFromTopic(
    const std::string& topic) {
  std::vector<std::string> fieldGroup;
  boost::split(fieldGroup, topic, boost::is_any_of(SEP_OF_TOPIC));
  if (fieldGroup.size() < 5) {
    return {SCODE_BQPUB_INVALID_TOPIC, nullptr};
  }

  auto ret = std::make_shared<MarketDataCond>();
  const auto marketCode = magic_enum::enum_cast<MarketCode>(fieldGroup[1]);
  if (!marketCode.has_value()) {
    return {SCODE_BQPUB_INVALID_TOPIC, nullptr};
  }
  ret->marketCode_ = marketCode.value();

  const auto symbolType = magic_enum::enum_cast<SymbolType>(fieldGroup[2]);
  if (!symbolType.has_value()) {
    return {SCODE_BQPUB_INVALID_TOPIC, nullptr};
  }
  ret->symbolType_ = symbolType.value();

  ret->symbolCode_ = fieldGroup[3];

  const auto mdType = magic_enum::enum_cast<MDType>(fieldGroup[4]);
  if (!mdType.has_value()) {
    return {SCODE_BQPUB_INVALID_TOPIC, nullptr};
  }
  ret->mdType_ = mdType.value();

  if (fieldGroup.size() == 6) {
    ret->ext_ = fieldGroup[5];

    switch (ret->mdType_) {
      case MDType::Books:
        if (isNumber(ret->ext_) == false) {
          return {SCODE_BQPUB_INVALID_TOPIC, nullptr};
        }
        break;

      case MDType::Candle:
        if (ret->ext_ != "detail") {
          return {SCODE_BQPUB_INVALID_TOPIC, nullptr};
        }
        break;

      default:
        return {SCODE_BQPUB_INVALID_TOPIC, nullptr};
    }
  }

  return {0, ret};
}

}  // namespace bq
