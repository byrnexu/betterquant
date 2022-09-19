#include "ExceedFlowCtrlHandler.hpp"

#include "SHMIPC.hpp"
#include "TDSvc.hpp"
#include "util/TaskDispatcher.hpp"

namespace bq::td::svc {

void ExceedFlowCtrlHandler::saveExceedFlowCtrlTask(
    const SHMIPCAsyncTaskSPtr& asyncTask) {
  {
    std::lock_guard<std::ext::spin_mutex> guard(
        mtxExceedFlowCtrlTaskInfoGroup_);
    exceedFlowCtrlTaskInfoGroup_.emplace_back(asyncTask);
    if (exceedFlowCtrlTaskInfoGroup_.size() % 10 == 0) {
      LOG_W("Exceed flow ctrl task num {}.",
            exceedFlowCtrlTaskInfoGroup_.size());
    }
  }
}

void ExceedFlowCtrlHandler::handleExceedFlowCtrlTask() {
  std::deque<SHMIPCAsyncTaskSPtr> exceedFlowCtrlTaskInfoGroupClone_;
  {
    std::lock_guard<std::ext::spin_mutex> guard(
        mtxExceedFlowCtrlTaskInfoGroup_);
    exceedFlowCtrlTaskInfoGroupClone_.assign(
        std::begin(exceedFlowCtrlTaskInfoGroup_),
        std::end(exceedFlowCtrlTaskInfoGroup_));
    exceedFlowCtrlTaskInfoGroup_.clear();
  }

  for (auto& asyncTask : exceedFlowCtrlTaskInfoGroupClone_) {
    tdSvc_->getTDSrvTaskDispatcher()->dispatch(asyncTask);
  }
}

}  // namespace bq::td::svc
