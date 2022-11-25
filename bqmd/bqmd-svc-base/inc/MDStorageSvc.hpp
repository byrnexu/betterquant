/*!
 * \file MDStorageSvc.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/16
 *
 * \brief
 */

#pragma once

#include "def/BQMDDef.hpp"
#include "util/Pch.hpp"

namespace bq::web {
struct TaskFromSrv;
using TaskFromSrvSPtr = std::shared_ptr<TaskFromSrv>;
}  // namespace bq::web

namespace bq {
template <typename Task>
class TaskDispatcher;
template <typename Task>
using TaskDispatcherSPtr = std::shared_ptr<TaskDispatcher<Task>>;
template <typename Task>
struct AsyncTask;
template <typename Task>
using AsyncTaskSPtr = std::shared_ptr<AsyncTask<Task>>;
using WSCliAsyncTask = AsyncTask<bq::web::TaskFromSrvSPtr>;
using WSCliAsyncTaskSPtr = std::shared_ptr<WSCliAsyncTask>;
}  // namespace bq

namespace bq::md {
struct WSCliAsyncTaskArg;
using WSCliAsyncTaskArgSPtr = std::shared_ptr<WSCliAsyncTaskArg>;
}  // namespace bq::md

namespace bq::md::svc {

using Filename2MDGroup = std::map<std::string, std::vector<std::string>>;
using Filename2MDGroupSPtr = std::shared_ptr<Filename2MDGroup>;

class MDSvc;

class MDStorageSvc {
 public:
  MDStorageSvc(MDSvc const* mdSvc);

 public:
  int init();
  void start();
  void stop();

 public:
  void handle(WSCliAsyncTaskSPtr& asyncTask);

 private:
  void handleAsyncTask(WSCliAsyncTaskSPtr& asyncTask);

 private:
  void cacheFilename2MDGroup(WSCliAsyncTaskSPtr& asyncTask);
  std::tuple<int, boost::filesystem::path> createDirOfMDOfUnifiedFmt(
      WSCliAsyncTaskSPtr& asyncTask);
  void cacheMDOfUnifiedFmt(WSCliAsyncTaskSPtr& asyncTask,
                           const boost::filesystem::path& storagePath);

  void cacheMDOfOrigFmt(const std::string& marketDataOfOrigFmt,
                        const boost::filesystem::path& storagePath,
                        const std::string& exchHour);
  std::tuple<int, boost::filesystem::path, std::string> createDirOfMDOfOrigFmt(
      WSCliAsyncTaskSPtr& asyncTask);

  void flushMDInCacheToDisk();

 protected:
  MDSvc const* mdSvc_{nullptr};
  TaskDispatcherSPtr<web::TaskFromSrvSPtr> taskDispatcher_{nullptr};

  Filename2MDGroupSPtr filename2MDGroup_;
  std::map<std::string, WSCliAsyncTaskArgSPtr> candleTopic2CandleData_;
};

}  // namespace bq::md::svc
