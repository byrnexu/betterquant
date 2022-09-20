/*!
 * \file TDSrvTaskHandler.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

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

namespace bq::td::svc {

class TDSvc;

class TDSrvTaskHandler {
 public:
  TDSrvTaskHandler(const TDSrvTaskHandler&) = delete;
  TDSrvTaskHandler& operator=(const TDSrvTaskHandler&) = delete;
  TDSrvTaskHandler(const TDSrvTaskHandler&&) = delete;
  TDSrvTaskHandler& operator=(const TDSrvTaskHandler&&) = delete;

  explicit TDSrvTaskHandler(TDSvc* tdSvc);

 public:
  void handleAsyncTask(SHMIPCAsyncTaskSPtr& asyncTask);

 private:
  void handleMsgIdOnOrder(SHMIPCAsyncTaskSPtr& asyncTask);
  void handleMsgIdOnCancelOrder(SHMIPCAsyncTaskSPtr& asyncTask);

  void handleMsgIdSyncUnclosedOrderInfo(SHMIPCAsyncTaskSPtr& asyncTask);
  void handleMsgIdSyncAssetsSnapshot(SHMIPCAsyncTaskSPtr& asyncTask);
  void handleMsgIdExtenConnLifecycle(SHMIPCAsyncTaskSPtr& asyncTask);

  void handleMsgIdOnTDGWReg(SHMIPCAsyncTaskSPtr& asyncTask);

  void handleMsgIdTestOrder(SHMIPCAsyncTaskSPtr& asyncTask);
  void handleMsgIdTestCancelOrder(SHMIPCAsyncTaskSPtr& asyncTask);

 private:
  TDSvc* tdSvc_;
};

}  // namespace bq::td::svc
