/*!
 * \file StgInstTaskHandlerImpl.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "StgInstTaskHandlerImpl.hpp"

#include "OrdMgr.hpp"
#include "PosMgr.hpp"
#include "SHMHeader.hpp"
#include "SHMIPCTask.hpp"
#include "SHMIPCUtil.hpp"
#include "StgEngImpl.hpp"
#include "StgEngUtil.hpp"
#include "db/TBLMonitorOfStgInstInfo.hpp"
#include "db/TBLMonitorOfSymbolInfo.hpp"
#include "def/BQDef.hpp"
#include "def/CommonIPCData.hpp"
#include "def/DataStruOfAssets.hpp"
#include "def/DataStruOfMD.hpp"
#include "def/DataStruOfStg.hpp"
#include "def/DataStruOfTD.hpp"
#include "util/Datetime.hpp"
#include "util/MarketDataCache.hpp"
#include "util/PosSnapshot.hpp"
#include "util/TaskDispatcher.hpp"

namespace bq::stg {

StgInstTaskHandlerImpl::StgInstTaskHandlerImpl(
    StgEngImpl* stgEng,
    const StgInstTaskHandlerBundle& stgInstTaskHandlerBundle)
    : stgEng_(stgEng), stgInstTaskHandlerBundle_(stgInstTaskHandlerBundle) {}

void StgInstTaskHandlerImpl::handleAsyncTask(
    const SHMIPCAsyncTaskSPtr& asyncTask) {
  const auto stgInstId = std::any_cast<StgInstId>(asyncTask->arg_);
  const auto [ret, stgInstInfo] =
      getStgEngImpl()->getTBLMonitorOfStgInstInfo()->getStgInstInfo(stgInstId);
  if (ret == 0) {
    handleAsyncTaskImpl(stgInstInfo, asyncTask);
  } else {
    LOG_W("Get stg inst info of {} - {} failed. ", stgEng_->getStgId(),
          stgInstId);
  }
}

void StgInstTaskHandlerImpl::handleAsyncTaskImpl(
    const StgInstInfoSPtr& stgInstInfo, const SHMIPCAsyncTaskSPtr& asyncTask) {
  const auto data = asyncTask->task_->data_;
  const auto msgId = static_cast<const SHMHeader*>(data)->msgId_;
  switch (msgId) {
    case MSG_ID_ON_STG_MANUAL_INTERVENTION: {
      const auto commonIPCData =
          MakeMsgSPtrByTask<CommonIPCData>(asyncTask->task_);
      LOG_I("Handle manual intervention: {}", commonIPCData->data_);
      if (stgInstTaskHandlerBundle_.onStgManualIntervention_) {
        stgInstTaskHandlerBundle_.onStgManualIntervention_(stgInstInfo,
                                                           commonIPCData);
      }
    } break;

    case MSG_ID_ON_PUSH_TOPIC: {
      const auto topicContent =
          MakeMsgSPtrByTask<TopicContent>(asyncTask->task_);
      if (stgInstTaskHandlerBundle_.onPushTopic_) {
        stgInstTaskHandlerBundle_.onPushTopic_(stgInstInfo, topicContent);
      }
    } break;

    case MSG_ID_ON_ORDER_RET: {
      const auto ordRet = MakeMsgSPtrByTask<OrderInfo>(asyncTask->task_);
      LOG_I("Recv order ret {}", ordRet->toShortStr());
      beforeOnOrderRet(stgInstInfo, ordRet);
      if (stgInstTaskHandlerBundle_.onOrderRet_) {
        stgInstTaskHandlerBundle_.onOrderRet_(stgInstInfo, ordRet);
      }
    } break;

    case MSG_ID_ON_CANCEL_ORDER_RET: {
      const auto ordRet = MakeMsgSPtrByTask<OrderInfo>(asyncTask->task_);
      LOG_I("Recv cancel order ret {}", ordRet->toShortStr());
      if (stgInstTaskHandlerBundle_.onCancelOrderRet_) {
        stgInstTaskHandlerBundle_.onCancelOrderRet_(stgInstInfo, ordRet);
      }
    } break;

    case MSG_ID_ON_MD_TRADES: {
      const auto trades = MakeMsgSPtrByTask<Trades>(asyncTask->task_);
      stgEng_->getMarketDataCache()->cache(trades);
      if (stgInstTaskHandlerBundle_.onTrades_) {
        stgInstTaskHandlerBundle_.onTrades_(stgInstInfo, trades);
      }
    } break;

    case MSG_ID_ON_MD_TICKERS: {
      const auto tickers = MakeMsgSPtrByTask<Tickers>(asyncTask->task_);
      if (stgInstTaskHandlerBundle_.onTickers_) {
        stgInstTaskHandlerBundle_.onTickers_(stgInstInfo, tickers);
      }
    } break;

    case MSG_ID_ON_MD_CANDLE: {
      const auto candle = MakeMsgSPtrByTask<Candle>(asyncTask->task_);
      if (stgInstTaskHandlerBundle_.onCandle_) {
        stgInstTaskHandlerBundle_.onCandle_(stgInstInfo, candle);
      }
    } break;

    case MSG_ID_ON_MD_BOOKS: {
      const auto books = MakeMsgSPtrByTask<Books>(asyncTask->task_);
      if (stgInstTaskHandlerBundle_.onBooks_) {
        stgInstTaskHandlerBundle_.onBooks_(stgInstInfo, books);
      }
    } break;

    case MSG_ID_ON_STG_START:
      LOG_I("On stg {} start trigged. ", stgInstInfo->stgId_);
      if (stgInstTaskHandlerBundle_.onStgStart_) {
        stgInstTaskHandlerBundle_.onStgStart_();
      }
      getStgEngImpl()->getBarrierOfStgStartSignal()->set_value();
      break;

    case MSG_ID_ON_STG_INST_START:
      LOG_I("On stg inst start trigged. {}", stgInstInfo->toStr());
      if (stgInstTaskHandlerBundle_.onStgInstStart_) {
        stgInstTaskHandlerBundle_.onStgInstStart_(stgInstInfo);
      }
      break;

    case MSG_ID_ON_STG_INST_ADD:
      LOG_I("On stg inst add trigged. {}", stgInstInfo->toStr());
      if (stgInstTaskHandlerBundle_.onStgInstAdd_) {
        stgInstTaskHandlerBundle_.onStgInstAdd_(stgInstInfo);
      }
      break;

    case MSG_ID_ON_STG_INST_DEL:
      LOG_I("On stg inst del trigged. {}", stgInstInfo->toStr());
      if (stgInstTaskHandlerBundle_.onStgInstDel_) {
        stgInstTaskHandlerBundle_.onStgInstDel_(stgInstInfo);
      }
      break;

    case MSG_ID_ON_STG_INST_CHG:
      LOG_I("On stg inst chg trigged. {}", stgInstInfo->toStr());
      if (stgInstTaskHandlerBundle_.onStgInstChg_) {
        stgInstTaskHandlerBundle_.onStgInstChg_(stgInstInfo);
      }
      break;

    case MSG_ID_ON_STG_INST_TIMER: {
      const auto commonIPCData =
          MakeMsgSPtrByTask<CommonIPCData>(asyncTask->task_);
      const auto timerName = std::string(commonIPCData->data_);
      LOG_D("On timer of {} stg inst {}. {}", stgInstInfo->stgInstId_,
            timerName, stgInstInfo->toStr());
      if (stgInstTaskHandlerBundle_.onStgInstTimer_) {
        stgInstTaskHandlerBundle_.onStgInstTimer_(stgInstInfo, timerName);
      }
    } break;

    case MSG_ID_ON_STG_REG:
      LOG_D("On stg reg trigged. ");
      break;

    case MSG_ID_POS_UPDATE_OF_ACCT_ID: {
      const auto posUpdateOfAcctIdForPub =
          MakeMsgSPtrByTask<PosUpdateOfAcctIdForPub>(asyncTask->task_);
      const auto posUpdateOfAcctId =
          MakePosUpdateOfAcctId(posUpdateOfAcctIdForPub);
      const auto posSnapshot = std::make_shared<PosSnapshot>(
          std::move(posUpdateOfAcctId), stgEng_->getMarketDataCache());
      if (stgInstTaskHandlerBundle_.onPosUpdateOfAcctId_) {
        stgInstTaskHandlerBundle_.onPosUpdateOfAcctId_(stgInstInfo,
                                                       posSnapshot);
      }
    } break;

    case MSG_ID_POS_SNAPSHOT_OF_ACCT_ID: {
      const auto posUpdateOfAcctIdForPub =
          MakeMsgSPtrByTask<PosUpdateOfAcctIdForPub>(asyncTask->task_);
      const auto posSnapshotOfAcctId =
          MakePosUpdateOfAcctId(posUpdateOfAcctIdForPub);
      const auto posSnapshot = std::make_shared<PosSnapshot>(
          std::move(posSnapshotOfAcctId), stgEng_->getMarketDataCache());
      if (stgInstTaskHandlerBundle_.onPosSnapshotOfAcctId_) {
        stgInstTaskHandlerBundle_.onPosSnapshotOfAcctId_(stgInstInfo,
                                                         posSnapshot);
      }
    } break;

    case MSG_ID_POS_UPDATE_OF_STG_ID: {
      const auto posUpdateOfStgIdForPub =
          MakeMsgSPtrByTask<PosUpdateOfStgIdForPub>(asyncTask->task_);
      const auto posUpdateOfStgId =
          MakePosUpdateOfStgId(posUpdateOfStgIdForPub);
      const auto posSnapshot = std::make_shared<PosSnapshot>(
          std::move(posUpdateOfStgId), stgEng_->getMarketDataCache());
      if (stgInstTaskHandlerBundle_.onPosUpdateOfStgId_) {
        stgInstTaskHandlerBundle_.onPosUpdateOfStgId_(stgInstInfo, posSnapshot);
      }
    } break;

    case MSG_ID_POS_SNAPSHOT_OF_STG_ID: {
      const auto posUpdateOfStgIdForPub =
          MakeMsgSPtrByTask<PosUpdateOfStgIdForPub>(asyncTask->task_);
      const auto posSnapshotOfStgId =
          MakePosUpdateOfStgId(posUpdateOfStgIdForPub);
      const auto posSnapshot = std::make_shared<PosSnapshot>(
          std::move(posSnapshotOfStgId), stgEng_->getMarketDataCache());
      if (stgInstTaskHandlerBundle_.onPosSnapshotOfStgId_) {
        stgInstTaskHandlerBundle_.onPosSnapshotOfStgId_(stgInstInfo,
                                                        posSnapshot);
      }
    } break;

    case MSG_ID_POS_UPDATE_OF_STG_INST_ID: {
      const auto posUpdateOfStgInstIdForPub =
          MakeMsgSPtrByTask<PosUpdateOfStgInstIdForPub>(asyncTask->task_);
      const auto posUpdateOfStgInstId =
          MakePosUpdateOfStgInstId(posUpdateOfStgInstIdForPub);
      const auto posSnapshot = std::make_shared<PosSnapshot>(
          std::move(posUpdateOfStgInstId), stgEng_->getMarketDataCache());
      if (stgInstTaskHandlerBundle_.onPosUpdateOfStgInstId_) {
        stgInstTaskHandlerBundle_.onPosUpdateOfStgInstId_(stgInstInfo,
                                                          posSnapshot);
      }
    } break;

    case MSG_ID_POS_SNAPSHOT_OF_STG_INST_ID: {
      const auto posUpdateOfStgInstIdForPub =
          MakeMsgSPtrByTask<PosUpdateOfStgInstIdForPub>(asyncTask->task_);
      const auto posSnapshotOfStgInstId =
          MakePosUpdateOfStgInstId(posUpdateOfStgInstIdForPub);
      const auto posSnapshot = std::make_shared<PosSnapshot>(
          std::move(posSnapshotOfStgInstId), stgEng_->getMarketDataCache());
      if (stgInstTaskHandlerBundle_.onPosSnapshotOfStgInstId_) {
        stgInstTaskHandlerBundle_.onPosSnapshotOfStgInstId_(stgInstInfo,
                                                            posSnapshot);
      }
    } break;

    case MSG_ID_ASSETS_UPDATE: {
      const auto assetsUpdateForPub =
          MakeMsgSPtrByTask<AssetsUpdateForPub>(asyncTask->task_);
      const auto assetsUpdate = MakeAssetsUpdate(assetsUpdateForPub);
      if (stgInstTaskHandlerBundle_.onAssetsUpdate_) {
        stgInstTaskHandlerBundle_.onAssetsUpdate_(stgInstInfo, assetsUpdate);
      }
    } break;

    case MSG_ID_ASSETS_SNAPSHOT: {
      const auto assetsUpdateForPub =
          MakeMsgSPtrByTask<AssetsUpdateForPub>(asyncTask->task_);
      const auto assetsSnapshot = MakeAssetsUpdate(assetsUpdateForPub);
      if (stgInstTaskHandlerBundle_.onAssetsSnapshot_) {
        stgInstTaskHandlerBundle_.onAssetsSnapshot_(stgInstInfo,
                                                    assetsSnapshot);
      }
    } break;

    default: {
      const auto msgId = static_cast<const SHMHeader*>(data)->msgId_;
      LOG_I("On msg {} - {} of stg inst {}. {}", msgId, GetMsgName(msgId),
            stgInstInfo->stgInstId_, stgInstInfo->toStr());
      if (stgInstTaskHandlerBundle_.onOtherStgInstTask_) {
        stgInstTaskHandlerBundle_.onOtherStgInstTask_(stgInstInfo, asyncTask);
      }
    } break;
  }
}

void StgInstTaskHandlerImpl::beforeOnOrderRet(
    const StgInstInfoSPtr& stgInstInfo, const OrderInfoSPtr& orderInfo) {
  const auto orderRet = std::make_shared<OrderInfo>(*orderInfo);
  const auto [isTheOrderCanBeUsedCalcPos, orderInfoInOrdMgr] =
      getStgEngImpl()->getOrdMgr()->updateByOrderInfoFromTDGW(orderRet,
                                                              LockFunc::True);
  if (isTheOrderCanBeUsedCalcPos == IsTheOrderCanBeUsedCalcPos::True) {
    getStgEngImpl()->getPosMgr()->updateByOrderInfoFromTDGW(orderRet,
                                                            LockFunc::True);
  }
}

}  // namespace bq::stg
