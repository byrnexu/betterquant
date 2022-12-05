/*!
 * \file PosSnapshot.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/21
 *
 * \brief
 */

#pragma once

#include "def/BQConst.hpp"
#include "def/BQDef.hpp"
#include "def/Def.hpp"
#include "util/Pch.hpp"

namespace bq {

struct MarketDataCond {
  MarketCode marketCode_;
  SymbolType symbolType_;
  std::string symbolCode_;
  MDType mdType_;
  std::string ext_;
};
using MarketDataCondSPtr = std::shared_ptr<MarketDataCond>;

std::tuple<int, MarketDataCondSPtr> GetMarketDataCondFromTopic(
    const std::string& topic);

}  // namespace bq
