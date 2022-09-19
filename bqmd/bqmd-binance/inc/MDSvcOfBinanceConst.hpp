#pragma once

#include "MDSvc.hpp"

namespace bq::md::svc::binance {

const static std::string EXCH_MD_TYPE_TRADES = "aggTrade";
const static std::string EXCH_MD_TYPE_BOOKS_PREFIX = "depth";
const static std::string EXCH_MD_TYPE_TICKERS = "miniTicker";
const static std::string EXCH_MD_TYPE_CANDLE = "kline_1m";

const static char* EXCH_MD_TYPE_IN_QUOTE_OF_TRADES = "aggTrade";
const static char* EXCH_MD_TYPE_IN_QUOTE_OF_BOOKS = "depthUpdate";
const static char* EXCH_MD_TYPE_IN_QUOTE_OF_TICKERS = "24hrMiniTicker";
const static char* EXCH_MD_TYPE_IN_QUOTE_OF_CANDLE = "kline";

const static std::string EXCH_OP_SUB = "SUBSCRIBE";
const static std::string EXCH_OP_UNSUB = "UNSUBSCRIBE";

const static std::string SEP_OF_EXCH_PARAMS = "@";

const static std::string SEP_OF_TRADE_ID = "-";

const static std::uint32_t MAX_BOOKS_CACHE_NUM_OF_EACH_SYM = 10000;

}  // namespace bq::md::svc::binance
