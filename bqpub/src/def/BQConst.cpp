/*!
 * \file BQConst.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "def/BQConst.hpp"

namespace bq {

std::string GetMarketName(MarketCode marketCode) {
  switch (marketCode) {
    case MarketCode::Okex:
      return "Okex";
    case MarketCode::Binance:
      return "Binance";
    case MarketCode::Coinbase:
      return "Coinbase";
    case MarketCode::Ftx:
      return "Ftx";
    case MarketCode::Kraken:
      return "Kraken";
    default:
      return "Others";
      break;
  }
  return "Others";
}

MarketCode GetMarketCode(const std::string& marketCodeName) {
  if (marketCodeName == "Okex") {
    return MarketCode::Okex;
  } else if (marketCodeName == "Binance") {
    return MarketCode::Binance;
  } else if (marketCodeName == "Coinbase") {
    return MarketCode::Coinbase;
  } else if (marketCodeName == "Ftx") {
    return MarketCode::Ftx;
  } else if (marketCodeName == "Kraken") {
    return MarketCode::Kraken;
  } else {
    return MarketCode::Others;
  }
  return MarketCode::Others;
}

}  // namespace bq
