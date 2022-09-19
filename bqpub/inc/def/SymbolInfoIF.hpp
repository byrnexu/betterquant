#pragma once

#include "def/BQConstIF.hpp"
#include "util/PchBase.hpp"

namespace bq {

struct SymbolInfo;
using SymbolInfoSPtr = std::shared_ptr<SymbolInfo>;

struct SymbolInfo {
  SymbolInfo(MarketCode marketCode, SymbolType symbolType,
             const std::string& symbolCode)
      : marketCode_(marketCode),
        symbolType_(symbolType),
        symbolCode_(symbolCode) {}
  MarketCode marketCode_;
  SymbolType symbolType_;
  std::string symbolCode_;

  std::string toStr() const;
};

}  // namespace bq
