/*!
 * \file SymbolInfoExt.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "util/Pch.hpp"

namespace bq {

struct SymbolInfo;
using SymbolInfoSPtr = std::shared_ptr<SymbolInfo>;

std::string SymbolInfoGroup2Str(
    const std::vector<SymbolInfoSPtr> symbolInfoGroup);

}  // namespace bq
