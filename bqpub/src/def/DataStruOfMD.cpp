#include "def/DataStruOfMD.hpp"

#include "util/Datetime.hpp"
#include "util/String.hpp"

namespace bq {

std::string MDHeader::toStr() const {
  const auto ret = fmt::format(
      "exchTs: {}; localTs_: {}; {} {} {} {}", ConvertTsToPtime(exchTs_),
      ConvertTsToPtime(localTs_), GetMarketName(marketCode_),
      magic_enum::enum_name(symbolType_), symbolCode_,
      magic_enum::enum_name(mdType_));
  return ret;
}

std::string MDHeader::getTopicPrefix() const {
  const auto ret = fmt::format("{}{}{}{}{}{}{}{}", TOPIC_PREFIX_OF_MARKET_DATA,
                               SEP_OF_TOPIC, GetMarketName(marketCode_),
                               SEP_OF_TOPIC, magic_enum::enum_name(symbolType_),
                               SEP_OF_TOPIC, symbolCode_, SEP_OF_TOPIC);
  return ret;
}

std::string Trades::toStr() const {
  const auto ret = fmt::format(
      "{} {} tradeTs: {}; tradeId: {}; "
      "price: {}; size: {}; side: {}; extDataLen: {}",
      shmHeader_.toStr(), mdHeader_.toStr(), tradeTs_, tradeId_, price_, size_,
      magic_enum::enum_name(side_), extDataLen_);
  return ret;
}

std::string Books::toStr() const {
  const auto ret =
      fmt::format("{} {} asks bids extDataLen: {}", shmHeader_.toStr(),
                  mdHeader_.toStr(), extDataLen_);
  return ret;
}

std::string Tickers::toStr() const {
  const auto ret = fmt::format(
      "{} {} lastPrice: {}; lastSize: {}; askPrice: {}; "
      "askSize: {}; bidPrice: {}; bidSize: {}; open24h: {}; "
      "high24h: {}; low24h: {}; vol24h: {}; amt24h: {}; extDataLen: {}",
      shmHeader_.toStr(), mdHeader_.toStr(), lastPrice_, lastSize_, askPrice_,
      askSize_, bidPrice_, bidSize_, open24h_, high24h_, low24h_, vol24h_,
      amt24h_, extDataLen_);
  return ret;
}

std::string Candle::toStr() const {
  const auto ret = fmt::format(
      "{} {} startTs: {}; open {}; high: {}; "
      "low: {}; close: {}; vol: {}; amt: {}; extDataLen: {}",
      shmHeader_.toStr(), mdHeader_.toStr(), ConvertTsToPtime(startTs_), open_,
      high_, low_, close_, vol_, amt_, extDataLen_);
  return ret;
}

}  // namespace bq
