/*!
 * \file AcctInfoCache.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "util/AcctInfoCache.hpp"

#include "db/DBE.hpp"
#include "db/TBLAcctInfo.hpp"
#include "db/TBLRecSetMaker.hpp"
#include "def/AcctInfoIF.hpp"
#include "def/BQConst.hpp"
#include "def/StatusCode.hpp"

namespace bq {

AcctInfoCache::AcctInfoCache(const db::DBEngSPtr& dbEng)
    : dbEng_(dbEng), acctId2AcctInfo_(std::make_shared<AcctId2AcctInfo>()) {}

int AcctInfoCache::load() {
  auto sql = fmt::format("SELECT * FROM {}; ", TBLAcctInfo::TableName);

  auto [retOfExec, tblRecSet] =
      db::TBLRecSetMaker<TBLAcctInfo>::ExecSql(dbEng_, sql);
  if (retOfExec != 0) {
    LOG_W("Query acct info failed.");
    return retOfExec;
  }

  auto acctId2AcctInfo = std::make_shared<AcctId2AcctInfo>();
  for (const auto& tblRec : *tblRecSet) {
    const auto rec = tblRec.second->getRecWithAllFields();
    auto acctInfo = std::make_shared<AcctInfo>();
    acctInfo->acctId_ = rec->acctId;
    const auto marketCodeValue =
        magic_enum::enum_cast<MarketCode>(rec->marketCode);
    if (!marketCodeValue.has_value()) {
      LOG_W("Invalid market code value {} in tbl acctinfo.", rec->marketCode);
      return -1;
    }
    acctInfo->marketCode_ = marketCodeValue.value();
    const auto symbolTypeValue =
        magic_enum::enum_cast<SymbolType>(rec->symbolType);
    if (!symbolTypeValue.has_value()) {
      LOG_W("Invalid symbol type value {} in tbl acctinfo.", rec->symbolType);
      return -1;
    }
    acctInfo->symbolType_ = symbolTypeValue.value();
    acctId2AcctInfo->emplace(acctInfo->acctId_, acctInfo);
  }
  LOG_D("Load acct info. [size = {}]", acctId2AcctInfo->size());

  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxAcctId2AcctInfo_);
    acctId2AcctInfo_ = acctId2AcctInfo;
  }

  return 0;
}

std::tuple<int, MarketCode, SymbolType>
AcctInfoCache::getMarketCodeAndSymbolType(AcctId acctId) {
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxAcctId2AcctInfo_);
    const auto iter = acctId2AcctInfo_->find(acctId);
    if (iter != std::end(*acctId2AcctInfo_)) {
      return {0, iter->second->marketCode_, iter->second->symbolType_};
    }
  }
  return {SCODE_DB_CAN_NOT_FIND_ACCT_INFO, MarketCode::Others,
          SymbolType::Others};
}

}  // namespace bq
