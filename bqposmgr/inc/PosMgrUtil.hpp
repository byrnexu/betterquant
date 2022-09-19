#pragma once

#include "def/BQConst.hpp"
#include "def/BQDef.hpp"
#include "util/Pch.hpp"

namespace bq {

Decimal calcPnlOfCloseShort(SymbolType symbolType, Decimal openPrice,
                            Decimal closePrice, Decimal closePos,
                            std::uint32_t parValue);

Decimal calcPnlOfCloseLong(SymbolType symbolType, Decimal openPrice,
                           Decimal closePrice, Decimal closePos,
                           std::uint32_t parValue);

}  // namespace bq
