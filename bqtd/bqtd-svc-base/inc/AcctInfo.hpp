#pragma once

#include "def/BQConst.hpp"
#include "def/BQDef.hpp"
#include "util/Pch.hpp"

namespace bq::td::svc {

struct AcctInfo;
using AcctInfoSPtr = std::shared_ptr<AcctInfo>;

struct AcctInfo {
 public:
  MarketCode marketCode_;
  SymbolType symbolType_;
  AcctId acctId_;
  std::string acctData_;

  int init(AcctId acctId, const std::string& execRet);
};

}  // namespace bq::td::svc
