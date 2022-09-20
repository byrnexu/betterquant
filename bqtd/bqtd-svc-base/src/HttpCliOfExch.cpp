/*!
 * \file HttpCliOfExch.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "HttpCliOfExch.hpp"

#include "AssetsMgr.hpp"
#include "Config.hpp"
#include "OrdMgr.hpp"
#include "SHMIPC.hpp"
#include "TDSvc.hpp"
#include "TDSvcUtil.hpp"
#include "WSCliOfExch.hpp"
#include "def/BQDef.hpp"
#include "def/DataStruOfOthers.hpp"
#include "def/TDWSCliAsyncTaskArg.hpp"
#include "def/TaskOfSync.hpp"
#include "util/ExternalStatusCodeCache.hpp"
#include "util/Logger.hpp"
#include "util/TaskDispatcher.hpp"

namespace bq::td::svc {

void HttpCliOfExch::handleRspOfOrder(OrderInfoSPtr ordReq, cpr::Response rsp) {
  const auto [failed, externalStatusCode, externalStatusMsg] =
      rspOfOrderIsFailed(rsp.text);
  if (failed) {
    const auto statusMsg =
        fmt::format("Handle order status rsp of failed. {} {}", rsp.text,
                    ordReq->toShortStr());
    LOG_W(statusMsg);

    ordReq->statusCode_ =
        tdSvc_->getExternalStatusCodeCache()->getAndSetStatusCodeIfNotExists(
            tdSvc_->getMarketCode(), tdSvc_->getSymbolType(),
            fmt::format("{}", externalStatusCode), externalStatusMsg, -1);

    if (ordReq->statusCode_ != 0) {
      ordReq->orderStatus_ = OrderStatus::Failed;

      const auto [isTheOrderInfoUpdated, orderInfoInOrdMgr] =
          tdSvc_->getOrdMgr()->updateByOrderInfoFromExch(
              ordReq, tdSvc_->getNextNoUsedToCalcPos(), DeepClone::True);
      if (isTheOrderInfoUpdated == IsSomeFieldOfOrderUpdated::False) {
        return;
      }

      tdSvc_->getSHMCliOfTDSrv()->asyncSendMsgWithZeroCopy(
          [&](void* shmBuf) { InitMsgBody(shmBuf, *orderInfoInOrdMgr); },
          MSG_ID_ON_ORDER_RET, sizeof(OrderInfo));

      tdSvc_->cacheTaskOfSyncGroup(MSG_ID_ON_ORDER_RET, orderInfoInOrdMgr,
                                   SyncToRiskMgr::True, SyncToDB::True);
    }

    return;
  }

  LOG_D("{} {}", rsp.text, ordReq->toShortStr());
}

void HttpCliOfExch::handleRspOfCancelOrder(OrderInfoSPtr ordReq,
                                           cpr::Response rsp) {
  const auto [failed, externalStatusCode, externalStatusMsg] =
      rspOfCancelOrderIsFailed(rsp.text);
  if (failed) {
    const auto statusMsg =
        fmt::format("Handle cancel order rsp of failed. {}", rsp.text);
    LOG_W("{} {}", statusMsg, ordReq->toShortStr());

    ordReq->statusCode_ =
        tdSvc_->getExternalStatusCodeCache()->getAndSetStatusCodeIfNotExists(
            tdSvc_->getMarketCode(), tdSvc_->getSymbolType(),
            fmt::format("{}", externalStatusCode), externalStatusMsg, -1);

    tdSvc_->getSHMCliOfTDSrv()->asyncSendMsgWithZeroCopy(
        [&](void* shmBuf) { InitMsgBody(shmBuf, *ordReq); },
        MSG_ID_ON_CANCEL_ORDER_RET, sizeof(OrderInfo));

    // not sync to db
    tdSvc_->cacheTaskOfSyncGroup(MSG_ID_ON_CANCEL_ORDER_RET, ordReq,
                                 SyncToRiskMgr::True, SyncToDB::False);
    return;
  }
}

void HttpCliOfExch::syncAssetsSnapshot() {
  const auto assetInfoGroupFromExch = doSyncAssetsSnapshot();
  if (assetInfoGroupFromExch.empty()) {
    return;
  }

  AssetInfoGroupSPtr assetInfoGroup = std::make_shared<AssetInfoGroup>();
  for (const auto& rec : assetInfoGroupFromExch) {
    rec->initKeyHash();
    assetInfoGroup->emplace(rec->keyHash_, rec);
  }

  const auto updateInfoOfAssetGroup =
      tdSvc_->getAssetsMgr()->compareWithAssetsSnapshot(assetInfoGroup);

  if (!updateInfoOfAssetGroup->empty()) {
    NotifyAssetInfo(tdSvc_->getSHMCliOfTDSrv(), tdSvc_->getAcctId(),
                    updateInfoOfAssetGroup);

    tdSvc_->cacheTaskOfSyncGroup(MSG_ID_SYNC_ASSETS, updateInfoOfAssetGroup,
                                 SyncToRiskMgr::True, SyncToDB::True);
    updateInfoOfAssetGroup->print();
  }
}

void HttpCliOfExch::syncUnclosedOrderInfo(
    SHMIPCAsyncTaskSPtr& shmIPCAsyncTask) {
  const auto orderInfoFromExch = doSyncUnclosedOrderInfo(shmIPCAsyncTask);
  if (!orderInfoFromExch) {
    return;
  }
  const auto wsCliAsyncTaskArg = std::make_shared<WSCliAsyncTaskArg>(
      WSMsgType::SyncUnclosedOrder, orderInfoFromExch);
  auto asyncTask = std::make_shared<WSCliAsyncTask>(nullptr, wsCliAsyncTaskArg);
  tdSvc_->getWSCliOfExch()->getTaskDispatcher()->dispatch(asyncTask);
}

}  // namespace bq::td::svc
