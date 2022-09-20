/*!
 * \file MDSvcOfBinanceUtil.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "Config.hpp"
#include "MDSvcOfBinanceConst.hpp"
#include "db/TBLMonitorOfSymbolInfo.hpp"
#include "def/BQConst.hpp"
#include "def/Const.hpp"
#include "util/Logger.hpp"
#include "util/Pch.hpp"

namespace bq::md::svc::binance {

/*
 * topic
 *
 * MD@Binance@Spot@BTC-USDT@books
 * MD@Binance@Spot@BTC-USDT@books@5
 * MD@Binance@Spot@BTC-USDT@candle
 * MD@Binance@Spot@BTC-USDT@tickers
 * MD@Binance@Spot@BTC-USDT@trades
 */
inline std::tuple<int, std::string> GetExchMDType(
    const std::vector<std::string>& topicFieldGroup) {
  if (topicFieldGroup.size() < 5) {
    const auto statusMsg = fmt::format(
        "Get exch market data type failed "
        "because of invalid topic field num of {}.",
        boost::join(topicFieldGroup, SEP_OF_TOPIC));
    LOG_W(statusMsg);
    return {-1, ""};
  }

  const auto mdType = topicFieldGroup[4];
  if (mdType == ENUM_VALUE_TO_STR(MDType::Candle)) {
    const auto exchMDType = EXCH_MD_TYPE_CANDLE;
    return {0, exchMDType};

  } else if (mdType == ENUM_VALUE_TO_STR(MDType::Books)) {
    const auto exchMDType = CONFIG["defaultExchMDTypeBooks"].as<std::string>();
    return {0, exchMDType};

  } else if (mdType == ENUM_VALUE_TO_STR(MDType::Tickers)) {
    const auto exchMDType = EXCH_MD_TYPE_TICKERS;
    return {0, exchMDType};

  } else if (mdType == ENUM_VALUE_TO_STR(MDType::Trades)) {
    const auto exchMDType = EXCH_MD_TYPE_TRADES;
    return {0, exchMDType};

  } else {
    const auto statusMsg = fmt::format(
        "Get exch market data type failed because unknown format of topic {}.",
        boost::join(topicFieldGroup, SEP_OF_TOPIC));
    LOG_W(statusMsg);
    return {-1, ""};
  }
}

/*
 * topic
 *
 * MD@Binance@Spot@ADA-USDT@books@5
 * MD@Binance@Spot@ADA-USDT@candle
 * MD@Binance@Spot@ADA-USDT@tickers
 * MD@Binance@Spot@ADA-USDT@trades
 * MD@Binance@Spot@BTC-USDT@books@5
 * MD@Binance@Spot@BTC-USDT@candle
 * MD@Binance@Spot@BTC-USDT@tickers
 * MD@Binance@Spot@BTC-USDT@trades
 * MD@Binance@Spot@ETH-USDT@books@5
 * MD@Binance@Spot@ETH-USDT@candle
 * MD@Binance@Spot@ETH-USDT@tickers
 * MD@Binance@Spot@ETH-USDT@trades
 */
inline std::tuple<int, std::string> GetMDType(const std::string& exchMDType) {
  if (exchMDType == EXCH_MD_TYPE_CANDLE) {
    const auto mdType = ENUM_VALUE_TO_STR(MDType::Candle);
    return {0, mdType};

  } else if (boost::starts_with(exchMDType, EXCH_MD_TYPE_BOOKS_PREFIX)) {
    const auto mdType = ENUM_VALUE_TO_STR(MDType::Books);
    return {0, mdType};

  } else if (exchMDType == EXCH_MD_TYPE_TICKERS) {
    const auto mdType = ENUM_VALUE_TO_STR(MDType::Tickers);
    return {0, mdType};

  } else if (exchMDType == EXCH_MD_TYPE_TRADES) {
    const auto mdType = ENUM_VALUE_TO_STR(MDType::Trades);
    return {0, mdType};

  } else {
    const auto statusMsg =
        fmt::format("Recv unknown exch market data type {}.", exchMDType);
    LOG_W(statusMsg);
    return {-1, ""};
  }
}

inline std::tuple<int, std::string> GetMDTypeInQuote(
    const std::string& exchMDType) {
  if (exchMDType == EXCH_MD_TYPE_IN_QUOTE_OF_CANDLE) {
    const auto mdType = ENUM_VALUE_TO_STR(MDType::Candle);
    return {0, mdType};

  } else if (boost::starts_with(exchMDType, EXCH_MD_TYPE_IN_QUOTE_OF_BOOKS)) {
    const auto mdType = ENUM_VALUE_TO_STR(MDType::Books);
    return {0, mdType};

  } else if (exchMDType == EXCH_MD_TYPE_IN_QUOTE_OF_TICKERS) {
    const auto mdType = ENUM_VALUE_TO_STR(MDType::Tickers);
    return {0, mdType};

  } else if (exchMDType == EXCH_MD_TYPE_IN_QUOTE_OF_TRADES) {
    const auto mdType = ENUM_VALUE_TO_STR(MDType::Trades);
    return {0, mdType};

  } else {
    const auto statusMsg =
        fmt::format("Recv unknown exch market data type {}.", exchMDType);
    LOG_W(statusMsg);
    return {-1, ENUM_VALUE_TO_STR(MDType::Others)};
  }
}

inline Side GetSide(bool exchSide) {
  if (exchSide == true) {
    return Side::Ask;
  } else {
    return Side::Bid;
  }
}

}  // namespace bq::md::svc::binance
