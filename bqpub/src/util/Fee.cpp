/*!
 * \file Fee.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/12/01
 *
 * \brief
 */

#include "util/Fee.hpp"

#include "def/DataStruOfTD.hpp"
#include "def/StatusCode.hpp"
#include "util/Float.hpp"
#include "util/Logger.hpp"

namespace bq {

Decimal calcFee(const OrderInfoSPTr& orderInfo, Decimal feeRatio,
                std::uint32_t parValue) {
  Decimal ret = 0;
  switch (orderInfo->symbolType_) {
    case SymbolType::Spot:
      ret = orderInfo->side_ == Side::Bid
                ? feeRatio * orderInfo->dealSize_
                : feeRatio * orderInfo->dealSize_ * orderInfo->avgDealPrice_;
      break;

    case SymbolType::Perp:
    case SymbolType::Futures:
      ret = feeRatio * orderInfo->dealSize_ * orderInfo->avgDealPrice_;
      break;

    case SymbolType::CPerp:
    case SymbolType::CFutures:
      if (!isApproximatelyZero(orderInfo->avgDealPrice_)) {
        ret = feeRatio * (orderInfo->dealSize_ * parValue) /
              orderInfo->avgDealPrice_;
      }
      break;
    default:
      break;
  }
  return ret >= 0 ? ret : ret * -1;
}

std::string getFeeCurrency(const OrderInfoSPTr& orderInfo,
                           const std::string& baseCurrency,
                           const std::string& quoteCurrency) {
  switch (orderInfo->symbolType_) {
    case SymbolType::Spot:
      return orderInfo->side_ == Side::Bid ? baseCurrency : quoteCurrency;
    case SymbolType::Perp:
    case SymbolType::Futures:
      return quoteCurrency;
    case SymbolType::CPerp:
    case SymbolType::CFutures:
      return baseCurrency;
    default:
      break;
  }
  return "";
}

}  // namespace bq
