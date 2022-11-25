/*!
 * \file RiskMgr.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "RiskMgr.hpp"

#include "AssetsMgr.hpp"
#include "ClientChannelGroup.hpp"
#include "Config.hpp"
#include "OrdMgr.hpp"
#include "PosMgr.hpp"
#include "PubSvc.hpp"
#include "RiskMgrConst.hpp"
#include "SHMHeader.hpp"
#include "SHMIPCTask.hpp"
#include "SHMSrv.hpp"
#include "StgEngTaskHandler.hpp"
#include "TDGWTaskHandler.hpp"
#include "db/DBEngConst.hpp"
#include "db/TBLMonitorOfSymbolInfo.hpp"
#include "def/BQDef.hpp"
#include "def/DataStruOfMD.hpp"
#include "util/BQUtil.hpp"
#include "util/Literal.hpp"
#include "util/Logger.hpp"
#include "util/MarketDataCache.hpp"
#include "util/ScheduleTaskBundle.hpp"
#include "util/Scheduler.hpp"
#include "util/StdExt.hpp"
#include "util/String.hpp"
#include "util/SubMgr.hpp"
#include "util/TaskDispatcher.hpp"

namespace bq::riskmgr {

int RiskMgr::prepareInit() {
  if (const auto ret = Config::get_mutable_instance().init(configFilename_);
      ret != 0) {
    const auto statusMsg = fmt::format("Prepare init failed.");
    std::cerr << statusMsg << std::endl;
    return ret;
  }

  if (const auto ret = InitLogger(configFilename_); ret != 0) {
    const auto statusMsg =
        fmt::format("Init td srv failed because of init logger failed. {}",
                    configFilename_);
    std::cerr << statusMsg << std::endl;
    return ret;
  }

  return 0;
}

int RiskMgr::doInit() {
  if (const auto ret = initDBEng(); ret != 0) {
    LOG_E("Do init failed. ");
    return ret;
  }

  initTBLMonitorOfSymbolInfo();

  marketDataCache_ = std::make_shared<MarketDataCache>();
  subMgr_ = std::make_shared<SubMgr>(
      AppName, [this](const auto shmBuf, auto shmBufLen) {
        const auto task = std::make_shared<SHMIPCTask>(shmBuf, shmBufLen);
        const auto header = static_cast<const SHMHeader*>(task->data_);
        if (header->msgId_ == MSG_ID_ON_MD_TRADES) {
          const auto trades = MakeMsgSPtrByTask<Trades>(task);
          marketDataCache_->cache(trades);
        }
      });

  initPosMgr();
  initAssetsMgr();
  initOrdMgr();

  pubSvc_ = std::make_shared<PubSvc>(this);

  tdGWGroup_ = std::make_shared<ClientChannelGroup>("TDGW");
  stgEngGroup_ = std::make_shared<ClientChannelGroup>("StgEng");

  tdGWTaskHandler_ = std::make_shared<TDGWTaskHandler>(this);
  stgEngTaskHandler_ = std::make_shared<StgEngTaskHandler>(this);

  if (const auto ret = initRiskMgrTaskDispatcher(); ret != 0) {
    LOG_E("Do init failed.");
    return ret;
  }

  initSHMSrv();

  scheduleTaskBundle_ = std::make_shared<ScheduleTaskBundle>();
  initScheduleTaskBundle();
  scheduleTaskBundleExecutor_ = std::make_shared<Scheduler>(
      std::string("RISK_MGR"),
      [this]() { ExecScheduleTaskBundle(getScheduleTaskBundle()); }, 1);

  return 0;
}

int RiskMgr::initDBEng() {
  const auto dbEngParam = SetParam(db::DEFAULT_DB_ENG_PARAM,
                                   CONFIG["dbEngParam"].as<std::string>());
  int retOfMakeDBEng = 0;
  std::tie(retOfMakeDBEng, dbEng_) = db::MakeDBEng(
      dbEngParam, [](db::DBTaskSPtr& dbTask, const StringSPtr& dbExecRet) {
        LOG_D("Exec sql finished. [{}] [exec result = {}]", dbTask->toStr(),
              *dbExecRet);
      });
  if (retOfMakeDBEng != 0) {
    LOG_E("Init dbeng failed. {}", dbEngParam);
    return retOfMakeDBEng;
  }

  if (auto retOfInit = getDBEng()->init(); retOfInit != 0) {
    LOG_E("Init dbeng failed. {}", dbEngParam);
    return retOfInit;
  }

  return 0;
}

void RiskMgr::initTBLMonitorOfSymbolInfo() {
  const auto milliSecIntervalOfTBLMonitorOfSymbolInfo =
      CONFIG["milliSecIntervalOfTBLMonitorOfSymbolInfo"].as<std::uint32_t>();
  const auto sql = fmt::format("SELECT * FROM {};", TBLSymbolInfo::TableName);
  tblMonitorOfSymbolInfo_ = std::make_shared<db::TBLMonitorOfSymbolInfo>(
      getDBEng(), milliSecIntervalOfTBLMonitorOfSymbolInfo, sql);
}

void RiskMgr::initPosMgr() {
  posMgr_ = std::make_shared<PosMgr>();
  const auto sql = fmt::format("SELECT * FROM `posInfo`");
  posMgr_->init(CONFIG, getDBEng(), sql);
}

void RiskMgr::initAssetsMgr() {
  assetsMgr_ = std::make_shared<AssetsMgr>();
  const auto sql = fmt::format("SELECT * FROM `assetInfo`");
  assetsMgr_->init(CONFIG, getDBEng(), sql);
}

void RiskMgr::initOrdMgr() {
  ordMgr_ = std::make_shared<OrdMgr>();
  const auto filled = magic_enum::enum_integer(OrderStatus::Filled);
  const auto sql = fmt::format(
      "SELECT * FROM `orderInfo` WHERE `orderStatus` < {}; ", filled);
  ordMgr_->init(CONFIG, getDBEng(), sql);
}

int RiskMgr::initRiskMgrTaskDispatcher() {
  const auto riskMgrTaskDispatcherParamInStrFmt =
      SetParam(DEFAULT_TASK_DISPATCHER_PARAM,
               CONFIG["riskMgrTaskDispatcherParam"].as<std::string>());
  const auto [ret, riskMgrTaskDispatcherParam] =
      MakeTaskDispatcherParam(riskMgrTaskDispatcherParamInStrFmt);
  if (ret != 0) {
    LOG_E("Init taskdispatcher failed. {}", riskMgrTaskDispatcherParamInStrFmt);
    return ret;
  }

  const auto makeAsyncTask = [](const auto& task) {
    const auto acctId = GetAcctIdFromTask(task);
    return std::make_tuple(0, std::make_shared<SHMIPCAsyncTask>(task, acctId));
  };

  const auto getThreadForAsyncTask = [](const auto& asyncTask,
                                        auto taskSpecificThreadPoolSize) {
    const auto acctId = std::any_cast<AcctId>(asyncTask->arg_);
    const auto threadNo = acctId % taskSpecificThreadPoolSize;
    return threadNo;
  };

  const auto handleAsyncTask = [this](auto& asyncTask) {
    const auto shmHeader =
        static_cast<const SHMHeader*>(asyncTask->task_->data_);
    switch (shmHeader->msgId_) {
      case MSG_ID_ON_ORDER:
        stgEngTaskHandler_->handleAsyncTask(asyncTask);
        break;
      case MSG_ID_ON_CANCEL_ORDER:
        stgEngTaskHandler_->handleAsyncTask(asyncTask);
        break;
      case MSG_ID_ON_STG_REG:
        stgEngTaskHandler_->handleAsyncTask(asyncTask);
        break;

      case MSG_ID_ON_ORDER_RET:
        tdGWTaskHandler_->handleAsyncTask(asyncTask);
        break;
      case MSG_ID_ON_CANCEL_ORDER_RET:
        tdGWTaskHandler_->handleAsyncTask(asyncTask);
        break;
      case MSG_ID_SYNC_ASSETS:
        tdGWTaskHandler_->handleAsyncTask(asyncTask);
        break;
      case MSG_ID_ON_TDGW_REG:
        tdGWTaskHandler_->handleAsyncTask(asyncTask);
        break;

      default:
        LOG_W("Unable to process msgId {}.", shmHeader->msgId_);
        break;
    }
  };

  riskMgrTaskDispatcher_ = std::make_shared<TaskDispatcher<SHMIPCTaskSPtr>>(
      riskMgrTaskDispatcherParam, makeAsyncTask, getThreadForAsyncTask,
      handleAsyncTask);
  riskMgrTaskDispatcher_->init();

  return ret;
}

void RiskMgr::initSHMSrv() {
  const auto tdGWChannel =
      fmt::format("{}@{}", AppName, CONFIG["tdGWChannel"].as<std::string>());
  shmSrvOfTDGW_ = std::make_shared<SHMSrv>(
      tdGWChannel, [this](const auto* shmBuf, std::size_t shmBufLen) {
        auto task = std::make_shared<SHMIPCTask>(shmBuf, shmBufLen);
        riskMgrTaskDispatcher_->dispatch(task);
      });

  const auto stgEngChannel =
      fmt::format("{}@{}", AppName, CONFIG["stgEngChannel"].as<std::string>());
  shmSrvOfStgEng_ = std::make_shared<SHMSrv>(
      stgEngChannel, [this](const auto* shmBuf, std::size_t shmBufLen) {
        auto task = std::make_shared<SHMIPCTask>(shmBuf, shmBufLen);
        riskMgrTaskDispatcher_->dispatch(task);
      });

  const auto pubChannel =
      fmt::format("{}@{}", AppName, CONFIG["pubChannel"].as<std::string>());
  shmSrvOfPub_ = std::make_shared<SHMSrv>(
      pubChannel, [this](const auto* shmBuf, std::size_t shmBufLen) {});
}

void RiskMgr::initScheduleTaskBundle() {
  getScheduleTaskBundle()->emplace_back(std::make_shared<ScheduleTask>(
      "removeExpiredChannel",
      [this]() {
        getTDGWGroup()->removeExpiredChannel();
        getStgEngGroup()->removeExpiredChannel();
        return true;
      },
      ExecAtStartup::False, MilliSecInterval(5000)));

  const auto milliSecIntervalOfSubMarketData =
      CONFIG["milliSecIntervalOfSubMarketData"].as<std::uint32_t>();
  getScheduleTaskBundle()->emplace_back(std::make_shared<ScheduleTask>(
      "subMarketData",
      [this]() {
        const auto posInfoGroup = getPosMgr()->getPosInfoGroup(LockFunc::True);
        for (const auto& posInfo : posInfoGroup) {
          const auto topicPrefix = posInfo->getTopicPrefix();
          const auto topic = fmt::format("{}{}", topicPrefix,
                                         magic_enum::enum_name(MDType::Trades));
          getSubMgr()->sub(PUB_CHANNEL, topic);
        }
        return true;
      },
      ExecAtStartup::True, milliSecIntervalOfSubMarketData));

  const auto milliSecIntervalOfPubPosUpdateOfAcctId =
      CONFIG["milliSecIntervalOfPubPosUpdateOfAcctId"].as<std::uint32_t>();
  getScheduleTaskBundle()->emplace_back(std::make_shared<ScheduleTask>(
      "pubPosUpdateOfAcctId",
      [this]() {
        getPubSvc()->pubPosUpdateOfAcctId();
        return true;
      },
      ExecAtStartup::True, milliSecIntervalOfPubPosUpdateOfAcctId));

  const auto milliSecIntervalOfPubPosUpdateOfStgId =
      CONFIG["milliSecIntervalOfPubPosUpdateOfStgId"].as<std::uint32_t>();
  getScheduleTaskBundle()->emplace_back(std::make_shared<ScheduleTask>(
      "pubPosUpdateOfStgId",
      [this]() {
        getPubSvc()->pubPosUpdateOfStgId();
        return true;
      },
      ExecAtStartup::True, milliSecIntervalOfPubPosUpdateOfStgId));

  const auto milliSecIntervalOfPubPosUpdateOfStgInstId =
      CONFIG["milliSecIntervalOfPubPosUpdateOfStgInstId"].as<std::uint32_t>();
  getScheduleTaskBundle()->emplace_back(std::make_shared<ScheduleTask>(
      "pubPosUpdateOfStgInstId",
      [this]() {
        getPubSvc()->pubPosUpdateOfStgInstId();
        return true;
      },
      ExecAtStartup::True, milliSecIntervalOfPubPosUpdateOfStgInstId));

  const auto milliSecIntervalOfPubPosSnapshotOfAcctId =
      CONFIG["milliSecIntervalOfPubPosSnapshotOfAcctId"].as<std::uint32_t>();
  getScheduleTaskBundle()->emplace_back(std::make_shared<ScheduleTask>(
      "pubPosSnapshotOfAcctId",
      [this]() {
        getPubSvc()->pubPosSnapshotOfAcctId();
        return true;
      },
      ExecAtStartup::True, milliSecIntervalOfPubPosSnapshotOfAcctId));

  const auto milliSecIntervalOfPubPosSnapshotOfStgId =
      CONFIG["milliSecIntervalOfPubPosSnapshotOfStgId"].as<std::uint32_t>();
  getScheduleTaskBundle()->emplace_back(std::make_shared<ScheduleTask>(
      "pubPosSnapshotOfStgId",
      [this]() {
        getPubSvc()->pubPosSnapshotOfStgId();
        return true;
      },
      ExecAtStartup::True, milliSecIntervalOfPubPosSnapshotOfStgId));

  const auto milliSecIntervalOfPubPosSnapshotOfStgInstId =
      CONFIG["milliSecIntervalOfPubPosSnapshotOfStgInstId"].as<std::uint32_t>();
  getScheduleTaskBundle()->emplace_back(std::make_shared<ScheduleTask>(
      "pubPosSnapshotOfStgInstId",
      [this]() {
        getPubSvc()->pubPosSnapshotOfStgInstId();
        return true;
      },
      ExecAtStartup::True, milliSecIntervalOfPubPosSnapshotOfStgInstId));

  const auto milliSecIntervalOfPubAssetsUpdate =
      CONFIG["milliSecIntervalOfPubAssetsUpdate"].as<std::uint32_t>();
  getScheduleTaskBundle()->emplace_back(std::make_shared<ScheduleTask>(
      "pubAssetsInfo",
      [this]() {
        getPubSvc()->pubAssetsUpdate();
        return true;
      },
      ExecAtStartup::True, milliSecIntervalOfPubAssetsUpdate));

  const auto milliSecIntervalOfPubAssetsSnapshot =
      CONFIG["milliSecIntervalOfPubAssetsSnapshot"].as<std::uint32_t>();
  getScheduleTaskBundle()->emplace_back(std::make_shared<ScheduleTask>(
      "pubAssetsInfo",
      [this]() {
        getPubSvc()->pubAssetsSnapshot();
        return true;
      },
      ExecAtStartup::True, milliSecIntervalOfPubAssetsSnapshot));
}

int RiskMgr::doRun() {
  getDBEng()->start();
  if (const auto ret = tblMonitorOfSymbolInfo_->start(); ret != 0) {
    LOG_E("Run failed.");
    return ret;
  }
  riskMgrTaskDispatcher_->start();
  shmSrvOfTDGW_->start();
  shmSrvOfStgEng_->start();
  shmSrvOfPub_->start();

  if (const auto ret = scheduleTaskBundleExecutor_->start(); ret != 0) {
    LOG_E("Start scheduler of multi task failed.");
    return ret;
  }

  return 0;
}

void RiskMgr::doExit(const boost::system::error_code* ec, int signalNum) {
  scheduleTaskBundleExecutor_->stop();
  shmSrvOfPub_->stop();
  shmSrvOfStgEng_->stop();
  shmSrvOfTDGW_->stop();
  riskMgrTaskDispatcher_->stop();
  tblMonitorOfSymbolInfo_->stop();
  getDBEng()->stop();
}

}  // namespace bq::riskmgr
