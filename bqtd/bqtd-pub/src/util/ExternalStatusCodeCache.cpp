
#include "util/ExternalStatusCodeCache.hpp"

#include "db/DBE.hpp"
#include "db/TBLRecSetMaker.hpp"
#include "def/BQConst.hpp"

namespace bq::td {

ExternalStatusCodeCache::ExternalStatusCodeCache(const db::DBEngSPtr& dbEng)
    : dbEng_(dbEng),
      identity2ExternalStatusCode_(
          std::make_shared<Identity2ExternalStatusCode>()) {}

int ExternalStatusCodeCache::load(const std::string& marketCode,
                                  const std::string& symbolType) {
  auto sql = fmt::format(
      "SELECT * FROM {} WHERE `marketCode` = '{}' AND `symbolType` = '{}'; ",
      TBLExternalStatusCode::TableName, marketCode, symbolType);

  auto [retOfExec, tblRecSet] =
      db::TBLRecSetMaker<TBLExternalStatusCode>::ExecSql(dbEng_, sql);
  if (retOfExec != 0) {
    LOG_W("Query external statuscode failed.");
    return retOfExec;
  }

  auto identity2ExternalStatusCode =
      std::make_shared<Identity2ExternalStatusCode>();
  for (const auto& tblRec : *tblRecSet) {
    const auto rec = tblRec.second->getRecWithAllFields();
    const auto identity = fmt::format("{}{}{}{}{}",                          //
                                      rec->marketCode, SEP_OF_REC_IDENTITY,  //
                                      rec->symbolType, SEP_OF_REC_IDENTITY,  //
                                      rec->externalStatusCode);
    const auto hash = XXH3_64bits(identity.data(), identity.size());
    identity2ExternalStatusCode->emplace(hash, rec);
  }
  LOG_D("Load external statuscode. [size = {}]",
        identity2ExternalStatusCode->size());

  {
    std::lock_guard<std::ext::spin_mutex> guard(
        mtxIdentity2ExternalStatusCode_);
    identity2ExternalStatusCode_ = identity2ExternalStatusCode;
  }

  return 0;
}

int ExternalStatusCodeCache::getAndSetStatusCodeIfNotExists(
    const std::string& marketCode, const std::string& symbolType,
    const std::string& externalStatusCode, const std::string& externalStatusMsg,
    int defaultValue) {
  const auto identity = fmt::format("{}{}{}{}{}",                     //
                                    marketCode, SEP_OF_REC_IDENTITY,  //
                                    symbolType, SEP_OF_REC_IDENTITY,  //
                                    externalStatusCode);
  const auto hash = XXH3_64bits(identity.data(), identity.size());
  std::shared_ptr<db::externalStatusCode::Record> rec;
  {
    std::lock_guard<std::ext::spin_mutex> guard(
        mtxIdentity2ExternalStatusCode_);
    const auto iter = identity2ExternalStatusCode_->find(hash);
    if (iter != std::end(*identity2ExternalStatusCode_)) {
      return iter->second->statusCode;
    } else {
      rec = std::make_shared<db::externalStatusCode::Record>();
      rec->marketCode = marketCode;
      rec->symbolType = symbolType;
      rec->externalStatusCode = externalStatusCode;
      rec->externalStatusMsg = externalStatusMsg;
      rec->statusCode = defaultValue;
      identity2ExternalStatusCode_->emplace(hash, rec);
    }
  }

  const auto tblRec = std::make_shared<db::TBLRec<TBLExternalStatusCode>>(rec);
  const auto ret = ExecRecInsert(dbEng_, tblRec);
  if (ret == 0) {
    LOG_I("Sync external statuscode to db success. {}", identity);
  } else {
    LOG_W("Sync external statuscode to db failed. {}", identity);
  }
  return defaultValue;
}

}  // namespace bq::td
