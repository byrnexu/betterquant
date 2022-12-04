/*!
 * \file MDSimDef.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/26
 *
 * \brief
 */

#pragma once

#include "def/Def.hpp"
#include "util/Pch.hpp"

namespace bq::md {

struct MarketDataOfSim {
  MarketDataOfSim(std::uint64_t localTs, void* data)
      : localTs_(localTs), data_(data) {}
  MarketDataOfSim() { SAFE_FREE(data_); }

  std::uint64_t localTs_;

  void* data_{nullptr};
  std::size_t dataLen_;

  std::uint64_t delay_;
};
using MarketDataOfSimSPtr = std::shared_ptr<MarketDataOfSim>;

using Ts2MarketDataOfSimGroup =
    std::multimap<std::uint64_t, MarketDataOfSimSPtr>;
using Ts2MarketDataOfSimGroupSPtr = std::shared_ptr<Ts2MarketDataOfSimGroup>;

}  // namespace bq::md
