/*!
 * \file DBEngAsync.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "db/DBEngDef.hpp"
#include "db/DBEngImpl.hpp"

namespace bq {
enum class WriteLog;
}

namespace bq::db {

class DBEngAsync : public DBEngImpl {
 public:
  DBEngAsync(const DBEngAsync&) = delete;
  DBEngAsync& operator=(const DBEngAsync&) = delete;
  DBEngAsync(const DBEngAsync&&) = delete;
  DBEngAsync& operator=(const DBEngAsync&&) = delete;

 public:
  DBEngAsync(const DBEngParamSPtr& dbEngParam, const CBOnExecRet& cbOnExecRet);

 private:
  std::tuple<int, std::string> asyncOrSyncExecSql(const std::string& identity,
                                                  const std::string& sql,
                                                  WriteLog writeLog) final;

  std::tuple<int, std::string> asyncExecSql(const std::string& identity,
                                            const std::string& sql,
                                            WriteLog writeLog);

 public:
  void start();

 private:
  void doStart();

  void checkUnprocessedTaskAndAlert(
      const moodycamel::BlockingConcurrentQueue<DBTaskSPtr>& taskQueue);

  void handleTask(DBTaskSPtr& dbTask);

 public:
  void stop();

 private:
  bool stopped_{false};

  moodycamel::BlockingConcurrentQueue<DBTaskSPtr> taskQueue_;
  std::vector<std::thread> threadPool_;

  CBOnExecRet cbOnExecRet_{nullptr};
};

}  // namespace bq::db
