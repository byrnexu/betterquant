/*!
 * \file StgEngTaskHandler.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "StgEngTaskHandler.hpp"

#include "ClientChannelGroup.hpp"
#include "OrdMgr.hpp"
#include "SHMHeader.hpp"
#include "SHMIPCMsgId.hpp"
#include "SHMIPCTask.hpp"
#include "SHMIPCUtil.hpp"
#include "SHMSrv.hpp"
#include "TDSrv.hpp"
#include "TDSrvRiskPluginMgr.hpp"
#include "db/TBLMonitorOfSymbolInfo.hpp"
#include "def/BQDef.hpp"
#include "def/DataStruOfMD.hpp"
#include "def/DataStruOfOthers.hpp"
#include "def/DataStruOfTD.hpp"
#include "def/StatusCode.hpp"
#include "def/TaskOfSync.hpp"
#include "util/Datetime.hpp"
#include "util/StdExt.hpp"
#include "util/TaskDispatcher.hpp"
#include "util/Util.hpp"

namespace bq::td::srv {

StgEngTaskHandler::StgEngTaskHandler(TDSrv* tdSrv) : tdSrv_(tdSrv) {}

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
#ifndef OPT_LOG
  LOG_I("Recv order {}", ordReq->toShortStr());
#endif

  if (tdSrv_->getTDGWGroup()->exists(ordReq->acctId_) == false) {
    LOG_W("Handle msg id on order failed. {}", ordReq->toShortStr());
    ordReq->orderStatus_ = OrderStatus::Failed;
    ordReq->statusCode_ = SCODE_TD_SRV_TDGW_NOT_EXISTS;

    tdSrv_->getSHMSrvOfStgEng()->pushMsgWithZeroCopy(
        [&](void* shmBuf) {
          InitMsgBody(shmBuf, *ordReq);
          LOG_I("Forward order ret {}",
                static_cast<OrderInfo*>(shmBuf)->toShortStr());
        },
        ordReq->stgId_, MSG_ID_ON_ORDER_RET, sizeof(OrderInfo));

    tdSrv_->cacheTaskOfSyncGroup(MSG_ID_ON_ORDER_RET, ordReq,
                                 SyncToRiskMgr::False, SyncToDB::True);
    return;
  }

  const auto ret = std::ext::tls_get<OrdMgr>().add(ordReq, DeepClone::False,
                                                   LockFunc::False);
  if (ret != 0) {
    LOG_W("Handle msg id on order failed. {}", ordReq->toShortStr());
    ordReq->orderStatus_ = OrderStatus::Failed;
    ordReq->statusCode_ = ret;

    tdSrv_->getSHMSrvOfStgEng()->pushMsgWithZeroCopy(
        [&](void* shmBuf) {
          InitMsgBody(shmBuf, *ordReq);
          LOG_I("Forward order ret {}",
                static_cast<OrderInfo*>(shmBuf)->toShortStr());
        },
        ordReq->stgId_, MSG_ID_ON_ORDER_RET, sizeof(OrderInfo));

    tdSrv_->cacheTaskOfSyncGroup(MSG_ID_ON_ORDER_RET, ordReq,
                                 SyncToRiskMgr::False, SyncToDB::True);
    return;
  }

  const auto statusCode = tdSrv_->getTDSrvRiskPluginMgr()->onOrder(ordReq);
  if (statusCode != 0) {
    LOG_W("Risk check order failed. [{} - {}] {}", statusCode,
          GetStatusMsg(statusCode), ordReq->toShortStr());
    ordReq->orderStatus_ = OrderStatus::Failed;
    ordReq->statusCode_ = statusCode;

    tdSrv_->getSHMSrvOfStgEng()->pushMsgWithZeroCopy(
        [&](void* shmBuf) {
          InitMsgBody(shmBuf, *ordReq);
          LOG_I("Forward order ret {}",
                static_cast<OrderInfo*>(shmBuf)->toShortStr());
        },
        ordReq->stgId_, MSG_ID_ON_ORDER_RET, sizeof(OrderInfo));

    tdSrv_->cacheTaskOfSyncGroup(MSG_ID_ON_ORDER_RET, ordReq,
                                 SyncToRiskMgr::False, SyncToDB::True);
    return;
  }

  tdSrv_->getSHMSrvOfTDGW()->pushMsgWithZeroCopy(
      [&](void* shmBuf) {
        InitMsgBody(shmBuf, *ordReq);
#ifndef OPT_LOG
        LOG_I("Forward order {}",
              static_cast<OrderInfo*>(shmBuf)->toShortStr());
#endif
      },
      ordReq->acctId_, MSG_ID_ON_ORDER, sizeof(OrderInfo));

#ifdef PERF_TEST
  EXEC_PERF_TEST("Order", ordReq->orderTime_, 100, 10);
#endif
}

void StgEngTaskHandler::handleMsgIdOnCancelOrder(
    const AsyncTaskSPtr<SHMIPCTaskSPtr>& asyncTask) {
  auto ordReq = MakeMsgSPtrByTask<OrderInfo>(asyncTask->task_);

  if (tdSrv_->getTDGWGroup()->exists(ordReq->acctId_) == false) {
    LOG_W("Handle msg id on cancel order failed. {}", ordReq->toShortStr());
    ordReq->statusCode_ = SCODE_TD_SRV_TDGW_NOT_EXISTS;

    tdSrv_->getSHMSrvOfStgEng()->pushMsgWithZeroCopy(
        [&](void* shmBuf) {
          InitMsgBody(shmBuf, *ordReq);
          LOG_I("Forward cancel order ret {}",
                static_cast<OrderInfo*>(shmBuf)->toShortStr());
        },
        ordReq->stgId_, MSG_ID_ON_CANCEL_ORDER_RET, sizeof(OrderInfo));

    return;
  }

  const auto statusCode =
      tdSrv_->getTDSrvRiskPluginMgr()->onCancelOrder(ordReq);
  if (statusCode != 0) {
    LOG_W("Risk check cancel order failed. [{} - {}] {}", statusCode,
          GetStatusMsg(statusCode), ordReq->toShortStr());
    ordReq->statusCode_ = statusCode;

    tdSrv_->getSHMSrvOfStgEng()->pushMsgWithZeroCopy(
        [&](void* shmBuf) {
          InitMsgBody(shmBuf, *ordReq);
          LOG_I("Forward cancel order ret {}",
                static_cast<OrderInfo*>(shmBuf)->toShortStr());
        },
        ordReq->stgId_, MSG_ID_ON_CANCEL_ORDER_RET, sizeof(OrderInfo));

    return;
  }

  tdSrv_->getSHMSrvOfTDGW()->pushMsgWithZeroCopy(
      [&](void* shmBuf) {
        InitMsgBody(shmBuf, *ordReq);
#ifndef OPT_LOG
        LOG_I("Forward cancel order {}",
              static_cast<OrderInfo*>(shmBuf)->toShortStr());
#endif
      },
      ordReq->acctId_, MSG_ID_ON_CANCEL_ORDER, sizeof(OrderInfo));
}

void StgEngTaskHandler::handleMsgIdOnStgReg(
    const AsyncTaskSPtr<SHMIPCTaskSPtr>& asyncTask) {
  const auto reqHeader = static_cast<const SHMHeader*>(asyncTask->task_->data_);
  LOG_D("Recv msg {}. [channel = {}]", GetMsgName(reqHeader->msgId_),
        reqHeader->clientChannel_);

  tdSrv_->getStgEngGroup()->update(reqHeader->clientChannel_);
  tdSrv_->getSHMSrvOfStgEng()->sendRspWithZeroCopy([&](void* shmBuf) {},
                                                   reqHeader, sizeof(StgReg));
}

}  // namespace bq::td::srv
