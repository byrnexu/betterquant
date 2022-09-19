#pragma once

#include "db/DBEngAsync.hpp"
#include "db/DBEngConst.hpp"
#include "db/DBEngParam.hpp"
#include "db/DBEngSync.hpp"
#include "db/DBTask.hpp"
#include "def/Const.hpp"

namespace bq::db {

class DBEng {
 public:
  DBEng(const DBEng&) = delete;
  DBEng& operator=(const DBEng&) = delete;
  DBEng(DBEng&&) = delete;
  DBEng& operator=(DBEng&&) = delete;

  DBEng(const DBEngParamSPtr& dbEngParam, const CBOnExecRet& cbOnExecRet);

 public:
  int init();

  void start();
  void stop();

 public:
  std::tuple<int, std::string> syncExec(const std::string& identity,
                                        const std::string& sql,
                                        WriteLog writeLog = WriteLog::True);

  std::tuple<int, std::string> asyncExec(const std::string& identity,
                                         const std::string& sql,
                                         WriteLog writeLog = WriteLog::True);

 private:
  DBEngParamSPtr dbEngParam_{nullptr};
  CBOnExecRet cbOnExecRet_{nullptr};
  DBEngSyncSPtr dbEngSync_{nullptr};
  DBEngAsyncSPtr dbEngAsync_{nullptr};
};

}  // namespace bq::db
