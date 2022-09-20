/*!
 * \file SymbolInfo.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "def/SymbolInfo.hpp"

#include "def/BQConst.hpp"

namespace bq {

std::string SymbolInfo::toStr() const {
  std::string ret;
  ret = fmt::format("{} {} {}", GetMarketName(marketCode_),
                    magic_enum::enum_name(symbolType_), symbolCode_);
  return ret;
}

std::string SymbolInfoGroup2Str(
    const std::vector<SymbolInfoSPtr> symbolInfoGroup) {
  std::string ret;
  for (const auto symbolInfo : symbolInfoGroup) {
    ret = ret + symbolInfo->toStr() + "; ";
  }
  if (ret.size() > 1) {
    ret = ret.substr(0, ret.length() - 2);
  }
  return ret;
}

}  // namespace bq
