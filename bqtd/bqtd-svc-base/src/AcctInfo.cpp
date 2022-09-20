/*!
 * \file AcctInfo.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "AcctInfo.hpp"

#include "def/Def.hpp"
#include "util/Logger.hpp"

namespace bq::td::svc {

int AcctInfo::init(AcctId acctId, const std::string& execRet) {
  Doc doc;
  doc.Parse(execRet.data());
  if (doc["recordSetGroup"].Size() == 0) {
    LOG_W("Init acct data failed because of no result of acct data of {}.",
          acctId);
    return -1;
  }

  const auto marketCode = doc["recordSetGroup"][0][0]["marketCode"].GetString();
  const auto symbolType = doc["recordSetGroup"][0][0]["symbolType"].GetString();
  marketCode_ = GetMarketCode(marketCode);
  symbolType_ = magic_enum::enum_cast<SymbolType>(symbolType).value();
  acctId_ = doc["recordSetGroup"][0][0]["acctId"].GetUint();
  acctData_ = doc["recordSetGroup"][0][0]["acctData"].GetString();

  return 0;
}

}  // namespace bq::td::svc
