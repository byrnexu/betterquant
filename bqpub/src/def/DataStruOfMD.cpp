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

#include "def/Def.hpp"
#include "util/BQUtil.hpp"
#include "util/Datetime.hpp"
#include "util/Float.hpp"
#include "util/Logger.hpp"
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
      "{} {} open: {}; high: {}; low: {}; "
      "close: {}; vol: {}; amt: {}; extDataLen: {}",
      shmHeader_.toStr(), mdHeader_.toStr(), open_, high_, low_, close_, vol_,
      amt_, extDataLen_);
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

void initMDHeader(SHMHeader& shmHeader, MDHeader& mdHeader, const Doc& doc) {
  mdHeader.exchTs_ = doc["mdHeader"]["exchTs"].GetUint64();
  mdHeader.localTs_ = doc["mdHeader"]["localTs"].GetUint64();

  const auto marketCode = doc["mdHeader"]["marketCode"].GetString();
  mdHeader.marketCode_ = magic_enum::enum_cast<MarketCode>(marketCode).value();

  const auto symbolType = doc["mdHeader"]["symbolType"].GetString();
  mdHeader.symbolType_ = magic_enum::enum_cast<SymbolType>(symbolType).value();

  const auto symbolCode = doc["mdHeader"]["symbolCode"].GetString();
  strncpy(mdHeader.symbolCode_, symbolCode, sizeof(mdHeader.symbolCode_) - 1);

  const auto mdType = doc["mdHeader"]["mdType"].GetString();
  mdHeader.mdType_ = magic_enum::enum_cast<MDType>(mdType).value();

  shmHeader.msgId_ = GetMsgIdByMDType(mdHeader.mdType_);

  std::string topic;
  TopicHash topicHash;
  if (mdHeader.mdType_ == MDType::Books) {
    std::tie(topic, topicHash) =
        MakeTopicInfo(marketCode, symbolType, symbolCode, mdHeader.mdType_,
                      Int2StrInCompileTime<MAX_DEPTH_LEVEL>::type::value);

  } else if (mdHeader.mdType_ == MDType::Candle) {
    std::tie(topic, topicHash) =
        MakeTopicInfo(marketCode, symbolType, symbolCode, mdHeader.mdType_,
                      SUFFIX_OF_CANDLE_DETAIL);

  } else {
    std::tie(topic, topicHash) =
        MakeTopicInfo(marketCode, symbolType, symbolCode, mdHeader.mdType_);
  }
  shmHeader.topicHash_ = topicHash;
  strncpy(shmHeader.topic_, topic.c_str(), sizeof(shmHeader.topic_) - 1);
}

/*
{
  "mdHeader": {
    "exchTs": 1669338603492000,
    "localTs": 1669338603626138,
    "marketCode": "Binance",
    "symbolType": "Spot",
    "symbolCode": "BTC-USDT",
    "mdType": "Trades"
  },
  "data": {
    "tradeTs": 1669338603491000,
    "tradeId": "1920957568-2242251309-22422513",
    "price": 16529.42,
    "size": 0.00312,
    "side": "Bid"
  }
}
*/
std::tuple<int, Trades> MakeTrades(const std::string& jsonStr) {
  Trades ret;

  Doc doc;
  if (doc.Parse(jsonStr.data()).HasParseError()) {
    LOG_W("Parse data failed. {0} [offset {1}] {2}",
          GetParseError_En(doc.GetParseError()), doc.GetErrorOffset(), jsonStr);
    return {-1, ret};
  }

  initMDHeader(ret.shmHeader_, ret.mdHeader_, doc);

  ret.tradeTs_ = doc["data"]["tradeTs"].GetUint64();
  strncpy(ret.tradeId_, doc["data"]["tradeId"].GetString(),
          sizeof(ret.tradeId_));
  ret.price_ = doc["data"]["price"].GetDouble();
  ret.size_ = doc["data"]["size"].GetDouble();
  const auto side = doc["data"]["side"].GetString();
  ret.side_ = magic_enum::enum_cast<Side>(side).value();

  return {0, ret};
}

/*
{
  "mdHeader": {
    "exchTs": 1669338601386000,
    "localTs": 1669338601888897,
    "marketCode": "Binance",
    "symbolType": "Spot",
    "symbolCode": "BTC-USDT",
    "mdType": "Books"
  },
  "data": {
    "asks": [{
      "price": 16527.85,
      "size": 0.0379,
      "orderNum": 0
    }, {
      "price": 16527.87,
      "size": 0.01513,
      "orderNum": 0
    }],
    "bids": [{
      "price": 16527.41,
      "size": 0.00065,
      "orderNum": 0
    }, {
      "price": 16527.4,
      "size": 0.006,
      "orderNum": 0
    }]
  }
}
*/
std::tuple<int, Books> MakeBooks(const std::string& jsonStr) {
  Books ret;

  Doc doc;
  if (doc.Parse(jsonStr.data()).HasParseError()) {
    LOG_W("Parse data failed. {0} [offset {1}] {2}",
          GetParseError_En(doc.GetParseError()), doc.GetErrorOffset(), jsonStr);
    return {-1, ret};
  }

  initMDHeader(ret.shmHeader_, ret.mdHeader_, doc);

  for (std::size_t i = 0; i < doc["data"]["asks"].Size(); ++i) {
    if (i == MAX_DEPTH_LEVEL) break;
    ret.asks_[i].price_ = doc["data"]["asks"][i]["price"].GetDouble();
    ret.asks_[i].size_ = doc["data"]["asks"][i]["size"].GetDouble();
    ret.asks_[i].orderNum_ = doc["data"]["asks"][i]["orderNum"].GetInt();
  }

  for (std::size_t i = 0; i < doc["data"]["bids"].Size(); ++i) {
    if (i == MAX_DEPTH_LEVEL) break;
    ret.bids_[i].price_ = doc["data"]["bids"][i]["price"].GetDouble();
    ret.bids_[i].size_ = doc["data"]["bids"][i]["size"].GetDouble();
    ret.bids_[i].orderNum_ = doc["data"]["bids"][i]["orderNum"].GetInt();
  }

  return {0, ret};
}

/*
{
  "mdHeader": {
    "exchTs": 1669339079941000,
    "localTs": 1669339080121968,
    "marketCode": "Binance",
    "symbolType": "Spot",
    "symbolCode": "BTC-USDT",
    "mdType": "Candle"
  },
  "data": {
    "open": 16541.34,
    "high": 16543.28,
    "low": 16538.99,
    "close": 16542.51,
    "vol": 41.77032,
    "amt": 690941.9306651
  }
}
*/
std::tuple<int, Candle> MakeCandle(const std::string& jsonStr) {
  Candle ret;

  Doc doc;
  if (doc.Parse(jsonStr.data()).HasParseError()) {
    LOG_W("Parse data failed. {0} [offset {1}] {2}",
          GetParseError_En(doc.GetParseError()), doc.GetErrorOffset(), jsonStr);
    return {-1, ret};
  }

  initMDHeader(ret.shmHeader_, ret.mdHeader_, doc);

  ret.open_ = doc["data"]["open"].GetDouble();
  ret.high_ = doc["data"]["high"].GetDouble();
  ret.low_ = doc["data"]["low"].GetDouble();
  ret.close_ = doc["data"]["close"].GetDouble();
  ret.vol_ = doc["data"]["vol"].GetDouble();
  ret.amt_ = doc["data"]["amt"].GetDouble();

  return {0, ret};
}

/*
{
  "mdHeader": {
    "exchTs": 1669338602578000,
    "localTs": 1669338602832317,
    "marketCode": "Binance",
    "symbolType": "Spot",
    "symbolCode": "BTC-USDT",
    "mdType": "Tickers"
  },
  "data": {
    "lastPrice": 16528.88,
    "lastSize": 0.0,
    "askPrice": 0.0,
    "askSize": 0.0,
    "bidPrice": 0.0,
    "bidSize": 0.0,
    "open24h": 16558.8,
    "high24h": 16812.63,
    "low24h": 16458.05,
    "vol24h": 208577.12905,
    "amt24h": 3464772623.884822
  }
}
*/
std::tuple<int, Tickers> MakeTickers(const std::string& jsonStr) {
  Tickers ret;

  Doc doc;
  if (doc.Parse(jsonStr.data()).HasParseError()) {
    LOG_W("Parse data failed. {0} [offset {1}] {2}",
          GetParseError_En(doc.GetParseError()), doc.GetErrorOffset(), jsonStr);
    return {-1, ret};
  }

  initMDHeader(ret.shmHeader_, ret.mdHeader_, doc);

  ret.lastPrice_ = doc["data"]["lastPrice"].GetDouble();
  ret.lastSize_ = doc["data"]["lastSize"].GetDouble();
  ret.askPrice_ = doc["data"]["askPrice"].GetDouble();
  ret.askSize_ = doc["data"]["askSize"].GetDouble();
  ret.bidPrice_ = doc["data"]["bidPrice"].GetDouble();
  ret.bidSize_ = doc["data"]["bidSize"].GetDouble();
  ret.open24h_ = doc["data"]["open24h"].GetDouble();
  ret.high24h_ = doc["data"]["high24h"].GetDouble();
  ret.low24h_ = doc["data"]["low24h"].GetDouble();
  ret.vol24h_ = doc["data"]["vol24h"].GetDouble();
  ret.amt24h_ = doc["data"]["amt24h"].GetDouble();

  return {0, ret};
}

MsgId GetMsgIdByMDType(MDType mdType) {
  switch (mdType) {
    case MDType::Trades:
      return MSG_ID_ON_MD_TRADES;
    case MDType::Books:
      return MSG_ID_ON_MD_BOOKS;
    case MDType::Tickers:
      return MSG_ID_ON_MD_TICKERS;
    case MDType::Candle:
      return MSG_ID_ON_MD_CANDLE;
    default:
      return 0;
  }
  return 0;
}

}  // namespace bq
