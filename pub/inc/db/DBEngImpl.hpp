/*!
 * \file DBEngImpl.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "db/DBEngDef.hpp"
#include "util/Pch.hpp"

namespace bq {
enum class WriteLog;
}

namespace bq::db {

class DBEngImpl {
 public:
  DBEngImpl(const DBEngImpl&) = delete;
  DBEngImpl& operator=(const DBEngImpl&) = delete;
  DBEngImpl(const DBEngImpl&&) = delete;
  DBEngImpl& operator=(const DBEngImpl&&) = delete;

 public:
  DBEngImpl(const DBEngParamSPtr& dbEngParam, ConnType connType);

 public:
  int init();

 public:
  std::tuple<int, std::string> execUSP(const std::string& identity,
                                       const std::string& sql,
                                       WriteLog writeLog);

 private:
  virtual std::tuple<int, std::string> asyncOrSyncExecSql(
      const std::string& identity, const std::string& sql,
      WriteLog writeLog) = 0;

 protected:
  std::tuple<int, std::string> execSql(const std::string& identity,
                                       const std::string& sql,
                                       WriteLog writeLog);

 private:
  std::string execSqlImpl(const ConnSPtr& conn, const std::string& sql);

  std::string handleRecordSet(const std::shared_ptr<sql::ResultSet>& res,
                              int no);

  std::string getJsonFmtOfRecordSet(
      int no, const std::shared_ptr<sql::ResultSet>& res,
      const std::vector<int>& fieldTypeGroup,
      const std::vector<std::string>& fieldNameGroup);

  std::string getJsonFmtOfRec(const std::shared_ptr<sql::ResultSet>& res,
                              const std::vector<int>& fieldTypeGroup,
                              const std::vector<std::string>& fieldNameGroup);

  std::string getJsonFmtOfField(const std::string& fieldName,
                                const std::string& fieldValue);
  std::string getJsonFmtOfField(const std::string& fieldName, int fieldValue);
  std::string getJsonFmtOfField(const std::string& fieldName,
                                std::int64_t fieldValue);
  std::string getJsonFmtOfField(const std::string& fieldName,
                                double fieldValue);
  std::string getJsonFmtOfStatus(int statusCode, const std::string& statusMsg);

 protected:
  DBEngParamSPtr dbEngParam_;
  DBConnpoolSPtr connPool_;
};

}  // namespace bq::db
