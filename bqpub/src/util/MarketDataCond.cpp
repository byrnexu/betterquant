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

namespace bq {

// topic = MD@Binance@Spot@BTC-USDT@Books@20
std::tuple<int, MarketDataCondSPtr> getMarketDataCondFromTopic(
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

  if (ret->mdType_ == MDType::Books && fieldGroup.size() == 6) {
    const auto level = CONV_OPT(std::uint32_t, fieldGroup[5]);
    if (level == boost::none) {
      return {SCODE_BQPUB_INVALID_TOPIC, nullptr};
    }
    ret->level_ = level.value();
  }

  return {0, ret};
}

}  // namespace bq
