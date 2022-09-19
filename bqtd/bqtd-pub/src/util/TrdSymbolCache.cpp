
#include "util/TrdSymbolCache.hpp"

#include "db/DBE.hpp"
#include "db/TBLRecSetMaker.hpp"
#include "db/TBLTrdSymbol.hpp"
#include "def/BQConst.hpp"

namespace bq::td {

TrdSymbolCache::TrdSymbolCache(const db::DBEngSPtr& dbEng)
    : dbEng_(dbEng),
      identity2TrdSymbol_(std::make_shared<Identity2TrdSymbol>()) {}

int TrdSymbolCache::load(const std::string& marketCode,
                         const std::string& symbolType, AcctId acctId) {
  auto sql =
      fmt::format("SELECT * FROM {} WHERE isDel = 0 ", TBLTrdSymbol::TableName);

  if (!marketCode.empty()) {
    sql = sql + fmt::format("AND `marketCode` = '{}' ", marketCode);
  }
  if (!symbolType.empty()) {
    sql = sql + fmt::format("AND `symbolType` = '{}' ", symbolType);
  }
  if (acctId == 0) {
    sql = sql + fmt::format("AND `acctId` = '{}' ", acctId);
  }

  const auto [retOfExec, tblRecSet] =
      db::TBLRecSetMaker<TBLTrdSymbol>::ExecSql(dbEng_, sql);
  if (retOfExec != 0) {
    LOG_W("Query exch trade failed.");
    return retOfExec;
  }

  for (const auto& tblRec : *tblRecSet) {
    const auto rec = tblRec.second->getRecWithAllFields();
    const auto identity = fmt::format("{}{}{}{}{}{}{}{}{}", rec->marketCode,  //
                                      SEP_OF_REC_IDENTITY, rec->symbolCode,   //
                                      SEP_OF_REC_IDENTITY, rec->symbolType,   //
                                      SEP_OF_REC_IDENTITY, rec->acctId,       //
                                      SEP_OF_REC_IDENTITY, rec->exchSymbolCode);
    const auto hash = XXH3_64bits(identity.data(), identity.size());
    {
      std::lock_guard<std::ext::spin_mutex> guard(mtxIdentity2TrdSymbol_);
      identity2TrdSymbol_->emplace(hash, rec);
    }
  }

  return 0;
}

std::tuple<int, bool> TrdSymbolCache::add(const OrderInfoSPtr& orderInfo,
                                          SyncToDB syncToDB) {
  const auto identity = fmt::format(
      "{}{}{}{}{}{}{}{}{}", GetMarketName(orderInfo->marketCode_),
      SEP_OF_REC_IDENTITY, orderInfo->symbolCode_,                         //
      SEP_OF_REC_IDENTITY, magic_enum::enum_name(orderInfo->symbolType_),  //
      SEP_OF_REC_IDENTITY, orderInfo->acctId_,                             //
      SEP_OF_REC_IDENTITY, orderInfo->exchSymbolCode_);
  const auto hash = XXH3_64bits(identity.data(), identity.size());
  std::shared_ptr<db::trdSymbol::Record> rec;
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxIdentity2TrdSymbol_);
    const auto iter = identity2TrdSymbol_->find(hash);
    if (iter != std::end(*identity2TrdSymbol_)) {
      return {0, false};
    } else {
      rec = std::make_shared<db::trdSymbol::Record>();
      rec->marketCode = GetMarketName(orderInfo->marketCode_);
      rec->symbolType = magic_enum::enum_name(orderInfo->symbolType_);
      rec->acctId = orderInfo->acctId_;
      rec->symbolCode = orderInfo->symbolCode_;
      rec->exchSymbolCode = orderInfo->exchSymbolCode_;
      rec->isDel = 0;
      identity2TrdSymbol_->emplace(hash, rec);
    }
  }

  if (syncToDB == SyncToDB::True) {
    const auto tblRec = std::make_shared<db::TBLRec<TBLTrdSymbol>>(rec);
    const auto ret = ExecRecInsert(dbEng_, tblRec);
    if (ret != 0) {
      LOG_W("Sync trd symbol to db failed.");
      return {ret, false};
    }
  }

  LOG_I("Sync trd symbol to db success. {}", identity);
  return {0, true};
}

Identity2TrdSymbolSPtr TrdSymbolCache::getTrdSymbolGroup() const {
  Identity2TrdSymbolSPtr ret = std::make_shared<Identity2TrdSymbol>();
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxIdentity2TrdSymbol_);
    for (const auto rec : *identity2TrdSymbol_) {
      ret->emplace(rec.first,
                   std::make_shared<db::trdSymbol::Record>(*rec.second));
    }
  }
  return ret;
}

}  // namespace bq::td
