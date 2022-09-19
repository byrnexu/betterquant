#include "StgEngTaskHandler.hpp"

#include "ClientChannelGroup.hpp"
#include "OrdMgr.hpp"
#include "RiskMgr.hpp"
#include "SHMHeader.hpp"
#include "SHMIPCMsgId.hpp"
#include "SHMIPCTask.hpp"
#include "SHMIPCUtil.hpp"
#include "SHMSrv.hpp"
#include "db/TBLMonitorOfSymbolInfo.hpp"
#include "def/BQDef.hpp"
#include "def/DataStruOfMD.hpp"
#include "def/DataStruOfOthers.hpp"
#include "def/DataStruOfTD.hpp"
#include "util/Datetime.hpp"
#include "util/StdExt.hpp"
#include "util/TaskDispatcher.hpp"

namespace bq::riskmgr {

StgEngTaskHandler::StgEngTaskHandler(RiskMgr* riskMgr) : riskMgr_(riskMgr) {}

void StgEngTaskHandler::handleAsyncTask(
    const AsyncTaskSPtr<SHMIPCTaskSPtr>& asyncTask) {
  const auto shmHeader = static_cast<const SHMHeader*>(asyncTask->task_->data_);
  switch (shmHeader->msgId_) {
    case MSG_ID_ON_ORDER:
      handleMsgIdOnOrder(asyncTask);
      break;
    case MSG_ID_ON_CANCEL_ORDER:
      handleMsgIdOnCancelOrder(asyncTask);
      break;
    case MSG_ID_ON_STG_REG:
      handleMsgIdOnStgReg(asyncTask);
      break;
    default:
      LOG_W("Unable to process msgId {}.", shmHeader->msgId_);
      break;
  }
}

void StgEngTaskHandler::handleMsgIdOnOrder(
    const AsyncTaskSPtr<SHMIPCTaskSPtr>& asyncTask) {
  auto ordReq = MakeMsgSPtrByTask<OrderInfo>(asyncTask->task_);
  LOG_I("Recv order {}", ordReq->toShortStr());
}

void StgEngTaskHandler::handleMsgIdOnCancelOrder(
    const AsyncTaskSPtr<SHMIPCTaskSPtr>& asyncTask) {
  auto ordReq = MakeMsgSPtrByTask<OrderInfo>(asyncTask->task_);
  LOG_I("Recv cancel order {}", ordReq->toShortStr());
}

void StgEngTaskHandler::handleMsgIdOnStgReg(
    const AsyncTaskSPtr<SHMIPCTaskSPtr>& asyncTask) {
  const auto reqHeader = static_cast<const SHMHeader*>(asyncTask->task_->data_);
  LOG_D("Recv msg {}. [channel = {}]", GetMsgName(reqHeader->msgId_),
        reqHeader->clientChannel_);
  riskMgr_->getStgEngGroup()->update(reqHeader->clientChannel_);
}

}  // namespace bq::riskmgr
