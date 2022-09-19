#include "db/DBEng.hpp"

#include "db/DBEngAsync.hpp"
#include "db/DBEngParam.hpp"
#include "db/DBEngSync.hpp"
#include "util/Logger.hpp"

namespace bq::db {

DBEng::DBEng(const DBEngParamSPtr& dbEngParam, const CBOnExecRet& cbOnExecRet)
    : dbEngParam_(dbEngParam),
      cbOnExecRet_(cbOnExecRet),
      dbEngSync_(std::make_shared<DBEngSync>(dbEngParam_)),
      dbEngAsync_(std::make_shared<DBEngAsync>(dbEngParam_, cbOnExecRet_)) {
  assert(dbEngParam_ != nullptr && "dbEngParam_ != nullptr");
}

int DBEng::init() {
  int ret = 0;
  ret = dbEngSync_->init();
  if (ret != 0) {
    LOG_E("[{}] Init dbeng of sync failed.", dbEngParam_->svcName_);
    return ret;
  }

  ret = dbEngAsync_->init();
  if (ret != 0) {
    LOG_E("[{}] Init dbeng of async failed.", dbEngParam_->svcName_);
    return ret;
  }

  return ret;
}

void DBEng::start() {
  LOG_D("[{}] Begin to start db engine.", dbEngParam_->svcName_);
  dbEngAsync_->start();
}
void DBEng::stop() {
  LOG_D("[{}] Begin to stop db engine.", dbEngParam_->svcName_);
  dbEngAsync_->stop();
}

std::tuple<int, std::string> DBEng::syncExec(const std::string& identity,
                                             const std::string& sql,
                                             WriteLog writeLog) {
  return dbEngSync_->execUSP(identity, sql, writeLog);
}

std::tuple<int, std::string> DBEng::asyncExec(const std::string& identity,
                                              const std::string& sql,
                                              WriteLog writeLog) {
  return dbEngAsync_->execUSP(identity, sql, writeLog);
}

}  // namespace bq::db
