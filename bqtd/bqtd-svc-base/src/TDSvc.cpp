/*!
 * \file TDSvc.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "TDSvc.hpp"

#include "AcctInfo.hpp"
#include "AssetsMgr.hpp"
#include "Config.hpp"
#include "ExceedFlowCtrlHandler.hpp"
#include "HttpCliOfExch.hpp"
#include "OrdMgr.hpp"
#include "SHMIPC.hpp"
#include "SimedOrderInfoHandler.hpp"
#include "TDSrvTaskHandler.hpp"
#include "TDSvcUtil.hpp"
#include "WSCli.hpp"
#include "WSCliOfExch.hpp"
#include "db/DBE.hpp"
#include "db/TBLAcctInfo.hpp"
#include "db/TBLMonitorOfSymbolInfo.hpp"
#include "db/TBLOrderInfo.hpp"
#include "db/TBLRecSetMaker.hpp"
#include "def/BQDef.hpp"
#include "def/DataStruOfOthers.hpp"
#include "def/Def.hpp"
#include "def/TaskOfSync.hpp"
#include "util/ExternalStatusCodeCache.hpp"
#include "util/FlowCtrlSvc.hpp"
#include "util/Literal.hpp"
#include "util/ScheduleTaskBundle.hpp"
#include "util/Scheduler.hpp"
#include "util/SignalHandler.hpp"
#include "util/String.hpp"
#include "util/TaskDispatcher.hpp"
#include "util/TrdSymbolCache.hpp"

namespace bq::td::svc {

int TDSvc::prepareInit() {
  taskOfSyncGroup_.reserve(1024);

  auto retOfConfInit = Config::get_mutable_instance().init(configFilename_);
  if (retOfConfInit != 0) {
    const auto statusMsg = fmt::format("Prepare init failed.");
    std::cerr << statusMsg << std::endl;
    return retOfConfInit;
  }

  const auto retOfLoggerInit = InitLogger(CONFIG);
  if (retOfLoggerInit != 0) {
    const auto statusMsg = fmt::format("Prepare init failed.");
    std::cerr << statusMsg << std::endl;
    return retOfLoggerInit;
  }

  return 0;
}

int TDSvc::doInit() {
  marketCode_ = CONFIG["marketCode"].as<std::string>();
  symbolType_ = CONFIG["symbolType"].as<std::string>();
  acctId_ = CONFIG["acctId"].as<AcctId>();
  simedMode_ = CONFIG["simedMode"]["enable"].as<bool>(false);

  marketCodeEnum_ = GetMarketCode(marketCode_);
  if (marketCodeEnum_ == MarketCode::Others) {
    LOG_W("Do init failed because of invalid marketcode {}.", marketCode_);
    return -1;
  }

  const auto symbolType = magic_enum::enum_cast<SymbolType>(symbolType_);
  if (!symbolType.has_value()) {
    LOG_W("Do init failed because of invalid symboltype {}.", symbolType_);
    return -1;
  }
  symbolTypeEnum_ = symbolType.value();

  appName_ = fmt::format("{}-{}-{}-{}", TOPIC_PREFIX_OF_TRADE_DATA,
                         getMarketCode(), getSymbolType(), getAcctId());

  if (const auto ret = initDBEng(); ret != 0) {
    LOG_E("Do init failed.");
    return ret;
  }

  initTBLMonitorOfSymbolInfo();
  initTrdSymbolCache();
  initExternalStatusCodeCache();

  if (const auto ret = queryMaxNoUsedToCalcPos(); ret != 0) {
    LOG_E("Do init failed.");
    return ret;
  }

  if (const auto ret = queryAcctInfoFromDB(); ret != 0) {
    LOG_E("Do init failed.");
    return ret;
  }
  makeAcctData(acctInfo_->acctData_);

  initAssetsMgr();
  initOrdMgr();

  if (isSimedMode()) {
    simedOrderInfoHandler_ = std::make_shared<SimedOrderInfoHandler>(this);
  } else {
    assert(wsCliOfExch_ != nullptr && "wsCliOfExch_ != nullptr");
    if (const auto ret = wsCliOfExch_->init(); ret != 0) {
      LOG_E("Do init failed.");
      return ret;
    }
  }

  tdSrvTaskHandler_ = std::make_shared<TDSrvTaskHandler>(this);
  initTDSrvTaskDispatcher();
  initSHMCliOfTDSrv();
  initSHMCliOfRiskMgr();

  flowCtrlSvc_ = std::make_shared<FlowCtrlSvc>(CONFIG);
  exceedFlowCtrlHandler_ = std::make_shared<ExceedFlowCtrlHandler>(this);

  scheduleTaskBundle_ = std::make_shared<ScheduleTaskBundle>();
  initScheduleTaskBundle();
  scheduleTaskBundleExecutor_ = std::make_shared<Scheduler>(
      appName_, [this]() { ExecScheduleTaskBundle(getScheduleTaskBundle()); },
      1 * 1000);

  return 0;
}

int TDSvc::initDBEng() {
  const auto dbEngParam = SetParam(db::DEFAULT_DB_ENG_PARAM,
                                   CONFIG["dbEngParam"].as<std::string>());
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

void TDSvc::initTBLMonitorOfSymbolInfo() {
  const auto milliSecIntervalOfTBLMonitorOfSymbolInfo =
      CONFIG["milliSecIntervalOfTBLMonitorOfSymbolInfo"].as<std::uint32_t>();

  const auto sql = fmt::format(
      "SELECT * FROM {} WHERE `marketCode` = '{}' AND `symbolType` = '{}'",
      TBLSymbolInfo::TableName, getMarketCode(), getSymbolType());

  tblMonitorOfSymbolInfo_ = std::make_shared<db::TBLMonitorOfSymbolInfo>(
      getDBEng(), milliSecIntervalOfTBLMonitorOfSymbolInfo, sql);
}

void TDSvc::initTrdSymbolCache() {
  trdSymbolCache_ = std::make_shared<TrdSymbolCache>(getDBEng());
  trdSymbolCache_->load(getMarketCode(), getSymbolType(), getAcctId());
}

void TDSvc::initExternalStatusCodeCache() {
  externalStatusCodeCache_ =
      std::make_shared<ExternalStatusCodeCache>(getDBEng());
  externalStatusCodeCache_->load(getMarketCode(), getSymbolType());
}

int TDSvc::queryMaxNoUsedToCalcPos() {
  const auto sql = fmt::format(
      "SELECT * FROM {} WHERE acctId = {} "
      "ORDER BY noUsedToCalcPos DESC LIMIT 1;",
      TBLOrderInfo::TableName, getAcctId());
  const auto [ret, tblRecSet] =
      db::TBLRecSetMaker<TBLOrderInfo>::ExecSql(getDBEng(), sql);
  if (ret != 0) {
    LOG_W("Query maxNoUsedToCalcPos from db failed. {}", sql);
    return ret;
  }

  if (!tblRecSet->empty()) {
    const auto& tblRec = std::begin(*tblRecSet);
    const auto& orderInfo = tblRec->second;
    maxNoUsedToCalcPos_ = orderInfo->getRecWithAllFields()->noUsedToCalcPos;
  }
  LOG_I("Query maxNoUsedToCalcPos success. [maxNoUsedToCalcPos = {}]",
        maxNoUsedToCalcPos_);

  return 0;
}

int TDSvc::queryAcctInfoFromDB() {
  const auto identity = GET_RAND_STR();
  const auto sql = fmt::format(
      "SELECT * FROM {} WHERE `marketCode` = '{}' AND `symbolType` = '{}' "
      "AND `acctId` = '{}' AND isDel = 0",
      TBLAcctInfo::TableName, getMarketCode(), getSymbolType(), getAcctId());

  auto [retOfExec, execRet] = getDBEng()->syncExec(identity, sql);
  if (retOfExec != 0) {
    LOG_W("Init acct data failed.");
    return retOfExec;
  }

  acctInfo_ = std::make_shared<AcctInfo>();
  const auto retOfInit = acctInfo_->init(getAcctId(), execRet);
  if (retOfInit != 0) {
    LOG_W("Init acct data failed.");
    return retOfInit;
  }

  return 0;
}

void TDSvc::makeAcctData(const std::string& data) {
  acctData_ = GetApiInfo(data);
}

void TDSvc::initAssetsMgr() {
  assetsMgr_ = std::make_shared<AssetsMgr>();
  const auto sql = fmt::format(
      "SELECT * FROM `assetInfo` "
      "WHERE `acctId` = {} AND `marketCode` = '{}' AND `symbolType` = '{}'",
      getAcctId(), getMarketCode(), getSymbolType());
  getAssetsMgr()->init(CONFIG, getDBEng(), sql);
}

void TDSvc::initOrdMgr() {
  const auto filled = magic_enum::enum_integer(OrderStatus::Filled);
  ordMgr_ = std::make_shared<OrdMgr>();
  const auto sql = fmt::format(
      "SELECT * FROM `orderInfo` WHERE `orderStatus` < {} AND `marketCode` = "
      "'{}' AND `symbolType` = '{}' AND `acctId` = {}; ",
      filled, getMarketCode(), getSymbolType(), getAcctId());
  getOrdMgr()->init(CONFIG, getDBEng(), sql);
}

int TDSvc::initTDSrvTaskDispatcher() {
  const auto tdSrvTaskDispatcherParamInStrFmt =
      SetParam(DEFAULT_TASK_DISPATCHER_PARAM,
               CONFIG["tdSrvTaskDispatcherParam"].as<std::string>());
  const auto [ret, tdSrvTaskDispatcherParam] =
      MakeTaskDispatcherParam(tdSrvTaskDispatcherParamInStrFmt);
  if (ret != 0) {
    LOG_E("[{}] Init taskdispatcher failed. {}", appName_,
          tdSrvTaskDispatcherParamInStrFmt);
    return ret;
  }

  const auto getThreadForAsyncTask = [](const auto& asyncTask,
                                        auto taskSpecificThreadPoolSize) {
    const auto shmHeader =
        static_cast<const SHMHeader*>(asyncTask->task_->data_);
    switch (shmHeader->msgId_) {
      case MSG_ID_ON_ORDER:
      case MSG_ID_ON_CANCEL_ORDER:
        return ThreadNo(0);
      default:
        return ThreadNo(1);
    }
  };

  const auto handleAsyncTask = [this](auto& asyncTask) {
    tdSrvTaskHandler_->handleAsyncTask(asyncTask);
  };

  tdSrvTaskDispatcher_ = std::make_shared<TaskDispatcher<SHMIPCTaskSPtr>>(
      tdSrvTaskDispatcherParam, nullptr, getThreadForAsyncTask,
      handleAsyncTask);
  tdSrvTaskDispatcher_->init();

  return ret;
}

void TDSvc::initSHMCliOfTDSrv() {
  const auto tdSrvChannel = CONFIG["tdSrvChannel"].as<std::string>();
  const auto addr =
      fmt::format("{}{}{}", appName_, SEP_OF_SHM_SVC, tdSrvChannel);

  const auto onSHMDataRecv = [this](const void* shmBuf, std::size_t shmBufLen) {
    const auto task = std::make_shared<SHMIPCTask>(shmBuf, shmBufLen);
    auto asyncTask = std::make_shared<SHMIPCAsyncTask>(task);
    tdSrvTaskDispatcher_->dispatch(asyncTask);
  };

  shmCliOfTDSrv_ = std::make_shared<SHMCli>(addr, onSHMDataRecv);
  shmCliOfTDSrv_->setClientChannel(acctId_);
}

void TDSvc::initSHMCliOfRiskMgr() {
  const auto riskMgrChannel = CONFIG["riskMgrChannel"].as<std::string>();
  const auto addr =
      fmt::format("{}{}{}", appName_, SEP_OF_SHM_SVC, riskMgrChannel);

  const auto onSHMDataRecv = [this](const void* shmBuf, std::size_t shmBufLen) {
  };

  shmCliOfRiskMgr_ = std::make_shared<SHMCli>(addr, onSHMDataRecv);
  shmCliOfRiskMgr_->setClientChannel(acctId_);
}

void TDSvc::beforeInitScheduleTaskBundle() {
  if (isSimedMode() == false) {
    const auto secIntervalOfExtendConnLifecycle =
        CONFIG["secIntervalOfExtendConnLifecycle"].as<std::uint32_t>();
    getScheduleTaskBundle()->emplace_back(std::make_shared<ScheduleTask>(
        "extendConnLifecycle",
        [this]() {
          auto asyncTask = MakeTDSrvSignal(MSG_ID_EXTEND_CONN_LIFECYCLE);
          getTDSrvTaskDispatcher()->dispatch(asyncTask);
          return true;
        },
        ExecAtStartup::False, secIntervalOfExtendConnLifecycle * 1000));

    const auto secIntervalOfSyncAssetsSnapshot =
        CONFIG["secIntervalOfSyncAssetsSnapshot"].as<std::uint32_t>();
    getScheduleTaskBundle()->emplace_back(std::make_shared<ScheduleTask>(
        "syncAssetsSnapshot",
        [this]() {
          auto asyncTask = MakeTDSrvSignal(MSG_ID_SYNC_ASSETS_SNAPSHOT);
          getTDSrvTaskDispatcher()->dispatch(asyncTask);
          return true;
        },
        ExecAtStartup::True, secIntervalOfSyncAssetsSnapshot * 1000));

    const auto secAgoTheOrderNeedToBeSynced =
        CONFIG["secAgoTheOrderNeedToBeSynced"].as<std::uint32_t>();
    const auto secIntervalOfSyncUnclosedOrderInfo =
        CONFIG["secIntervalOfSyncUnclosedOrderInfo"].as<std::uint32_t>();
    getScheduleTaskBundle()->emplace_back(std::make_shared<ScheduleTask>(
        "syncUnclosedOrderInfo",
        [this, secAgoTheOrderNeedToBeSynced]() {
          const auto& orderInfoGroup =
              getOrdMgr()->getOrderInfoGroup(secAgoTheOrderNeedToBeSynced);
          for (const auto& orderInfo : orderInfoGroup) {
            auto asyncTask =
                MakeTDSrvSignal(MSG_ID_SYNC_UNCLOSED_ORDER_INFO, orderInfo);
            getTDSrvTaskDispatcher()->dispatch(asyncTask);
          }
          return true;
        },
        ExecAtStartup::True, secIntervalOfSyncUnclosedOrderInfo * 1000));
  }

  getScheduleTaskBundle()->emplace_back(std::make_shared<ScheduleTask>(
      "exceedFlowCtrlHandler",
      [this]() {
        exceedFlowCtrlHandler_->handleExceedFlowCtrlTask();
        return true;
      },
      ExecAtStartup::False, MilliSecInterval(1000), UINT64_MAX,
      WriteLog::False));

  const auto secIntervalOfReloadExternalStatusCode =
      CONFIG["secIntervalOfReloadExternalStatusCode"].as<std::uint32_t>();
  getScheduleTaskBundle()->emplace_back(std::make_shared<ScheduleTask>(
      "reloadExternalStatusCode",
      [this]() {
        externalStatusCodeCache_->reload(getMarketCode(), getSymbolType());
        return true;
      },
      ExecAtStartup::False, secIntervalOfReloadExternalStatusCode * 1000));

  getScheduleTaskBundle()->emplace_back(std::make_shared<ScheduleTask>(
      "sendTDGWReg",
      [this]() {
        sendTDGWReg();
        return true;
      },
      ExecAtStartup::False, MilliSecInterval(5000)));

  const auto milliSecIntervalOfSyncTask =
      CONFIG["milliSecIntervalOfSyncTask"].as<std::uint32_t>();
  getScheduleTaskBundle()->emplace_back(std::make_shared<ScheduleTask>(
      "syncTask",
      [this]() {
        handleTaskOfSyncGroup();
        return true;
      },
      ExecAtStartup::False, milliSecIntervalOfSyncTask));
}

int TDSvc::doRun() {
  getDBEng()->start();

  if (const auto ret = tblMonitorOfSymbolInfo_->start(); ret != 0) {
    LOG_E("Run failed.");
    return ret;
  }

  if (isSimedMode() == false) {
    if (auto ret = wsCliOfExch_->start(); ret != 0) {
      LOG_E("Run failed.");
      return ret;
    }

    if (const auto [ret, addrOfWS] = getAddrOfWS(); ret != 0) {
      LOG_E("Run failed.");
      return ret;
    } else {
      if (const auto [ret, no] = wsCliOfExch_->getWSCli()->connect(addrOfWS);
          ret != 0) {
        LOG_E("Run failed.");
        return ret;
      }
    }
  }

  tdSrvTaskDispatcher_->start();
  shmCliOfTDSrv_->start();
  shmCliOfRiskMgr_->start();
  sendTDGWReg();

  if (const auto ret = scheduleTaskBundleExecutor_->start(); ret != 0) {
    LOG_E("Start scheduler of multi task failed.");
    return ret;
  }

  return 0;
}

void TDSvc::sendTDGWReg() {
  shmCliOfTDSrv_->asyncSendReqWithZeroCopy(
      [&](void* shmBufOfReq) {
        auto tdGWReg = static_cast<TDGWReg*>(shmBufOfReq);
      },
      MSG_ID_ON_TDGW_REG, sizeof(TDGWReg));

  shmCliOfRiskMgr_->asyncSendReqWithZeroCopy(
      [&](void* shmBufOfReq) {
        auto tdGWReg = static_cast<TDGWReg*>(shmBufOfReq);
      },
      MSG_ID_ON_TDGW_REG, sizeof(TDGWReg));
}

void TDSvc::cacheTaskOfSyncGroup(MsgId msgId, const std::any& task,
                                 SyncToRiskMgr syncToRiskMgr,
                                 SyncToDB syncToDB) {
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxTaskOfSyncGroup_);
    taskOfSyncGroup_.emplace_back(
        std::make_shared<TaskOfSync>(msgId, task, syncToRiskMgr, syncToDB));
  }
}

void TDSvc::handleTaskOfSyncGroup() {
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
          [&](void* shmBuf) {
            InitMsgBody(shmBuf, *orderInfo);
            LOG_I("Send order info to risk mgr. {}",
                  static_cast<OrderInfo*>(shmBuf)->toShortStr());
          },
          rec->msgId_, sizeof(OrderInfo));

    } else if (rec->msgId_ == MSG_ID_SYNC_ASSETS) {
      const auto updateInfoOfAssetGroup =
          std::any_cast<UpdateInfoOfAssetGroupSPtr>(rec->task_);
      NotifyAssetInfo(shmCliOfRiskMgr_, getAcctId(), updateInfoOfAssetGroup);

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

    } else if (rec->msgId_ == MSG_ID_SYNC_ASSETS) {
      const auto updateInfoOfAssetGroup =
          std::any_cast<UpdateInfoOfAssetGroupSPtr>(rec->task_);

      for (const auto& assetInfo :
           *updateInfoOfAssetGroup->assetInfoGroupAdd_) {
        const auto identity = GET_RAND_STR();
        const auto sql = assetInfo->getSqlOfInsert();
        const auto [ret, execRet] = dbEng_->asyncExec(identity, sql);
        if (ret != 0) {
          LOG_W("Insert asset info to db failed. [{}]", sql);
        }
      }

      for (const auto& assetInfo :
           *updateInfoOfAssetGroup->assetInfoGroupDel_) {
        const auto identity = GET_RAND_STR();
        const auto sql = assetInfo->getSqlOfDelete();
        const auto [ret, execRet] = dbEng_->asyncExec(identity, sql);
        if (ret != 0) {
          LOG_W("Del asset info from db failed. [{}]", sql);
        }
      }

      for (const auto& assetInfo :
           *updateInfoOfAssetGroup->assetInfoGroupChg_) {
        const auto identity = GET_RAND_STR();
        const auto sql = assetInfo->getSqlOfUpdate();
        const auto [ret, execRet] = dbEng_->asyncExec(identity, sql);
        if (ret != 0) {
          LOG_W("Update asset info from db failed. [{}]", sql);
        }
      }

    } else {
      LOG_W("Unhandled task of sync to db. {} - {}", rec->msgId_,
            GetMsgName(rec->msgId_))
    }
  }
}

void TDSvc::doExit(const boost::system::error_code* ec, int signalNum) {
  scheduleTaskBundleExecutor_->stop();
  getAssetsMgr()->syncUpdateInfoOfAssetGroupToDB();
  shmCliOfRiskMgr_->stop();
  shmCliOfTDSrv_->stop();
  tdSrvTaskDispatcher_->stop();
  if (isSimedMode() == false) {
    wsCliOfExch_->stop();
  }
  tblMonitorOfSymbolInfo_->stop();
  getDBEng()->stop();
}

std::tuple<int, std::string> TDSvc::getAddrOfWS() {
  const auto addrOfWS = CONFIG["addrOfWS"].as<std::string>();
  return {0, addrOfWS};
}

}  // namespace bq::td::svc
