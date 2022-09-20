#pragma once

#include "util/Pch.hpp"

namespace bq {
struct SHMIPCTask;
using SHMIPCTaskSPtr = std::shared_ptr<SHMIPCTask>;
template <typename Task>
struct AsyncTask;
using SHMIPCAsyncTask = AsyncTask<SHMIPCTaskSPtr>;
using SHMIPCAsyncTaskSPtr = std::shared_ptr<SHMIPCAsyncTask>;
}  // namespace bq

namespace bq::td::srv {

class TDSrv;

class StgEngTaskHandler {
 public:
  StgEngTaskHandler(const StgEngTaskHandler&) = delete;
  StgEngTaskHandler& operator=(const StgEngTaskHandler&) = delete;
  StgEngTaskHandler(const StgEngTaskHandler&&) = delete;
  StgEngTaskHandler& operator=(const StgEngTaskHandler&&) = delete;

  explicit StgEngTaskHandler(TDSrv* tdSrv);

 public:
  void handleAsyncTask(const SHMIPCAsyncTaskSPtr& asyncTask);

 private:
  void handleMsgIdOnOrder(const SHMIPCAsyncTaskSPtr& asyncTask);
  void handleMsgIdOnCancelOrder(const SHMIPCAsyncTaskSPtr& asyncTask);

  void handleMsgIdOnStgReg(const SHMIPCAsyncTaskSPtr& asyncTask);

 private:
  TDSrv* tdSrv_;
};

}  // namespace bq::td::srv