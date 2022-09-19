#pragma once

#include "util/Pch.hpp"
#include "util/StdExt.hpp"

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

class ExceedFlowCtrlHandler {
 public:
  ExceedFlowCtrlHandler(const ExceedFlowCtrlHandler&) = delete;
  ExceedFlowCtrlHandler& operator=(const ExceedFlowCtrlHandler&) = delete;
  ExceedFlowCtrlHandler(const ExceedFlowCtrlHandler&&) = delete;
  ExceedFlowCtrlHandler& operator=(const ExceedFlowCtrlHandler&&) = delete;

  explicit ExceedFlowCtrlHandler(TDSvc* tdSvc) : tdSvc_(tdSvc) {}

 public:
  void saveExceedFlowCtrlTask(const SHMIPCAsyncTaskSPtr& asyncTask);
  void handleExceedFlowCtrlTask();

 private:
  std::deque<SHMIPCAsyncTaskSPtr> exceedFlowCtrlTaskInfoGroup_;
  std::ext::spin_mutex mtxExceedFlowCtrlTaskInfoGroup_;

  TDSvc* tdSvc_;
};

}  // namespace bq::td::svc
