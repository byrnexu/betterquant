/*!
 * \file WSCliOfExch.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "WSCliOfExch.hpp"

#include "AssetsMgr.hpp"
#include "Config.hpp"
#include "OrdMgr.hpp"
#include "SHMIPC.hpp"
#include "TDSvc.hpp"
#include "TDSvcUtil.hpp"
#include "WSCli.hpp"
#include "WSTask.hpp"
#include "WebConst.hpp"
#include "WebParam.hpp"
#include "def/DataStruOfOthers.hpp"
#include "def/DataStruOfTD.hpp"
#include "def/Def.hpp"
#include "def/TDWSCliAsyncTaskArg.hpp"
#include "def/TaskOfSync.hpp"
#include "util/Datetime.hpp"
#include "util/FlowCtrlSvc.hpp"
#include "util/Literal.hpp"
#include "util/String.hpp"
#include "util/TaskDispatcher.hpp"

namespace bq::td::svc {

WSCliOfExch::WSCliOfExch(TDSvc* tdSvc) : tdSvc_(tdSvc) {}

int WSCliOfExch::init() {
  if (const auto ret = initWSCli(); ret != 0) {
    return ret;
  }
  if (const auto ret = initTaskDispatcher(); ret != 0) {
    return ret;
  }
  taskDispatcher_->init();
  return 0;
}

int WSCliOfExch::initWSCli() {
  const auto wsParamInStrFmt =
      SetParam(web::DEFAULT_WS_PARAM, CONFIG["wsParam"].as<std::string>());
  const auto [ret, wsParam] = web::MakeWSParam(wsParamInStrFmt);
  if (ret != 0) {
    LOG_E("Init wscli failed. {}", wsParamInStrFmt);
    return ret;
  }

  wsCli_ = std::make_shared<web::WSCli>(
      wsParam,
      [this](auto* wsCli, const auto& connMetadata, const auto& msg) {
        OnWSCliMsg(wsCli, connMetadata, msg);
      },
      [this](auto* wsCli, const auto& connMetadata) {
        OnWSCliOpen(wsCli, connMetadata);
      },
      nullptr, nullptr, tdSvc_->getPingPongSvc());

  return 0;
}

int WSCliOfExch::initTaskDispatcher() {
  const auto wsTaskDispatcherParamInStrFmt =
      SetParam(DEFAULT_TASK_DISPATCHER_PARAM,
               CONFIG["wsTaskDispatcherParam"].as<std::string>());
  const auto [ret, wsTaskDispatcherParam] =
      MakeTaskDispatcherParam(wsTaskDispatcherParamInStrFmt);
  if (ret != 0) {
    LOG_E("Init taskdispatcher failed. {}", wsTaskDispatcherParamInStrFmt);
    return ret;
  }

  const auto getThreadForAsyncTask =
      [](const WSCliAsyncTaskSPtr& asyncTask,
         std::uint32_t taskSpecificThreadPoolSize) { return ThreadNo(0); };

  taskDispatcher_ = std::make_shared<TaskDispatcher<web::TaskFromSrvSPtr>>(
      wsTaskDispatcherParam,
      [this](const auto& task) { return makeAsyncTask(task); },
      getThreadForAsyncTask,
      [this](auto& asyncTask) { handleAsyncTask(asyncTask); });

  return ret;
}

int WSCliOfExch::start() {
  taskDispatcher_->start();
  const auto ret = wsCli_->start();
  if (ret != 0) {
    LOG_E("Start WSCliOfExch failed.");
    return ret;
  }
  return ret;
}

void WSCliOfExch::stop() {
  wsCli_->stop();
  taskDispatcher_->stop();
}

void WSCliOfExch::OnWSCliOpen(web::WSCli* wsCli,
                              const web::ConnMetadataSPtr& connMetadata) {
  onBeforeOpen(wsCli, connMetadata);
  tdSvc_->getFlowCtrlSvc()->reset();
}

void WSCliOfExch::OnWSCliMsg(web::WSCli* wsCli,
                             const web::ConnMetadataSPtr& connMetadata,
                             const web::MsgSPtr& msg) {
  auto task = std::make_shared<web::TaskFromSrv>(wsCli, connMetadata, msg);
  taskDispatcher_->dispatch(task);
}

std::tuple<int, WSCliAsyncTaskSPtr> WSCliOfExch::makeAsyncTask(
    const web::TaskFromSrvSPtr& task) {
  const auto asyncTaskArg = MakeWSCliAsyncTaskArg(task);
  if (asyncTaskArg == nullptr) {
    return {0, nullptr};
  }
  const auto asyncTask = std::make_shared<WSCliAsyncTask>(task, asyncTaskArg);
  return std::make_tuple(0, asyncTask);
}

void WSCliOfExch::handleAsyncTask(WSCliAsyncTaskSPtr& asyncTask) {
  const auto asyncTaskArg =
      std::any_cast<WSCliAsyncTaskArgSPtr>(asyncTask->arg_);

  switch (asyncTaskArg->wsMsgType_) {
    case WSMsgType::Order:
      handleOrder(asyncTask);
      break;

    case WSMsgType::SyncUnclosedOrder:
      handleSyncUnclosedOrder(asyncTask);
      break;
    case WSMsgType::SyncAssetsUpdate:
      handleSyncAssetsUpdate(asyncTask);
      break;

    default:
      LOG_W("Unhandled wsMsgType {}.",
            magic_enum::enum_name(asyncTaskArg->wsMsgType_));
      break;
  }
}

void WSCliOfExch::handleOrder(WSCliAsyncTaskSPtr& asyncTask) {
  const auto orderInfoFromExch = makeOrderInfoFromExch(asyncTask);
  if (!orderInfoFromExch) return;

  const auto [isTheOrderInfoUpdated, orderInfoInOrdMgr] =
      tdSvc_->getOrdMgr()->updateByOrderInfoFromExch(
          orderInfoFromExch, tdSvc_->getNextNoUsedToCalcPos(), DeepClone::True);
  if (isTheOrderInfoUpdated == IsSomeFieldOfOrderUpdated::False) {
    return;
  }

  tdSvc_->getSHMCliOfTDSrv()->asyncSendMsgWithZeroCopy(
      [&](void* shmBuf) {
        InitMsgBody(shmBuf, *orderInfoInOrdMgr);
        LOG_I("Send order ret. {}",
              static_cast<OrderInfo*>(shmBuf)->toShortStr());
      },
      MSG_ID_ON_ORDER_RET, sizeof(OrderInfo));

  tdSvc_->cacheTaskOfSyncGroup(MSG_ID_ON_ORDER_RET, orderInfoInOrdMgr,
                               SyncToRiskMgr::True, SyncToDB::True);
}

void WSCliOfExch::handleSyncUnclosedOrder(WSCliAsyncTaskSPtr& asyncTask) {
  const auto arg = std::any_cast<WSCliAsyncTaskArgSPtr>(asyncTask->arg_);
  const auto orderInfoFromExch = std::any_cast<OrderInfoSPtr>(arg->extData_);

  const auto [isTheOrderInfoUpdated, orderInfoInOrdMgr] =
      tdSvc_->getOrdMgr()->updateByOrderInfoFromExch(
          orderInfoFromExch, tdSvc_->getNextNoUsedToCalcPos(), DeepClone::True);
  if (isTheOrderInfoUpdated == IsSomeFieldOfOrderUpdated::False) {
    return;
  }

  tdSvc_->getSHMCliOfTDSrv()->asyncSendMsgWithZeroCopy(
      [&](void* shmBuf) {
        InitMsgBody(shmBuf, *orderInfoInOrdMgr);
        LOG_I("Send order ret of sync. {}",
              static_cast<OrderInfo*>(shmBuf)->toShortStr());
      },
      MSG_ID_ON_ORDER_RET, sizeof(OrderInfo));

  tdSvc_->cacheTaskOfSyncGroup(MSG_ID_ON_ORDER_RET, orderInfoInOrdMgr,
                               SyncToRiskMgr::True, SyncToDB::True);
}

void WSCliOfExch::handleSyncAssetsUpdate(WSCliAsyncTaskSPtr& asyncTask) {
  const auto assetsUpdate = makeAssetsUpdate(asyncTask);

  auto updateInfoOfAssetGroup = std::make_shared<UpdateInfoOfAssetGroup>();

  for (const auto& assetInfo : assetsUpdate) {
    const auto assetChgType =
        tdSvc_->getAssetsMgr()->compareWithAssetsUpdate(assetInfo);
    switch (assetChgType) {
      case AssetChgType::Add:
        updateInfoOfAssetGroup->assetInfoGroupAdd_->emplace_back(assetInfo);
        break;
      case AssetChgType::Del:
        updateInfoOfAssetGroup->assetInfoGroupDel_->emplace_back(assetInfo);
        break;
      case AssetChgType::Chg:
        updateInfoOfAssetGroup->assetInfoGroupChg_->emplace_back(assetInfo);
        break;
      default:
        break;
    }
  }

  if (!updateInfoOfAssetGroup->empty()) {
    NotifyAssetInfo(tdSvc_->getSHMCliOfTDSrv(), tdSvc_->getAcctId(),
                    updateInfoOfAssetGroup);

    tdSvc_->cacheTaskOfSyncGroup(MSG_ID_SYNC_ASSETS, updateInfoOfAssetGroup,
                                 SyncToRiskMgr::True, SyncToDB::True);
    updateInfoOfAssetGroup->print();
  }
}

}  // namespace bq::td::svc
