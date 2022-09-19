#include "db/DBEngSync.hpp"

#include "def/Const.hpp"

namespace bq::db {

DBEngSync::DBEngSync(const DBEngParamSPtr& dbEngParam)
    : DBEngImpl(dbEngParam, ConnType::Sync) {}

std::tuple<int, std::string> DBEngSync::asyncOrSyncExecSql(
    const std::string& identity, const std::string& sql, WriteLog writeLog) {
  return syncExecSql(identity, sql, writeLog);
}

std::tuple<int, std::string> DBEngSync::syncExecSql(const std::string& identity,
                                                    const std::string& sql,
                                                    WriteLog writeLog) {
  const auto [retOfExec, ret] = execSql(identity, sql, writeLog);
  const auto jsonFmtOfRet = fmt::format("{}{}{}", "{", ret, "}");
  return {retOfExec, jsonFmtOfRet};
}

}  // namespace bq::db
