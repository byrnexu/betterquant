/*!
 * \file StgEngImpl.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "StgEngImpl.hpp"

#include "OrdMgr.hpp"
#include "PosMgr.hpp"
#include "SHMIPC.hpp"
#include "StgEngConst.hpp"
#include "StgEngUtil.hpp"
#include "StgInstTaskHandlerImpl.hpp"
#include "db/DBE.hpp"
#include "db/DBEngConst.hpp"
#include "db/TBLMonitorOfStgInstInfo.hpp"
#include "db/TBLMonitorOfSymbolInfo.hpp"
#include "def/AssetInfo.hpp"
#include "def/BQConst.hpp"
#include "def/BQDef.hpp"
#include "def/CommonIPCData.hpp"
#include "def/Const.hpp"
#include "def/DataStruOfOthers.hpp"
#include "def/DataStruOfStg.hpp"
#include "def/Def.hpp"
#include "def/Pnl.hpp"
#include "def/StatusCode.hpp"
#include "def/TaskOfSync.hpp"
#include "util/AcctInfoCache.hpp"
#include "util/File.hpp"
#include "util/Literal.hpp"
#include "util/MarketDataCache.hpp"
#include "util/MarketDataCond.hpp"
#include "util/Random.hpp"
#include "util/ScheduleTaskBundle.hpp"
#include "util/Scheduler.hpp"
#include "util/String.hpp"
#include "util/SubMgr.hpp"
#include "util/TaskDispatcher.hpp"
#include "util/Util.hpp"

namespace bq::stg {

StgEngImpl::StgEngImpl(const std::string& configFilename)
    : SvcBase(configFilename) {}

int StgEngImpl::prepareInit() {
  taskOfSyncGroup_.reserve(1024);

  try {
    config_ = YAML::LoadFile(configFilename_);
  } catch (const std::exception& e) {
    std::cerr << fmt::format("Open config filename {} failed. [{}]",
                             configFilename_, e.what())
              << std::endl;
    return SCODE_STG_ENG_INVALID_CONFIG_FILENAME;
  }

  const auto ret = InitLogger(configFilename_);
  if (ret != 0) {
    const auto statusMsg =
        fmt::format("Init stg {} failed because of init logger failed. {}",
                    getStgId(), configFilename_);
    std::cerr << statusMsg << std::endl;
    return ret;
  }

  return 0;
}

int StgEngImpl::doInit() {
  stgId_ = getConfig()["stgId"].as<StgId>();
  appName_ = fmt::format("Stg-{}", getStgId());
  rootDirOfStgPrivateData_ = fmt::format(
      "{}/{}", getConfig()["rootDirOfStgPrivateData"].as<std::string>(),
      getStgId());

  if (auto ret = initDBEng(); ret != 0) {
    LOG_E("[{}] Init failed.", appName_);
    return ret;
  }

  initTBLMonitorOfSymbolInfo();
  initTBLMonitorOfStgInstInfo();

  acctInfoCache_ = std::make_shared<AcctInfoCache>(getDBEng());
  marketDataCache_ = std::make_shared<MarketDataCache>();

  initSubMgr();
  initOrdMgr();
  initPosMgr();

  initStgInstTaskDispatcher();

  initSHMCliOfTDSrv();
  initSHMCliOfRiskMgr();
  initSHMCliOfWebSrv();

  scheduleTaskBundle_ = std::make_shared<ScheduleTaskBundle>();
  initScheduleTaskBundle();
  scheduleTaskBundleExecutor_ = std::make_shared<Scheduler>(
      getAppName(),
      [this]() {
        auto scheduleTaskBundle = getScheduleTaskBundle();
        ExecScheduleTaskBundle(scheduleTaskBundle);
      },
      1);

  return 0;
}

int StgEngImpl::initDBEng() {
  const auto dbEngParam = SetParam(db::DEFAULT_DB_ENG_PARAM,
                                   getConfig()["dbEngParam"].as<std::string>());
  int retOfMakeDBEng = 0;
  std::tie(retOfMakeDBEng, dbEng_) = db::MakeDBEng(
      dbEngParam, [this](db::DBTaskSPtr& dbTask, const StringSPtr& dbExecRet) {
        LOG_D("[{}] Exec sql finished. [{}] [exec result = {}]", appName_,
              dbTask->toStr(), *dbExecRet);
      });
  if (retOfMakeDBEng != 0) {
    LOG_E("[{}] Init dbeng failed. {}", appName_, dbEngParam);
    return retOfMakeDBEng;
  }

  if (auto retOfInit = getDBEng()->init(); retOfInit != 0) {
    LOG_E("[{}] Init dbeng failed. {}", appName_, dbEngParam);
    return retOfInit;
  }

  return 0;
}

void StgEngImpl::initTBLMonitorOfSymbolInfo() {
  const auto milliSecIntervalOfTBLMonitorOfSymbolInfo =
      getConfig()["milliSecIntervalOfTBLMonitorOfSymbolInfo"]
          .as<std::uint32_t>();
  const auto sql = fmt::format("SELECT * FROM {}", TBLSymbolInfo::TableName);
  tblMonitorOfSymbolInfo_ = std::make_shared<db::TBLMonitorOfSymbolInfo>(
      getDBEng(), milliSecIntervalOfTBLMonitorOfSymbolInfo, sql);
}

void StgEngImpl::initTBLMonitorOfStgInstInfo() {
  const auto cbOnStgInstInfoChg = [this](const auto& tblRecSetAdd,
                                         const auto& tblRecSetDel,
                                         const auto& tblRecSetChg) {
    for (const auto tblRec : *tblRecSetAdd) {
      const auto stgInstId = tblRec.second->getRecWithAllFields()->stgInstId;
      auto asynTask = MakeStgSignal(MSG_ID_ON_STG_INST_ADD, stgInstId);
      stgInstTaskDispatcher_->dispatch(asynTask);
    }
    for (const auto tblRec : *tblRecSetDel) {
      const auto stgInstId = tblRec.second->getRecWithAllFields()->stgInstId;
      auto asynTask = MakeStgSignal(MSG_ID_ON_STG_INST_DEL, stgInstId);
      stgInstTaskDispatcher_->dispatch(asynTask);
    }
    for (const auto tblRec : *tblRecSetChg) {
      const auto stgInstId = tblRec.second->getRecWithAllFields()->stgInstId;
      auto asynTask = MakeStgSignal(MSG_ID_ON_STG_INST_CHG, stgInstId);
      stgInstTaskDispatcher_->dispatch(asynTask);
    }
  };

  const auto milliSecIntervalOfTBLMonitorOfStgInstInfo =
      getConfig()["milliSecIntervalOfTBLMonitorOfStgInstInfo"]
          .as<std::uint32_t>();
  const auto sql = fmt::format(
      "SELECT a.`productId`, a.`stgId`, a.`stgName`, a.`userIdOfAuthor`, "
      "b.`stgInstId`, b.`stgInstParams`, b.`stgInstName`, b.`userId`, "
      "b.`isDel` FROM stgInfo a, stgInstInfo b WHERE a.`stgId` = {} AND "
      "a.`stgId` = b.`stgId` AND b.`isDel` = 0; ",
      getStgId());
  tblMonitorOfStgInstInfo_ = std::make_shared<db::TBLMonitorOfStgInstInfo>(
      getDBEng(), milliSecIntervalOfTBLMonitorOfStgInstInfo, sql,
      cbOnStgInstInfoChg);
}

void StgEngImpl::initSubMgr() {
  const auto onSHMDataRecv = [this](const void* shmBuf, std::size_t shmBufLen) {
    const auto shmHeader = static_cast<const SHMHeader*>(shmBuf);
    const auto subscriberGroup =
        subMgr_->getSubscriberGroupByTopicHash(shmHeader->topicHash_);
    for (auto stgInstId : subscriberGroup) {
      auto asyncTask = std::make_shared<SHMIPCAsyncTask>(
          std::make_shared<SHMIPCTask>(shmBuf, shmBufLen), stgInstId);
      stgInstTaskDispatcher_->dispatch(asyncTask);
    }
  };

  subMgr_ = std::make_shared<SubMgr>(appName_, onSHMDataRecv);
}

void StgEngImpl::initSHMCliOfTDSrv() {
  const auto stgEngChannelOfTDSrv =
      getConfig()["stgEngChannelOfTDSrv"].as<std::string>();
  const auto addr =
      fmt::format("{}{}{}", appName_, SEP_OF_SHM_SVC, stgEngChannelOfTDSrv);

  const auto onSHMDataRecv = [this](const void* shmBuf, std::size_t shmBufLen) {
    const auto shmHeader = static_cast<const SHMHeader*>(shmBuf);
    StgInstId stgInstId = 1;
    if (shmHeader->msgId_ == MSG_ID_ON_ORDER_RET ||
        shmHeader->msgId_ == MSG_ID_ON_CANCEL_ORDER_RET) {
      stgInstId = static_cast<const OrderInfo*>(shmBuf)->stgInstId_;
    }
    auto asyncTask = std::make_shared<SHMIPCAsyncTask>(
        std::make_shared<SHMIPCTask>(shmBuf, shmBufLen), stgInstId);
    stgInstTaskDispatcher_->dispatch(asyncTask);
  };

  shmCliOfTDSrv_ = std::make_shared<SHMCli>(addr, onSHMDataRecv);
  shmCliOfTDSrv_->setClientChannel(getStgId());
}

void StgEngImpl::initSHMCliOfRiskMgr() {
  const auto stgEngChannelOfRiskMgr =
      getConfig()["stgEngChannelOfRiskMgr"].as<std::string>();
  const auto addr =
      fmt::format("{}{}{}", appName_, SEP_OF_SHM_SVC, stgEngChannelOfRiskMgr);

  const auto onSHMDataRecv = [this](const void* shmBuf, std::size_t shmBufLen) {
    const auto shmHeader = static_cast<const SHMHeader*>(shmBuf);
    StgInstId stgInstId = 1;
    if (shmHeader->msgId_ == MSG_ID_ON_ORDER_RET ||
        shmHeader->msgId_ == MSG_ID_ON_CANCEL_ORDER_RET) {
      stgInstId = static_cast<const OrderInfo*>(shmBuf)->stgInstId_;
    }
    auto asyncTask = std::make_shared<SHMIPCAsyncTask>(
        std::make_shared<SHMIPCTask>(shmBuf, shmBufLen), stgInstId);
    stgInstTaskDispatcher_->dispatch(asyncTask);
  };

  shmCliOfRiskMgr_ = std::make_shared<SHMCli>(addr, onSHMDataRecv);
  shmCliOfRiskMgr_->setClientChannel(getStgId());
}

void StgEngImpl::initSHMCliOfWebSrv() {
  const auto stgEngChannelOfWebSrv =
      getConfig()["stgEngChannelOfWebSrv"].as<std::string>();
  const auto addr =
      fmt::format("{}{}{}", appName_, SEP_OF_SHM_SVC, stgEngChannelOfWebSrv);

  const auto onSHMDataRecv = [this](const void* shmBuf, std::size_t shmBufLen) {
    const auto msgId = static_cast<const SHMHeader*>(shmBuf)->msgId_;
    switch (msgId) {
      case MSG_ID_ON_STG_MANUAL_INTERVENTION: {
        const auto [statusCode, stgInstId] =
            GetStgInstId(static_cast<const CommonIPCData*>(shmBuf));
        if (statusCode != 0) {
          LOG_W("Get invalid stgInstId from common ipc data of {} - {}.", msgId,
                GetMsgName(msgId));
          return;
        }
        LOG_I("Dispatch manual intervention: {}",
              static_cast<const CommonIPCData*>(shmBuf)->data_);
        auto asyncTask = std::make_shared<SHMIPCAsyncTask>(
            std::make_shared<SHMIPCTask>(shmBuf, shmBufLen), stgInstId);
        stgInstTaskDispatcher_->dispatch(asyncTask);
      } break;

      default:
        LOG_W("Unhandled msgId {}.", msgId);
        break;
    }
  };

  shmCliOfWebSrv_ = std::make_shared<SHMCli>(addr, onSHMDataRecv);
  shmCliOfWebSrv_->setClientChannel(getStgId());
}

void StgEngImpl::initOrdMgr() {
  const auto filled = magic_enum::enum_integer(OrderStatus::Filled);
  ordMgr_ = std::make_shared<OrdMgr>();
  const auto sql = fmt::format(
      "SELECT * FROM `orderInfo` WHERE `stgId` = {} AND `orderStatus` < {}; ",
      getStgId(), filled);
  getOrdMgr()->init(getConfig(), getDBEng(), sql);
}

void StgEngImpl::initPosMgr() {
  const auto sql =
      fmt::format("SELECT * FROM `posInfo` WHERE `stgId` = {}", getStgId());
  posMgr_ = std::make_shared<PosMgr>();
  getPosMgr()->init(getConfig(), getDBEng(), sql);
}

int StgEngImpl::initStgInstTaskDispatcher() {
  const auto stgInstTaskDispatcherParamInStrFmt =
      SetParam(DEFAULT_TASK_DISPATCHER_PARAM,
               getConfig()["stgInstTaskDispatcherParam"].as<std::string>());
  const auto [ret, stgInstTaskDispatcherParam] =
      MakeTaskDispatcherParam(stgInstTaskDispatcherParamInStrFmt);
  if (ret != 0) {
    LOG_E("[{}] Init taskdispatcher failed. {}", appName_,
          stgInstTaskDispatcherParamInStrFmt);
    return ret;
  }

  const auto getThreadForAsyncTask = [](const auto& asyncTask,
                                        auto taskSpecificThreadPoolSize) {
    const auto stgInstId = std::any_cast<StgInstId>(asyncTask->arg_);
    return (stgInstId - 1) % taskSpecificThreadPoolSize;
  };

  const auto handleAsyncTask = [this](auto& asyncTask) {
    stgInstTaskHandler_->handleAsyncTask(asyncTask);
  };

  stgInstTaskDispatcher_ = std::make_shared<TaskDispatcher<SHMIPCTaskSPtr>>(
      stgInstTaskDispatcherParam, nullptr, getThreadForAsyncTask,
      handleAsyncTask);
  stgInstTaskDispatcher_->init();

  return ret;
}

void StgEngImpl::initScheduleTaskBundle() {
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxScheduleTaskBundle_);

    scheduleTaskBundle_->emplace_back(std::make_shared<ScheduleTask>(
        "reloadAcctInfoCache",
        [this]() {
          acctInfoCache_->reload();
          return true;
        },
        ExecAtStartup::True, MilliSecInterval(5000)));

    scheduleTaskBundle_->emplace_back(std::make_shared<ScheduleTask>(
        "sendStgReg",
        [this]() {
          sendStgReg();
          return true;
        },
        ExecAtStartup::False, MilliSecInterval(5000)));

    scheduleTaskBundle_->emplace_back(std::make_shared<ScheduleTask>(
        "syncOrderGroupToDB",
        [this]() {
          getOrdMgr()->syncOrderGroupToDB();
          return true;
        },
        ExecAtStartup::False, MilliSecInterval(3000)));

    const auto milliSecIntervalOfSyncTask =
        getConfig()["milliSecIntervalOfSyncTask"].as<std::uint32_t>();
    scheduleTaskBundle_->emplace_back(std::make_shared<ScheduleTask>(
        "syncTask",
        [this]() {
          handleTaskOfSyncGroup();
          return true;
        },
        ExecAtStartup::False, milliSecIntervalOfSyncTask));
  }
}

int StgEngImpl::doRun() {
  if (stgInstTaskHandler_ == nullptr) {
    LOG_E(
        "[{}] Start failed because of "
        "StgInstTaskHandler is null, please install first.",
        appName_);
    return SCODE_STG_INST_TASK_HANDLER_NOT_INSTALL;
  }

  getDBEng()->start();

  if (auto ret = tblMonitorOfSymbolInfo_->start(); ret != 0) {
    LOG_E("[{}] Start failed.", appName_);
    return ret;
  }

  if (auto ret = tblMonitorOfStgInstInfo_->start(); ret != 0) {
    LOG_E("[{}] Start failed.", appName_);
    return ret;
  }

  if (const auto [ret, stgInstInfo] =
          tblMonitorOfStgInstInfo_->getStgInstInfo(1);
      ret != 0) {
    LOG_E("[{}] The stg must have at least one instance with an id of 1.",
          appName_);
    return SCODE_STG_MUST_HAVE_STG_INST_1;
  }

  if (const auto [ret, stgInstInfo] =
          tblMonitorOfStgInstInfo_->getStgInstInfo(0);
      stgInstInfo != nullptr) {
    LOG_E("[{}] The id of the stg instance must start from 1", appName_);
    return SCODE_STG_INST_ID_MUST_START_FROM_1;
  }

  stgInstTaskDispatcher_->start();
  shmCliOfTDSrv_->start();
  shmCliOfRiskMgr_->start();
  shmCliOfWebSrv_->start();

  resetBarrierOfStgStartSignal();
  sendStgStartSignal();
  getBarrierOfStgStartSignal()->get_future().wait();
  sendStgInstStartSignal();

  sendStgReg();

  if (const auto ret = scheduleTaskBundleExecutor_->start(); ret != 0) {
    LOG_E("Start scheduler of multi task failed.");
    return ret;
  }

  return 0;
}

void StgEngImpl::sendStgStartSignal() {
  const StgInstId stgInstId = 1;
  auto asynTask = MakeStgSignal(MSG_ID_ON_STG_START, stgInstId);
  stgInstTaskDispatcher_->dispatch(asynTask);
}

void StgEngImpl::sendStgInstStartSignal() {
  const auto stgInstIdGroup = getTBLMonitorOfStgInstInfo()->getStgInstIdGroup();
  for (const auto stgInstId : stgInstIdGroup) {
    auto asynTask = MakeStgSignal(MSG_ID_ON_STG_INST_START, stgInstId);
    stgInstTaskDispatcher_->dispatch(asynTask);
  }
}

void StgEngImpl::sendStgReg() {
  shmCliOfTDSrv_->asyncSendReqWithZeroCopy(
      [&](void* shmBufOfReq) {
        auto stgReg = static_cast<StgReg*>(shmBufOfReq);
      },
      MSG_ID_ON_STG_REG, sizeof(StgReg));

  shmCliOfRiskMgr_->asyncSendReqWithZeroCopy(
      [&](void* shmBufOfReq) {
        auto stgReg = static_cast<StgReg*>(shmBufOfReq);
      },
      MSG_ID_ON_STG_REG, sizeof(StgReg));
}

void StgEngImpl::cacheTaskOfSyncGroup(MsgId msgId, const std::any& task,
                                      SyncToRiskMgr syncToRiskMgr,
                                      SyncToDB syncToDB) {
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxTaskOfSyncGroup_);
    taskOfSyncGroup_.emplace_back(
        std::make_shared<TaskOfSync>(msgId, task, syncToRiskMgr, syncToDB));
  }
}

void StgEngImpl::handleTaskOfSyncGroup() {
  std::vector<TaskOfSyncSPtr> taskGroup;
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxTaskOfSyncGroup_);
    std::swap(taskGroup, taskOfSyncGroup_);
  }

  if (taskGroup.size() > 100) {
    LOG_W("Too many unprocessed task of sync. [num = {}]", taskGroup.size());
  }

  for (const auto& rec : taskGroup) {
    if (rec->syncToRiskMgr_ == SyncToRiskMgr::False) continue;
    if (rec->msgId_ == MSG_ID_ON_ORDER || rec->msgId_ == MSG_ID_ON_ORDER_RET ||
        rec->msgId_ == MSG_ID_ON_CANCEL_ORDER ||
        rec->msgId_ == MSG_ID_ON_CANCEL_ORDER_RET) {
      const auto orderInfo = std::any_cast<OrderInfoSPtr>(rec->task_);
      shmCliOfRiskMgr_->asyncSendMsgWithZeroCopy(
          [&](void* shmBufOfReq) {
            InitMsgBody(shmBufOfReq, *orderInfo);
            LOG_I("Send order info to risk mgr. {}",
                  static_cast<OrderInfo*>(shmBufOfReq)->toShortStr());
          },
          rec->msgId_, sizeof(OrderInfo));
    } else {
      LOG_W("Unhandled task of sync to riskmgr. {} - {}", rec->msgId_,
            GetMsgName(rec->msgId_))
    }
  }

  for (const auto& rec : taskGroup) {
    if (rec->syncToDB_ == SyncToDB::False) continue;
    if (rec->msgId_ == MSG_ID_ON_ORDER || rec->msgId_ == MSG_ID_ON_ORDER_RET ||
        rec->msgId_ == MSG_ID_ON_CANCEL_ORDER ||
        rec->msgId_ == MSG_ID_ON_CANCEL_ORDER_RET) {
      const auto orderInfo = std::any_cast<OrderInfoSPtr>(rec->task_);
      const auto identity = GET_RAND_STR();
      const auto sql = orderInfo->getSqlOfUSPOrderInfoUpdate();
      const auto [ret, execRet] = getDBEng()->asyncExec(identity, sql);
      if (ret != 0) {
        LOG_W("Sync order info to db failed. [{}]", sql);
      }

    } else {
      LOG_W("Unhandled task of sync to db. {} - {}", rec->msgId_,
            GetMsgName(rec->msgId_))
    }
  }
}

void StgEngImpl::doExit(const boost::system::error_code* ec, int signalNum) {
  scheduleTaskBundleExecutor_->stop();
  getOrdMgr()->syncOrderGroupToDB();
  shmCliOfWebSrv_->stop();
  shmCliOfRiskMgr_->stop();
  shmCliOfTDSrv_->stop();
  stgInstTaskDispatcher_->stop();
  tblMonitorOfSymbolInfo_->stop();
  tblMonitorOfStgInstInfo_->stop();
  getDBEng()->stop();
}

std::tuple<int, OrderId> StgEngImpl::order(const StgInstInfoSPtr& stgInstInfo,
                                           AcctId acctId,
                                           const std::string& symbolCode,
                                           Side side, PosSide posSide,
                                           Decimal orderPrice,
                                           Decimal orderSize) {
  auto orderInfo = MakeOrderInfo(stgInstInfo, acctId, symbolCode, side, posSide,
                                 orderPrice, orderSize);
  return order(orderInfo);
}

std::tuple<int, OrderId> StgEngImpl::order(OrderInfoSPtr& orderInfo) {
  int retOfGetMarketCodeAndSymbolType = 0;
  std::tie(retOfGetMarketCodeAndSymbolType, orderInfo->marketCode_,
           orderInfo->symbolType_) =
      acctInfoCache_->getMarketCodeAndSymbolType(orderInfo->acctId_);
  if (retOfGetMarketCodeAndSymbolType != 0) {
    orderInfo->orderStatus_ = OrderStatus::Failed;
    orderInfo->statusCode_ = retOfGetMarketCodeAndSymbolType;
    LOG_W("[{}] Order failed. [{} - {}] {}", appName_, orderInfo->statusCode_,
          GetStatusMsg(orderInfo->statusCode_), orderInfo->toShortStr());
    return {retOfGetMarketCodeAndSymbolType, 0};
  }

  const auto [retOfGetSym, recSymbolInfo] =
      getTBLMonitorOfSymbolInfo()->getRecSymbolInfoBySymbolCode(
          GetMarketName(orderInfo->marketCode_), orderInfo->symbolCode_);
  if (retOfGetSym != 0) {
    orderInfo->orderStatus_ = OrderStatus::Failed;
    orderInfo->statusCode_ = retOfGetSym;
    LOG_W("[{}] Order failed. [{} - {}] {}", appName_, orderInfo->statusCode_,
          GetStatusMsg(orderInfo->statusCode_), orderInfo->toShortStr());
    return {retOfGetSym, 0};
  }

  const auto [retOfGetStgInst, stgInstInfo] =
      tblMonitorOfStgInstInfo_->getStgInstInfo(orderInfo->stgInstId_);
  if (retOfGetStgInst != 0) {
    orderInfo->orderStatus_ = OrderStatus::Failed;
    orderInfo->statusCode_ = retOfGetStgInst;
    LOG_W("[{}] Order failed. [{} - {}] {}", appName_, orderInfo->statusCode_,
          GetStatusMsg(orderInfo->statusCode_), orderInfo->toShortStr());
    return {retOfGetStgInst, 0};
  }

  if (orderInfo->symbolType_ == SymbolType::Spot) {
    orderInfo->posSide_ = PosSide::Both;
  }

  orderInfo->orderId_ = GET_RAND_INT();
  orderInfo->parValue_ = recSymbolInfo->parValue;
  strncpy(orderInfo->exchSymbolCode_, recSymbolInfo->exchSymbolCode.c_str(),
          sizeof(orderInfo->exchSymbolCode_) - 1);
  orderInfo->orderStatus_ = OrderStatus::Created;

  if (const auto ret = getOrdMgr()->add(orderInfo, DeepClone::True); ret != 0) {
    orderInfo->orderStatus_ = OrderStatus::Failed;
    orderInfo->statusCode_ = ret;
    LOG_W("[{}] Order failed. [{} - {}] {}", appName_, orderInfo->statusCode_,
          GetStatusMsg(orderInfo->statusCode_), orderInfo->toShortStr());
    return {ret, 0};
  }

#ifdef PERF_TEST
  EXEC_PERF_TEST("Order", orderInfo->orderTime_, 100, 10);
#endif

  shmCliOfTDSrv_->asyncSendMsgWithZeroCopy(
      [&](void* shmBufOfReq) {
        InitMsgBody(shmBufOfReq, *orderInfo);
#ifndef OPT_LOG
        LOG_I("Send order {}",
              static_cast<OrderInfo*>(shmBufOfReq)->toShortStr());
#endif
      },
      MSG_ID_ON_ORDER, sizeof(OrderInfo));
  cacheTaskOfSyncGroup(MSG_ID_ON_ORDER, orderInfo, SyncToRiskMgr::True,
                       SyncToDB::True);

#ifdef PERF_TEST
  EXEC_PERF_TEST("Order", orderInfo->orderTime_, 100, 10);
#endif
  return {0, orderInfo->orderId_};
}

int StgEngImpl::cancelOrder(OrderId orderId) {
  auto [ret, orderInfo] = getOrdMgr()->getOrderInfo(orderId, DeepClone::False);
  if (ret != 0) {
    auto orderInfo = std::make_shared<OrderInfo>();
    orderInfo->statusCode_ = ret;
    LOG_W("[{}] Cancel order {} failed. [{} - {}]", appName_, orderId,
          orderInfo->statusCode_, GetStatusMsg(orderInfo->statusCode_));
    return ret;
  }

  shmCliOfTDSrv_->asyncSendMsgWithZeroCopy(
      [&](void* shmBufOfReq) {
        InitMsgBody(shmBufOfReq, *orderInfo);
        LOG_I("Send cancel order {}",
              static_cast<OrderInfo*>(shmBufOfReq)->toShortStr());
      },
      MSG_ID_ON_CANCEL_ORDER, sizeof(OrderInfo));
  cacheTaskOfSyncGroup(MSG_ID_ON_CANCEL_ORDER, orderInfo, SyncToRiskMgr::True,
                       SyncToDB::False);

  return 0;
}

int StgEngImpl::sub(StgInstId subscriber, const std::string& topic) {
  return subMgr_->sub(subscriber, topic);
}

int StgEngImpl::unSub(StgInstId subscriber, const std::string& topic) {
  return subMgr_->unSub(subscriber, topic);
}

std::tuple<int, std::string> StgEngImpl::queryHisMDBetween2Ts(
    const std::string& topic, std::uint64_t tsBegin, std::uint64_t tsEnd,
    std::uint32_t level) {
  const auto [statusCode, marketDataCond] = getMarketDataCondFromTopic(topic);
  if (statusCode != 0) return {statusCode, ""};
  return queryHisMDBetween2Ts(
      marketDataCond->marketCode_, marketDataCond->symbolType_,
      marketDataCond->symbolCode_, marketDataCond->mdType_, tsBegin, tsEnd,
      marketDataCond->level_);
}

//
// http://192.168.19.113/v1/QueryHisMD/between/Binance/Spot/BTC-USDT/Trades?tsBegin=1668989747663000&tsEnd=1668989747697000
// http://192.168.19.113/v1/QueryHisMD/between/Binance/Spot/BTC-USDT/Books?level=20&tsBegin=1669032414507000&tsEnd=1669032415008000
//
std::tuple<int, std::string> StgEngImpl::queryHisMDBetween2Ts(
    MarketCode marketCode, SymbolType symbolType, const std::string& symbolCode,
    MDType mdType, std::uint64_t tsBegin, std::uint64_t tsEnd,
    std::uint32_t level) {
  std::string addr;
  if (mdType != MDType::Books) {
    addr = fmt::format(
        "{}/{}/{}/{}/{}?tsBegin={}&tsEnd={}", prefixOfQueryHisMDBetween,
        magic_enum::enum_name(marketCode), magic_enum::enum_name(symbolType),
        symbolCode, magic_enum::enum_name(mdType), tsBegin, tsEnd);
  } else {
    addr = fmt::format("{}/{}/{}/{}/{}?level={}&tsBegin={}&tsEnd={}",
                       prefixOfQueryHisMDBetween,
                       magic_enum::enum_name(marketCode),
                       magic_enum::enum_name(symbolType), symbolCode,
                       magic_enum::enum_name(mdType), level, tsBegin, tsEnd);
  }

  const auto timeoutOfQueryHisMD =
      getConfig()["timeoutOfQueryHisMD"].as<std::uint32_t>(60000);
  cpr::Response rsp =
      cpr::Get(cpr::Url{addr}, cpr::Timeout(timeoutOfQueryHisMD));
  if (rsp.status_code != cpr::status::HTTP_OK) {
    const auto statusMsg =
        fmt::format("Query his market data between 2 ts failed. [{}:{}] {} {}",
                    rsp.status_code, rsp.reason, rsp.text, rsp.url.str());
    LOG_W(statusMsg);
    return {SCODE_STG_SEND_HTTP_REQ_TO_QUERY_HIS_MD_FAILED, ""};
  } else {
    LOG_D("Send http req success. {}", addr);
  }

  return {0, rsp.text};
}

std::tuple<int, std::string> StgEngImpl::querySpecificNumOfHisMDBeforeTs(
    const std::string& topic, std::uint64_t ts, int num, std::uint32_t level) {
  const auto [statusCode, marketDataCond] = getMarketDataCondFromTopic(topic);
  if (statusCode != 0) return {statusCode, ""};
  return querySpecificNumOfHisMDBeforeTs(
      marketDataCond->marketCode_, marketDataCond->symbolType_,
      marketDataCond->symbolCode_, marketDataCond->mdType_, ts, num,
      marketDataCond->level_);
}

// http://192.168.19.113/v1/QueryHisMD/offset/Binance/Spot/BTC-USDT/Trades?ts=1668989747697000&offset=1
// http://192.168.19.113/v1/QueryHisMD/offset/Binance/Spot/BTC-USDT/Books?level=20&ts=1669032414507000&offset=1000
std::tuple<int, std::string> StgEngImpl::querySpecificNumOfHisMDBeforeTs(
    MarketCode marketCode, SymbolType symbolType, const std::string& symbolCode,
    MDType mdType, std::uint64_t ts, int num, std::uint32_t level) {
  std::string addr;
  if (mdType != MDType::Books) {
    addr = fmt::format(
        "{}/{}/{}/{}/{}?ts={}&offset={}", prefixOfQueryHisMDOffset,
        magic_enum::enum_name(marketCode), magic_enum::enum_name(symbolType),
        symbolCode, magic_enum::enum_name(mdType), ts, -1 * num);
  } else {
    addr = fmt::format(
        "{}/{}/{}/{}/{}?level={}&ts={}&offset={}", prefixOfQueryHisMDOffset,
        magic_enum::enum_name(marketCode), magic_enum::enum_name(symbolType),
        symbolCode, magic_enum::enum_name(mdType), level, ts, -1 * num);
  }

  const auto timeoutOfQueryHisMD =
      getConfig()["timeoutOfQueryHisMD"].as<std::uint32_t>(60000);
  cpr::Response rsp =
      cpr::Get(cpr::Url{addr}, cpr::Timeout(timeoutOfQueryHisMD));
  if (rsp.status_code != cpr::status::HTTP_OK) {
    const auto statusMsg = fmt::format(
        "Query specific num of his market data before ts failed. [{}:{}] {} {}",
        rsp.status_code, rsp.reason, rsp.text, rsp.url.str());
    LOG_W(statusMsg);
    return {SCODE_STG_SEND_HTTP_REQ_TO_QUERY_HIS_MD_FAILED, ""};
  } else {
    LOG_D("Send http req success. {}", addr);
  }

  return {0, rsp.text};
}

std::tuple<int, std::string> StgEngImpl::querySpecificNumOfHisMDAfterTs(
    const std::string& topic, std::uint64_t ts, int num, std::uint32_t level) {
  const auto [statusCode, marketDataCond] = getMarketDataCondFromTopic(topic);
  if (statusCode != 0) return {statusCode, ""};
  return querySpecificNumOfHisMDAfterTs(
      marketDataCond->marketCode_, marketDataCond->symbolType_,
      marketDataCond->symbolCode_, marketDataCond->mdType_, ts, num,
      marketDataCond->level_);
}

// http://192.168.19.113/v1/QueryHisMD/offset/Binance/Spot/BTC-USDT/Trades?ts=1668989747697000&offset=1
// http://192.168.19.113/v1/QueryHisMD/offset/Binance/Spot/BTC-USDT/Books?level=20&ts=1669032414507000&offset=1000
std::tuple<int, std::string> StgEngImpl::querySpecificNumOfHisMDAfterTs(
    MarketCode marketCode, SymbolType symbolType, const std::string& symbolCode,
    MDType mdType, std::uint64_t ts, int num, std::uint32_t level) {
  std::string addr;
  if (mdType != MDType::Books) {
    addr = fmt::format(
        "{}/{}/{}/{}/{}?ts={}&offset={}", prefixOfQueryHisMDOffset,
        magic_enum::enum_name(marketCode), magic_enum::enum_name(symbolType),
        symbolCode, magic_enum::enum_name(mdType), ts, num);
  } else {
    addr = fmt::format(
        "{}/{}/{}/{}/{}?level={}&ts={}&offset={}", prefixOfQueryHisMDOffset,
        magic_enum::enum_name(marketCode), magic_enum::enum_name(symbolType),
        symbolCode, magic_enum::enum_name(mdType), level, ts, num);
  }

  const auto timeoutOfQueryHisMD =
      getConfig()["timeoutOfQueryHisMD"].as<std::uint32_t>(60000);
  cpr::Response rsp =
      cpr::Get(cpr::Url{addr}, cpr::Timeout(timeoutOfQueryHisMD));
  if (rsp.status_code != cpr::status::HTTP_OK) {
    const auto statusMsg = fmt::format(
        "Query specific num of his market data after ts failed. [{}:{}] {} {}",
        rsp.status_code, rsp.reason, rsp.text, rsp.url.str());
    LOG_W(statusMsg);
    return {SCODE_STG_SEND_HTTP_REQ_TO_QUERY_HIS_MD_FAILED, ""};
  } else {
    LOG_D("Send http req success. {}", addr);
  }

  return {0, rsp.text};
}

void StgEngImpl::installStgInstTimer(StgInstId stgInstId,
                                     const std::string& timerName,
                                     ExecAtStartup execAtStartUp,
                                     std::uint32_t milliSecInterval,
                                     std::uint64_t maxExecTimes) {
  const auto callback = [stgInstId, this]() {
    auto asynTask = MakeStgSignal(MSG_ID_ON_STG_INST_TIMER, stgInstId);
    stgInstTaskDispatcher_->dispatch(asynTask);
    return true;
  };

  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxScheduleTaskBundle_);
    scheduleTaskBundle_->emplace_back(std::make_shared<ScheduleTask>(
        timerName, callback, execAtStartUp, milliSecInterval, maxExecTimes));
    if (scheduleTaskBundle_->size() % 100 == 0) {
      LOG_W("[{}] Size of scheduleTaskBundle is {}", getAppName(),
            scheduleTaskBundle_->size());
    }
  }
}

bool StgEngImpl::saveStgPrivateData(StgInstId stgInstId,
                                    const std::string& jsonStr) {
  try {
    if (!boost::filesystem::exists(rootDirOfStgPrivateData_)) {
      boost::filesystem::create_directories(rootDirOfStgPrivateData_);
    }
  } catch (const std::exception& e) {
    LOG_W(
        "Save stg private data failed "
        "because of create directories {} failed. {}",
        rootDirOfStgPrivateData_, e.what());
    return false;
  }
  const auto filename =
      fmt::format("{}/{}.dat", rootDirOfStgPrivateData_, stgInstId);
  bool ret = OverwriteStrToFile(filename, jsonStr);
  return ret;
}

std::string StgEngImpl::loadStgPrivateData(StgInstId stgInstId) {
  const auto filename =
      fmt::format("{}/{}.dat", rootDirOfStgPrivateData_, stgInstId);
  const auto filecont = LoadFileContToStr(filename);
  return filecont;
}

void StgEngImpl::saveToDB(const PnlSPtr& pnl) {
  const auto identity = GET_RAND_STR();
  const auto sql = pnl->getSqlOfInsert();
  const auto [ret, execRet] = dbEng_->asyncExec(identity, sql);
  if (ret != 0) {
    LOG_W("Insert pnl to db failed. [{}]", sql);
  }
}

std::tuple<int, OrderInfoSPtr> StgEngImpl::getOrderInfo(OrderId orderId) const {
  return ordMgr_->getOrderInfo(orderId, DeepClone::True, LockFunc::True);
}

ScheduleTaskBundleSPtr StgEngImpl::getScheduleTaskBundle() {
  auto ret = std::make_shared<ScheduleTaskBundle>();
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxScheduleTaskBundle_);
    std::ext::erase_if(*scheduleTaskBundle_, [this](const auto& scheduleTask) {
      if (scheduleTask->execTimes_ == scheduleTask->maxExecTimes_) {
        const auto statusMsg = fmt::format(
            "[{}] scheduleTask {} finished. [execTimes = {}; maxExecTimes {}]",
            getAppName(), scheduleTask->scheduleTaskName_,
            scheduleTask->execTimes_, scheduleTask->maxExecTimes_);
        LOG_I(statusMsg);
        return true;
      } else {
        return false;
      }
    });
    ret->assign(std::begin(*scheduleTaskBundle_),
                std::end(*scheduleTaskBundle_));
  }
  return ret;
};

}  // namespace bq::stg
