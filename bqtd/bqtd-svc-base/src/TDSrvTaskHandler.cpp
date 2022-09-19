#include "TDSrvTaskHandler.hpp"

#include "AssetsMgr.hpp"
#include "ExceedFlowCtrlHandler.hpp"
#include "HttpCliOfExch.hpp"
#include "OrdMgr.hpp"
#include "PosMgr.hpp"
#include "SHMIPC.hpp"
#include "TDSvc.hpp"
#include "TDSvcDef.hpp"
#include "db/TBLMonitorOfSymbolInfo.hpp"
#include "def/BQDef.hpp"
#include "def/Const.hpp"
#include "def/DataStruOfMD.hpp"
#include "def/DataStruOfOthers.hpp"
#include "def/DataStruOfTD.hpp"
#include "def/TaskOfSync.hpp"
#include "util/Datetime.hpp"
#include "util/FlowCtrlSvc.hpp"
#include "util/Logger.hpp"
#include "util/TaskDispatcher.hpp"
#include "util/TrdSymbolCache.hpp"

namespace bq::td::svc {

TDSrvTaskHandler::TDSrvTaskHandler(TDSvc* tdSvc) : tdSvc_(tdSvc) {}

void TDSrvTaskHandler::handleAsyncTask(SHMIPCAsyncTaskSPtr& asyncTask) {
  const auto shmHeader = static_cast<const SHMHeader*>(asyncTask->task_->data_);
  switch (shmHeader->msgId_) {
    case MSG_ID_ON_ORDER:
      handleMsgIdOnOrder(asyncTask);
      break;
    case MSG_ID_ON_CANCEL_ORDER:
      handleMsgIdOnCancelOrder(asyncTask);
      break;

    case MSG_ID_SYNC_UNCLOSED_ORDER_INFO:
      handleMsgIdSyncUnclosedOrderInfo(asyncTask);
      break;
    case MSG_ID_SYNC_ASSETS_SNAPSHOT:
      handleMsgIdSyncAssetsSnapshot(asyncTask);
      break;
    case MSG_ID_EXTEND_CONN_LIFECYCLE:
      handleMsgIdExtenConnLifecycle(asyncTask);
      break;

    case MSG_ID_ON_TDGW_REG:
      handleMsgIdOnTDGWReg(asyncTask);
      break;

    case MSG_ID_ON_TEST_ORDER:
      handleMsgIdTestOrder(asyncTask);
      break;
    case MSG_ID_ON_TEST_CANCEL_ORDER:
      handleMsgIdTestCancelOrder(asyncTask);
      break;

    default:
      LOG_W("Unable to process msgId {}.", shmHeader->msgId_);
      break;
  }
}

void TDSrvTaskHandler::handleMsgIdOnOrder(SHMIPCAsyncTaskSPtr& asyncTask) {
  auto ordReq = MakeMsgSPtrByTask<OrderInfo>(asyncTask->task_);
#ifndef OPT_LOG
  LOG_I("Recv order {}", ordReq->toShortStr());
#endif

  bool exceedFlowCtrl =
      tdSvc_->getFlowCtrlSvc()->exceedFlowCtrl(GetMsgName(MSG_ID_ON_ORDER));
  if (exceedFlowCtrl) {
    LOG_W("Order exceed flow ctrl. {}", ordReq->toShortStr());

    ordReq->orderStatus_ = OrderStatus::Failed;
    ordReq->statusCode_ = SCODE_TD_SVC_EXCEED_FLOW_CTRL;

    tdSvc_->getSHMCliOfTDSrv()->asyncSendMsgWithZeroCopy(
        [&](void* shmBuf) { InitMsgBody(shmBuf, *ordReq); },
        MSG_ID_ON_ORDER_RET, sizeof(OrderInfo));

    tdSvc_->cacheTaskOfSyncGroup(MSG_ID_ON_ORDER_RET, ordReq,
                                 SyncToRiskMgr::True, SyncToDB::True);
    return;
  }

  tdSvc_->getTrdSymbolCache()->add(ordReq, SyncToDB::True);

  ordReq->orderStatus_ = OrderStatus::Pending;
  if (const auto ret = tdSvc_->getOrdMgr()->add(ordReq, DeepClone::True);
      ret != 0) {
    LOG_W("Handle op order failed. {}", ordReq->toShortStr());
    return;
  }

  tdSvc_->getSHMCliOfTDSrv()->asyncSendMsgWithZeroCopy(
      [&](void* shmBuf) {
        InitMsgBody(shmBuf, *ordReq);
#ifndef OPT_LOG
        LOG_I("Send order ret. {}",
              static_cast<OrderInfo*>(shmBuf)->toShortStr());
#endif
      },
      MSG_ID_ON_ORDER_RET, sizeof(OrderInfo));

  tdSvc_->cacheTaskOfSyncGroup(MSG_ID_ON_ORDER_RET,
                               std::make_shared<OrderInfo>(*ordReq),
                               SyncToRiskMgr::True, SyncToDB::True);

#ifdef PERF_TEST
  EXEC_PERF_TEST("Order", ordReq->orderTime_, 100, 10);
  return;
#endif

  if (const auto ret = tdSvc_->getHttpCliOfExch()->order(
          std::make_shared<OrderInfo>(*ordReq));
      ret != 0) {
    LOG_W("Handle op order failed. {}", ordReq->toShortStr());
    ordReq->orderStatus_ = OrderStatus::Failed;
    ordReq->statusCode_ = ret;
    tdSvc_->getOrdMgr()->remove(ordReq->orderId_);

    tdSvc_->getSHMCliOfTDSrv()->asyncSendMsgWithZeroCopy(
        [&](void* shmBuf) { InitMsgBody(shmBuf, *ordReq); },
        MSG_ID_ON_ORDER_RET, sizeof(OrderInfo));

    tdSvc_->cacheTaskOfSyncGroup(MSG_ID_ON_ORDER_RET, ordReq,
                                 SyncToRiskMgr::True, SyncToDB::True);
    return;
  }
}

void TDSrvTaskHandler::handleMsgIdOnCancelOrder(
    SHMIPCAsyncTaskSPtr& asyncTask) {
  auto ordReq = MakeMsgSPtrByTask<OrderInfo>(asyncTask->task_);
  LOG_I("Recv cancel order {}", ordReq->toShortStr());

  bool exceedFlowCtrl = tdSvc_->getFlowCtrlSvc()->exceedFlowCtrl(
      GetMsgName(MSG_ID_ON_CANCEL_ORDER));
  if (exceedFlowCtrl) {
    LOG_W("Cancel order exceed flow ctrl. {}", ordReq->toShortStr());
    ordReq->statusCode_ = SCODE_TD_SVC_EXCEED_FLOW_CTRL;

    tdSvc_->getSHMCliOfTDSrv()->asyncSendMsgWithZeroCopy(
        [&](void* shmBuf) { InitMsgBody(shmBuf, ordReq.get()); },
        MSG_ID_ON_CANCEL_ORDER_RET, sizeof(OrderInfo));

    // not sync to db
    tdSvc_->cacheTaskOfSyncGroup(MSG_ID_ON_CANCEL_ORDER_RET, ordReq,
                                 SyncToRiskMgr::True, SyncToDB::False);
    return;
  }

  if (const auto ret = tdSvc_->getHttpCliOfExch()->cancelOrder(
          std::make_shared<OrderInfo>(*ordReq));
      ret != 0) {
    LOG_W("Handle op cancel order failed. {}", ordReq->toShortStr());
    ordReq->statusCode_ = ret;

    tdSvc_->getSHMCliOfTDSrv()->asyncSendMsgWithZeroCopy(
        [&](void* shmBuf) { InitMsgBody(shmBuf, ordReq.get()); },
        MSG_ID_ON_CANCEL_ORDER_RET, sizeof(OrderInfo));

    // not sync to db
    tdSvc_->cacheTaskOfSyncGroup(MSG_ID_ON_CANCEL_ORDER_RET, ordReq,
                                 SyncToRiskMgr::True, SyncToDB::False);
    return;
  }
}

void TDSrvTaskHandler::handleMsgIdSyncUnclosedOrderInfo(
    SHMIPCAsyncTaskSPtr& asyncTask) {
  const auto tdSrvSignal = static_cast<TDSrvSignal*>(asyncTask->task_->data_);
  LOG_D("Recv msg {}. [channel = {}]",
        GetMsgName(tdSrvSignal->shmHeader_.msgId_),
        tdSrvSignal->shmHeader_.clientChannel_);

  bool exceedFlowCtrl = tdSvc_->getFlowCtrlSvc()->exceedFlowCtrl(
      GetMsgName(MSG_ID_SYNC_UNCLOSED_ORDER_INFO));
  if (exceedFlowCtrl) {
    tdSvc_->getExceedFlowCtrlHandler()->saveExceedFlowCtrlTask(asyncTask);
    return;
  }

  tdSvc_->getHttpCliOfExch()->syncUnclosedOrderInfo(asyncTask);
}

void TDSrvTaskHandler::handleMsgIdSyncAssetsSnapshot(
    SHMIPCAsyncTaskSPtr& asyncTask) {
  const auto tdSrvSignal = static_cast<TDSrvSignal*>(asyncTask->task_->data_);
  LOG_D("Recv msg {}. [channel = {}]",
        GetMsgName(tdSrvSignal->shmHeader_.msgId_),
        tdSrvSignal->shmHeader_.clientChannel_);

  bool exceedFlowCtrl = tdSvc_->getFlowCtrlSvc()->exceedFlowCtrl(
      GetMsgName(MSG_ID_SYNC_ASSETS_SNAPSHOT));
  if (exceedFlowCtrl) {
    tdSvc_->getExceedFlowCtrlHandler()->saveExceedFlowCtrlTask(asyncTask);
    return;
  }

  tdSvc_->getHttpCliOfExch()->syncAssetsSnapshot();
}

void TDSrvTaskHandler::handleMsgIdExtenConnLifecycle(
    SHMIPCAsyncTaskSPtr& asyncTask) {
  const auto tdSrvSignal = static_cast<TDSrvSignal*>(asyncTask->task_->data_);
  LOG_D("Recv msg {}. [channel = {}]",
        GetMsgName(tdSrvSignal->shmHeader_.msgId_),
        tdSrvSignal->shmHeader_.clientChannel_);

  bool exceedFlowCtrl = tdSvc_->getFlowCtrlSvc()->exceedFlowCtrl(
      GetMsgName(MSG_ID_EXTEND_CONN_LIFECYCLE));
  if (exceedFlowCtrl) {
    tdSvc_->getExceedFlowCtrlHandler()->saveExceedFlowCtrlTask(asyncTask);
    return;
  }

  tdSvc_->getHttpCliOfExch()->extendConnLifecycle();
}

void TDSrvTaskHandler::handleMsgIdOnTDGWReg(SHMIPCAsyncTaskSPtr& asyncTask) {
  const auto msgHeader = static_cast<const SHMHeader*>(asyncTask->task_->data_);
  LOG_D("Recv msg {}. [channel = {}]", GetMsgName(msgHeader->msgId_),
        msgHeader->clientChannel_);
}

void TDSrvTaskHandler::handleMsgIdTestOrder(SHMIPCAsyncTaskSPtr& asyncTask) {
  LOG_D("{} trigged.", __func__);
  tdSvc_->getHttpCliOfExch()->testOrder();
}

void TDSrvTaskHandler::handleMsgIdTestCancelOrder(
    SHMIPCAsyncTaskSPtr& asyncTask) {
  LOG_D("{} trigged.", __func__);
  tdSvc_->getHttpCliOfExch()->testCancelOrder();
}

}  // namespace bq::td::svc
