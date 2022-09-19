#include "TDGWTaskHandler.hpp"

#include "AssetsMgr.hpp"
#include "ClientChannelGroup.hpp"
#include "OrdMgr.hpp"
#include "PosMgr.hpp"
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

TDGWTaskHandler::TDGWTaskHandler(RiskMgr* riskMgr) : riskMgr_(riskMgr) {}

void TDGWTaskHandler::handleAsyncTask(const SHMIPCAsyncTaskSPtr& asyncTask) {
  const auto shmHeader = static_cast<const SHMHeader*>(asyncTask->task_->data_);
  switch (shmHeader->msgId_) {
    case MSG_ID_ON_ORDER_RET:
      handleMsgIdOnOrderRet(asyncTask);
      break;
    case MSG_ID_ON_CANCEL_ORDER_RET:
      handleMsgIdOnCancelOrderRet(asyncTask);
      break;
    case MSG_ID_SYNC_ASSETS:
      handleMsgIdSyncAssets(asyncTask);
      break;
    case MSG_ID_ON_TDGW_REG:
      handleMsgIdOnTDGWReg(asyncTask);
      break;
    default:
      LOG_W("Unable to process msgId {}.", shmHeader->msgId_);
      break;
  }
}

void TDGWTaskHandler::handleMsgIdOnOrderRet(
    const SHMIPCAsyncTaskSPtr& asyncTask) {
  auto ordRet = MakeMsgSPtrByTask<OrderInfo>(asyncTask->task_);
  LOG_I("Recv order ret {}", ordRet->toShortStr());

  if (ordRet->orderStatus_ == OrderStatus::Pending) {
    riskMgr_->getOrdMgr()->add(ordRet, DeepClone::False, LockFunc::True);
  } else {
    const auto [isTheOrderCanBeUsedCalcPos, orderInfoInOrdMgr] =
        riskMgr_->getOrdMgr()->updateByOrderInfoFromTDGW(ordRet,
                                                         LockFunc::True);
    if (isTheOrderCanBeUsedCalcPos == IsTheOrderCanBeUsedCalcPos::True) {
      riskMgr_->getPosMgr()->updateByOrderInfoFromTDGW(ordRet, LockFunc::True);
    }

    // TODO 通知此账号的仓位信息
  }
}

void TDGWTaskHandler::handleMsgIdOnCancelOrderRet(
    const SHMIPCAsyncTaskSPtr& asyncTask) {
  auto ordRet = MakeMsgSPtrByTask<OrderInfo>(asyncTask->task_);
  LOG_I("Recv cancel order ret {}", ordRet->toShortStr());
}

void TDGWTaskHandler::handleMsgIdSyncAssets(
    const SHMIPCAsyncTaskSPtr& asyncTask) {
  auto assetInfoNotify = MakeMsgSPtrByTask<AssetInfoNotify>(asyncTask->task_);
  LOG_D("Recv msg {}. ", assetInfoNotify->shmHeader_.toStr());

  const auto updateInfoOfAssetGroup =
      GetUpdateInfoOfAssetGroup(assetInfoNotify);

  for (const auto& assetInfo : *updateInfoOfAssetGroup->assetInfoGroupAdd_) {
    riskMgr_->getAssetsMgr()->add(assetInfo, LockFunc::True);
    LOG_I("Add asset {}", assetInfo->toStr());
  }
  for (const auto& assetInfo : *updateInfoOfAssetGroup->assetInfoGroupDel_) {
    riskMgr_->getAssetsMgr()->remove(assetInfo, LockFunc::True);
    LOG_I("Del asset {}", assetInfo->toStr());
  }
  for (const auto& assetInfo : *updateInfoOfAssetGroup->assetInfoGroupChg_) {
    riskMgr_->getAssetsMgr()->update(assetInfo, LockFunc::True);
    LOG_I("Chg asset {}", assetInfo->toStr());
  }

  LOG_D("Sync asset info group. {}",
        riskMgr_->getAssetsMgr()->toStr(LockFunc::True));

  // TODO 通知此账号的资产信息
}

void TDGWTaskHandler::handleMsgIdOnTDGWReg(
    const SHMIPCAsyncTaskSPtr& asyncTask) {
  const auto msgHeader = static_cast<const SHMHeader*>(asyncTask->task_->data_);
  LOG_D("Recv msg {}. [channel = {}]", GetMsgName(msgHeader->msgId_),
        msgHeader->clientChannel_);
  riskMgr_->getTDGWGroup()->update(msgHeader->clientChannel_);
}

}  // namespace bq::riskmgr
