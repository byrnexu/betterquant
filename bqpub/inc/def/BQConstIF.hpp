/*!
 * \file BQConstIF.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "util/PchBase.hpp"

namespace bq {

enum class MarketCode : std::uint16_t {
  Okex = 1,
  Binance = 2,
  Coinbase = 3,
  Ftx = 4,
  Kraken = 5,
  Others = UINT16_MAX
};
std::string GetMarketName(MarketCode marketCode);
MarketCode GetMarketCode(const std::string& marketCodeName);

enum class SymbolType : std::uint8_t {
  Spot = 1,
  Futures = 2,
  CFutures = 3,
  Perp = 4,
  CPerp = 5,
  Option = 6,
  index = 7,
  Others = UINT8_MAX
};

enum class Side : std::uint8_t { Bid = 1, Ask = 2, Others = UINT8_MAX };

enum class PosSide : std::uint8_t {
  Long = 1,
  Short = 2,
  Both = 3,
  Others = UINT8_MAX
};

enum class LiquidityDirection : std::uint8_t {
  Taker = 1,
  Maker = 2,
  Others = UINT8_MAX
};

enum class OrderType : std::uint8_t { Limit = 1, Others = UINT8_MAX };

enum class OrderTypeExtra : std::uint8_t {
  Normal = 1,
  MakeOnly = 2,
  Ioc = 3,
  Fok = 4,
  Others = UINT8_MAX
};

enum class MDType : std::uint8_t {
  Trades = 1,
  Books = 2,
  Tickers = 3,
  Candle = 4,
  Others = UINT8_MAX
};

enum class OrderStatus {
  Created = 1,
  ConfirmedInLocal = 3,
  Pending = 5,
  ConfirmedByExch = 10,
  PartialFilled = 20,
  Filled = 100,
  Canceled = 101,
  PartialFilledCanceled = 105,
  Failed = 110,
  Others = 127
};

enum class WSMsgType : std::uint8_t {
  Order = 1,

  SyncUnclosedOrder = 11,
  SyncAssetsSnapshot = 12,
  SyncAssetsUpdate = 13,

  Trades = 21,
  Books = 22,
  Tickers = 23,
  Candle = 24,

  SubRet = 41,
  UnSubRet = 42,

  Others = UINT8_MAX
};

enum class IsTheAssetInfoUpdated { True = 1, False = 2 };

constexpr static std::uint32_t MAX_DEPTH_LEVEL = 400;
constexpr static std::uint32_t DEFAULT_DEPTH_LEVEL = 20;

constexpr static std::uint16_t DEFAULT_PRODUCT_ID = 0xFFFF;
constexpr static std::uint32_t DEFAULT_ALGO_ID = 0xFFFFFFFF;
constexpr static std::uint64_t DEFAULT_PARENT_ORDER_ID = 0xFFFF;

constexpr static std::uint16_t MAX_SYMBOL_CODE_LEN = 32;
constexpr static std::uint16_t MAX_TRADE_ID_LEN = 32;
constexpr static std::uint16_t MAX_CURRENCY_LEN = 16;
constexpr static std::uint16_t MAX_ASSETS_NAME_LEN = 32;

const static std::string TOPIC_PREFIX_OF_MARKET_DATA = "MD";
const static std::string TOPIC_PREFIX_OF_TRADE_DATA = "TD";
const static std::string SEP_OF_TOPIC = "@";

const static std::string SEP_OF_SYMBOL_SPOT = "-";
const static std::string SEP_OF_SYMBOL_FUTURES = "-";
const static std::string SEP_OF_SYMBOL_PERP = "-";
const static std::string SEP_OF_REC_IDENTITY = "/";
const static std::string SEP_OF_GROUP_COND = "&";

const static std::string SYMBOL_STATE_PEROPEN = "preopen";
const static std::string SYMBOL_STATE_ONLINE = "online";
const static std::string SYMBOL_STATE_SUSPEND = "suspend";
const static std::string SYMBOL_STATE_SETTLEMENT = "settlement";

const static std::uint64_t DBL_TO_INT_MULTI = 1000000000000;
const static std::uint64_t DBL_PREC = 10;

const static std::string UNDEFINED_FIELD_MIN_DATETIME =
    "2000-01-01 00:00:00.000000";
const static std::string UNDEFINED_FIELD_MAX_DATETIME =
    "2030-01-01 00:00:00.000000";

const static std::uint64_t UNDEFINED_FIELD_MIN_TS = 946684800000000;
const static std::uint64_t UNDEFINED_FIELD_MAX_TS = 1893456000000000;

const static std::size_t MAX_TD_SRV_RISK_PLUGIN_NUM = 32;

const static std::string HIS_MD_FILE_EXT = "dat";
const static std::string SEP_OF_HIS_MD_DATA = ":";

const static std::string MIN_DATE_OF_HIS_MD = "20180101";
const static std::string SUB_DIR_OF_CANDLE_DETAIL = "detail";

}  // namespace bq
