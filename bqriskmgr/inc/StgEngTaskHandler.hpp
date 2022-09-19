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

namespace bq::riskmgr {

class RiskMgr;

class StgEngTaskHandler {
 public:
  StgEngTaskHandler(const StgEngTaskHandler&) = delete;
  StgEngTaskHandler& operator=(const StgEngTaskHandler&) = delete;
  StgEngTaskHandler(const StgEngTaskHandler&&) = delete;
  StgEngTaskHandler& operator=(const StgEngTaskHandler&&) = delete;

  explicit StgEngTaskHandler(RiskMgr* riskMgr);

 public:
  void handleAsyncTask(const SHMIPCAsyncTaskSPtr& asyncTask);

 private:
  void handleMsgIdOnOrder(const SHMIPCAsyncTaskSPtr& asyncTask);
  void handleMsgIdOnCancelOrder(const SHMIPCAsyncTaskSPtr& asyncTask);

  void handleMsgIdOnStgReg(const SHMIPCAsyncTaskSPtr& asyncTask);

 private:
  RiskMgr* riskMgr_;
};

}  // namespace bq::riskmgr
