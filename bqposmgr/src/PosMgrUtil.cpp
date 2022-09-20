/*!
 * \file PosMgrUtil.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "PosMgrUtil.hpp"

#include "util/Logger.hpp"

namespace bq {

Decimal calcPnlOfCloseShort(SymbolType symbolType, Decimal openPrice,
                            Decimal closePrice, Decimal closePos,
                            std::uint32_t parValue) {
  Decimal ret = 0;

  if (symbolType == SymbolType::Spot) {
    ret = closePos * (openPrice - closePrice);

  } else if (symbolType == SymbolType::Perp ||
             symbolType == SymbolType::Futures) {
    ret = closePos * (openPrice - closePrice);

  } else if (symbolType == SymbolType::CPerp ||
             symbolType == SymbolType::CFutures) {
    ret = static_cast<Decimal>(parValue) * closePos *
          (1.0 / closePrice - 1.0 / openPrice);

  } else {
    LOG_W("Unhandled symbolType {}.", magic_enum::enum_name(symbolType));
  }

  return ret;
}

Decimal calcPnlOfCloseLong(SymbolType symbolType, Decimal openPrice,
                           Decimal closePrice, Decimal closePos,
                           std::uint32_t parValue) {
  Decimal ret = 0;

  if (symbolType == SymbolType::Spot) {
    ret = closePos * (closePrice - openPrice);

  } else if (symbolType == SymbolType::Perp ||
             symbolType == SymbolType::Futures) {
    ret = closePos * (closePrice - openPrice);

  } else if (symbolType == SymbolType::CPerp ||
             symbolType == SymbolType::CFutures) {
    ret = static_cast<Decimal>(parValue) * closePos *
          (1.0 / openPrice - 1.0 / closePrice);

  } else {
    LOG_W("Unhandled symbolType {}.", magic_enum::enum_name(symbolType));
  }

  return ret;
}

}  // namespace bq
