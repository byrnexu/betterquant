/*!
 * \file Fee.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/12/02
 *
 * \brief
 */

#pragma once

#include "def/BQConst.hpp"
#include "def/BQDef.hpp"
#include "def/Const.hpp"
#include "def/Def.hpp"
#include "util/Pch.hpp"

namespace bq {

struct OrderInfo;
using OrderInfoSPTr = std::shared_ptr<OrderInfo>;

Decimal calcFee(const OrderInfoSPTr& orderInfo, Decimal feeRatio,
                std::uint32_t parValue = 0);

std::string getFeeCurrency(const OrderInfoSPTr& orderInfo,
                           const std::string& baseCurrency,
                           const std::string& quoteCurrency);

}  // namespace bq
