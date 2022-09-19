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

class TDGWTaskHandler {
 public:
  TDGWTaskHandler(const TDGWTaskHandler&) = delete;
  TDGWTaskHandler& operator=(const TDGWTaskHandler&) = delete;
  TDGWTaskHandler(const TDGWTaskHandler&&) = delete;
  TDGWTaskHandler& operator=(const TDGWTaskHandler&&) = delete;

  explicit TDGWTaskHandler(RiskMgr* riskMgr);

 public:
  void handleAsyncTask(const SHMIPCAsyncTaskSPtr& asyncTask);

 private:
  void handleMsgIdOnOrderRet(const SHMIPCAsyncTaskSPtr& asyncTask);
  void handleMsgIdOnCancelOrderRet(const SHMIPCAsyncTaskSPtr& asyncTask);

  void handleMsgIdSyncAssets(const SHMIPCAsyncTaskSPtr& asyncTask);
  void handleMsgIdOnTDGWReg(const SHMIPCAsyncTaskSPtr& asyncTask);

 private:
  RiskMgr* riskMgr_;
};

}  // namespace bq::riskmgr
