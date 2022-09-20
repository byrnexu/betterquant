/*!
 * \file DBEngAsync.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "db/DBEngAsync.hpp"

#include "db/DBConnpool.hpp"
#include "db/DBEngDef.hpp"
#include "db/DBEngImpl.hpp"
#include "db/DBEngParam.hpp"
#include "db/DBTask.hpp"
#include "def/Const.hpp"
#include "util/Logger.hpp"

namespace bq::db {

DBEngAsync::DBEngAsync(const DBEngParamSPtr& dbEngParam,
                       const CBOnExecRet& cbOnExecRet)
    : DBEngImpl(dbEngParam, ConnType::Async), cbOnExecRet_(cbOnExecRet) {}

std::tuple<int, std::string> DBEngAsync::asyncOrSyncExecSql(
    const std::string& identity, const std::string& sql, WriteLog writeLog) {
  return asyncExecSql(identity, sql, writeLog);
}

std::tuple<int, std::string> DBEngAsync::asyncExecSql(
    const std::string& identity, const std::string& sql, WriteLog writeLog) {
  const auto dbTask = std::make_shared<bq::db::DBTask>(identity, sql, writeLog);
  taskQueue_.enqueue(dbTask);
  return {0, ""};
}

void DBEngAsync::start() {
  for (std::uint32_t i = 0; i < connPool_->getSize(); ++i) {
    threadPool_.emplace_back(std::thread([this]() { doStart(); }));
  }
}

void DBEngAsync::doStart() {
  while (stopped_ == false || taskQueue_.size_approx() != 0) {
    checkUnprocessedTaskAndAlert(taskQueue_);
    bq::db::DBTaskSPtr dbTask;
    if (taskQueue_.wait_dequeue_timed(
            dbTask,
            std::chrono::milliseconds(dbEngParam_->timeDurOfWaitForTask_))) {
      handleTask(dbTask);
    } else {
      continue;
    }
  }
}

void DBEngAsync::checkUnprocessedTaskAndAlert(
    const moodycamel::BlockingConcurrentQueue<DBTaskSPtr>& taskQueue) {
  if (taskQueue.size_approx() > 0 &&
      taskQueue.size_approx() % dbEngParam_->numOfUnprocessedTaskAlert_ == 0) {
    LOG_W("[{}] Too many unprocessed task. [num = {}].", dbEngParam_->svcName_,
          taskQueue_.size_approx());
  }
}

void DBEngAsync::handleTask(DBTaskSPtr& dbTask) {
  const auto [retOfExec, ret] =
      execSql(dbTask->identity_, dbTask->sql_, dbTask->writeLog_);
  const auto jsonFmtOfRet =
      std::make_shared<std::string>(fmt::format("{}{}{}", "{", ret, "}"));
  if (cbOnExecRet_) cbOnExecRet_(dbTask, jsonFmtOfRet);
}

void DBEngAsync::stop() {
  stopped_ = true;
  for (auto& trd : threadPool_) {
    if (trd.joinable()) {
      trd.join();
    }
  }
}

}  // namespace bq::db
