/*!
 * \file DBEngSync.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "db/DBEngImpl.hpp"

namespace bq {
enum class WriteLog;
}

namespace bq::db {

class DBEngSync : public DBEngImpl {
 public:
  DBEngSync(const DBEngSync&) = delete;
  DBEngSync& operator=(const DBEngSync&) = delete;
  DBEngSync(const DBEngSync&&) = delete;
  DBEngSync& operator=(const DBEngSync&&) = delete;

 public:
  DBEngSync(const DBEngParamSPtr& dbEngParam);

 private:
  std::tuple<int, std::string> asyncOrSyncExecSql(const std::string& identity,
                                                  const std::string& sql,
                                                  WriteLog writeLog) final;

  std::tuple<int, std::string> syncExecSql(const std::string& identity,
                                           const std::string& sql,
                                           WriteLog writeLog);
};

}  // namespace bq::db
