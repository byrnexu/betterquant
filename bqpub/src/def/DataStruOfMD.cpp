/*!
 * \file DataStruOfMD.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "def/DataStruOfMD.hpp"

#include "util/Datetime.hpp"
#include "util/Float.hpp"
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

std::string MDHeader::toJson() const {
  rapidjson::StringBuffer strBuf;
  rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
  writer.StartObject();

  writer.Key("exchTs");
  writer.Uint64(exchTs_);

  writer.Key("localTs");
  writer.Uint64(localTs_);

  writer.Key("marketCode");
  writer.String(GetMarketName(marketCode_).data());

  writer.Key("symbolType");
  writer.String(std::string(magic_enum::enum_name(symbolType_)).data());

  writer.Key("symbolCode");
  writer.String(symbolCode_);

  writer.Key("mdType");
  writer.String(std::string(magic_enum::enum_name(mdType_)).data());

  writer.EndObject();

  const auto ret = strBuf.GetString();
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

std::string Trades::toJson() const {
  const auto ret = MakeMarketData(shmHeader_, mdHeader_, data());
  return ret;
}

std::string Trades::data() const {
  rapidjson::StringBuffer strBuf;
  rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);

  writer.StartObject();
  writer.Key("tradeTs");
  writer.Uint64(tradeTs_);
  writer.Key("tradeId");
  writer.String(tradeId_);
  writer.Key("price");
  writer.Double(price_);
  writer.Key("size");
  writer.Double(size_);
  writer.Key("side");
  writer.String(std::string(magic_enum::enum_name(side_)).data());
  writer.EndObject();

  return strBuf.GetString();
}

std::string Trades::dataOfUnifiedFmt() const {
  const auto ret =
      fmt::format(R"({{"mdHeader":{},"data":{}}})", mdHeader_.toJson(), data());
  return ret;
}

std::string Books::toStr() const {
  const auto ret =
      fmt::format("{} {} asks bids extDataLen: {}", shmHeader_.toStr(),
                  mdHeader_.toStr(), extDataLen_);
  return ret;
}

std::string Books::toJson(std::uint32_t level) const {
  const auto ret = MakeMarketData(shmHeader_, mdHeader_, data(level));
  return ret;
}

std::string Books::data(std::uint32_t level) const {
  rapidjson::StringBuffer strBuf;
  rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);

  writer.StartObject();

  writer.Key("asks");
  writer.StartArray();
  for (std::size_t i = 0; i < MAX_DEPTH_LEVEL; ++i) {
    if (i >= level) break;
    if (isApproximatelyZero(asks_[i].price_) &&
        isApproximatelyZero(asks_[i].size_) && asks_[i].orderNum_ == 0) {
      break;
    }
    writer.StartObject();
    writer.Key("price");
    writer.Double(asks_[i].price_);
    writer.Key("size");
    writer.Double(asks_[i].size_);
    writer.Key("orderNum");
    writer.Uint(asks_[i].orderNum_);
    writer.EndObject();
  }
  writer.EndArray();

  writer.Key("bids");
  writer.StartArray();
  for (std::size_t i = 0; i < MAX_DEPTH_LEVEL; ++i) {
    if (i >= level) break;
    if (isApproximatelyZero(bids_[i].price_) &&
        isApproximatelyZero(bids_[i].size_) && bids_[i].orderNum_ == 0) {
      break;
    }
    writer.StartObject();
    writer.Key("price");
    writer.Double(bids_[i].price_);
    writer.Key("size");
    writer.Double(bids_[i].size_);
    writer.Key("orderNum");
    writer.Uint(bids_[i].orderNum_);
    writer.EndObject();
  }
  writer.EndArray();

  writer.EndObject();

  return strBuf.GetString();
}

std::string Books::dataOfUnifiedFmt(std::uint32_t level) const {
  const auto ret = fmt::format(R"({{"mdHeader":{},"data":{}}})",
                               mdHeader_.toJson(), data(level));
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

std::string Tickers::toJson() const {
  const auto ret = MakeMarketData(shmHeader_, mdHeader_, data());
  return ret;
}

std::string Tickers::data() const {
  rapidjson::StringBuffer strBuf;
  rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);

  writer.StartObject();
  writer.Key("lastPrice");
  writer.Double(lastPrice_);
  writer.Key("lastSize");
  writer.Double(lastSize_);
  writer.Key("askPrice");
  writer.Double(askPrice_);
  writer.Key("askSize");
  writer.Double(askSize_);
  writer.Key("bidPrice");
  writer.Double(bidPrice_);
  writer.Key("bidSize");
  writer.Double(bidSize_);
  writer.Key("open24h");
  writer.Double(open24h_);
  writer.Key("high24h");
  writer.Double(high24h_);
  writer.Key("low24h");
  writer.Double(low24h_);
  writer.Key("vol24h");
  writer.Double(vol24h_);
  writer.Key("amt24h");
  writer.Double(amt24h_);
  writer.EndObject();

  return strBuf.GetString();
}

std::string Tickers::dataOfUnifiedFmt() const {
  const auto ret =
      fmt::format(R"({{"mdHeader":{},"data":{}}})", mdHeader_.toJson(), data());
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

std::string Candle::toJson() const {
  const auto ret = MakeMarketData(shmHeader_, mdHeader_, data());
  return ret;
}

std::string Candle::data() const {
  rapidjson::StringBuffer strBuf;
  rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);

  writer.StartObject();
  writer.Key("startTs");
  writer.Uint64(startTs_);
  writer.Key("open");
  writer.Double(open_);
  writer.Key("high");
  writer.Double(high_);
  writer.Key("low");
  writer.Double(low_);
  writer.Key("close");
  writer.Double(close_);
  writer.Key("vol");
  writer.Double(vol_);
  writer.Key("amt");
  writer.Double(amt_);
  writer.EndObject();

  return strBuf.GetString();
}

std::string Candle::dataOfUnifiedFmt() const {
  const auto ret =
      fmt::format(R"({{"mdHeader":{},"data":{}}})", mdHeader_.toJson(), data());
  return ret;
}

std::string MakeMarketData(const SHMHeader& shmHeader, const MDHeader& mdHeader,
                           const std::string& data) {
  std::string ret;
  ret = R"({"shmHeader":)" + shmHeader.toJson() + ",";
  ret = ret + R"("mdHeader":)" + mdHeader.toJson() + ",";
  ret = ret + R"("data":)" + data + "}";
  return ret;
}

}  // namespace bq
