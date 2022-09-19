#include "TDGWTaskHandler.hpp"

#include "AssetsMgr.hpp"
#include "ClientChannelGroup.hpp"
#include "OrdMgr.hpp"
#include "PosMgr.hpp"
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
#include "def/TaskOfSync.hpp"
#include "util/Datetime.hpp"
#include "util/StdExt.hpp"
#include "util/TaskDispatcher.hpp"

namespace bq::td::srv {

TDGWTaskHandler::TDGWTaskHandler(TDSrv* tdSrv) : tdSrv_(tdSrv) {}

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
  const auto ordRet = MakeMsgSPtrByTask<OrderInfo>(asyncTask->task_);
#ifndef OPT_LOG
  LOG_I("Recv order ret {}", ordRet->toShortStr());
#endif

  tdSrv_->getSHMSrvOfStgEng()->pushMsgWithZeroCopy(
      [&](void* shmBuf) {
        InitMsgBody(shmBuf, *ordRet);
#ifndef OPT_LOG
        LOG_I("Forward order ret {}",
              static_cast<OrderInfo*>(shmBuf)->toShortStr());
#endif
      },
      ordRet->stgId_, MSG_ID_ON_ORDER_RET, sizeof(OrderInfo));

  const auto statusCode = tdSrv_->getTDSrvRiskPluginMgr()->onOrderRet(ordRet);
  if (statusCode != 0) {
    LOG_W("Risk check order ret failed. [{} - {}] {}", statusCode,
          GetStatusMsg(statusCode), ordRet->toShortStr());
  }

  const auto [isTheOrderCanBeUsedCalcPos, orderInfoInOrdMgr] =
      std::ext::tls_get<OrdMgr>().updateByOrderInfoFromTDGW(ordRet,
                                                            LockFunc::False);

  if (isTheOrderCanBeUsedCalcPos == IsTheOrderCanBeUsedCalcPos::True) {
    const auto posChgInfo =
        std::ext::tls_get<PosMgr>().updateByOrderInfoFromTDGW(ordRet);

    PosChgInfoSPtr posInfGroup = std::make_shared<PosChgInfo>();
    posInfGroup->reserve(posChgInfo->size());
    std::for_each(
        std::begin(*posChgInfo), std::end(*posChgInfo),
        [&](const auto& posInfo) {
          posInfGroup->emplace_back(std::make_shared<PosInfo>(*posInfo));
        });
    tdSrv_->cacheTaskOfSyncGroup(MSG_ID_SYNC_POS_INFO, posInfGroup,
                                 SyncToRiskMgr::False, SyncToDB::True);
  }
}

void TDGWTaskHandler::handleMsgIdOnCancelOrderRet(
    const SHMIPCAsyncTaskSPtr& asyncTask) {
  auto ordRet = MakeMsgSPtrByTask<OrderInfo>(asyncTask->task_);
  LOG_I("Recv cancel order ret {}", ordRet->toShortStr());

  tdSrv_->getSHMSrvOfStgEng()->pushMsgWithZeroCopy(
      [&](void* shmBuf) {
        InitMsgBody(shmBuf, *ordRet);
        LOG_I("Forward cancel order ret {}",
              static_cast<OrderInfo*>(shmBuf)->toShortStr());
      },
      ordRet->stgId_, MSG_ID_ON_CANCEL_ORDER_RET, sizeof(OrderInfo));

  const auto statusCode =
      tdSrv_->getTDSrvRiskPluginMgr()->onCancelOrderRet(ordRet);
  if (statusCode != 0) {
    LOG_W("Risk check order ret failed. [{} - {}] {}", statusCode,
          GetStatusMsg(statusCode), ordRet->toShortStr());
  }
}

void TDGWTaskHandler::handleMsgIdSyncAssets(
    const SHMIPCAsyncTaskSPtr& asyncTask) {
  auto assetInfoNotify = MakeMsgSPtrByTask<AssetInfoNotify>(asyncTask->task_);
  LOG_D("Recv msg {}. ", assetInfoNotify->shmHeader_.toStr());

  const auto updateInfoOfAssetGroup =
      GetUpdateInfoOfAssetGroup(assetInfoNotify);

  for (const auto& assetInfo : *updateInfoOfAssetGroup->assetInfoGroupAdd_) {
    std::ext::tls_get<AssetsMgr>().add(assetInfo, LockFunc::False);
    LOG_I("Add asset {}", assetInfo->toStr());
  }
  for (const auto& assetInfo : *updateInfoOfAssetGroup->assetInfoGroupDel_) {
    std::ext::tls_get<AssetsMgr>().remove(assetInfo, LockFunc::False);
    LOG_I("Del asset {}", assetInfo->toStr());
  }
  for (const auto& assetInfo : *updateInfoOfAssetGroup->assetInfoGroupChg_) {
    std::ext::tls_get<AssetsMgr>().update(assetInfo, LockFunc::False);
    LOG_I("Chg asset {}", assetInfo->toStr());
  }

  LOG_D("Sync asset info group. {}",
        std::ext::tls_get<AssetsMgr>().toStr(LockFunc::False));
}

void TDGWTaskHandler::handleMsgIdOnTDGWReg(
    const SHMIPCAsyncTaskSPtr& asyncTask) {
  const auto msgHeader = static_cast<const SHMHeader*>(asyncTask->task_->data_);
  LOG_D("Recv msg {}. [channel = {}]", GetMsgName(msgHeader->msgId_),
        msgHeader->clientChannel_);

  tdSrv_->getTDGWGroup()->update(msgHeader->clientChannel_);
  tdSrv_->getSHMSrvOfTDGW()->sendRspWithZeroCopy([&](void* shmBuf) {},
                                                 msgHeader, sizeof(TDGWReg));
}

}  // namespace bq::td::srv
