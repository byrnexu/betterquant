/*!
 * \file StgEngImpl.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "SHMIPCMsgId.hpp"
#include "def/BQConstIF.hpp"
#include "def/BQDefIF.hpp"
#include "util/Pch.hpp"
#include "util/StdExt.hpp"
#include "util/SvcBase.hpp"

namespace YAML {
class Node;
using NodeSPtr = std::shared_ptr<Node>;
}  // namespace YAML

namespace bq {
enum class ExecAtStartup;

struct SHMIPCTask;
using SHMIPCTaskSPtr = std::shared_ptr<SHMIPCTask>;
class SHMCli;
using SHMCliSPtr = std::shared_ptr<SHMCli>;

struct StgInstInfo;
using StgInstInfoSPtr = std::shared_ptr<StgInstInfo>;

struct OrderInfo;
using OrderInfoSPtr = std::shared_ptr<OrderInfo>;

class AcctInfoCache;
using AcctInfoCacheSPtr = std::shared_ptr<AcctInfoCache>;

class MarketDataCache;
using MarketDataCacheSPtr = std::shared_ptr<MarketDataCache>;

template <typename Task>
class TaskDispatcher;
template <typename Task>
using TaskDispatcherSPtr = std::shared_ptr<TaskDispatcher<Task>>;
template <typename Task>
struct AsyncTask;
template <typename Task>
using AsyncTaskSPtr = std::shared_ptr<AsyncTask<Task>>;

class SubMgr;
using SubMgrSPtr = std::shared_ptr<SubMgr>;

class OrdMgr;
using OrdMgrSPtr = std::shared_ptr<OrdMgr>;

class PosMgr;
using PosMgrSPtr = std::shared_ptr<PosMgr>;

struct TaskOfSync;
using TaskOfSyncSPtr = std::shared_ptr<TaskOfSync>;

class Scheduler;
using SchedulerSPtr = std::shared_ptr<Scheduler>;

struct ScheduleTask;
using ScheduleTaskSPtr = std::shared_ptr<ScheduleTask>;
using ScheduleTaskBundle = std::vector<ScheduleTaskSPtr>;
using ScheduleTaskBundleSPtr = std::shared_ptr<ScheduleTaskBundle>;

struct Pnl;
using PnlSPtr = std::shared_ptr<Pnl>;

enum class SyncToRiskMgr;
enum class SyncToDB;
}  // namespace bq

namespace bq::db {
class DBEng;
using DBEngSPtr = std::shared_ptr<DBEng>;

class TBLMonitorOfSymbolInfo;
using TBLMonitorOfSymbolInfoSPtr = std::shared_ptr<TBLMonitorOfSymbolInfo>;
class TBLMonitorOfStgInstInfo;
using TBLMonitorOfStgInstInfoSPtr = std::shared_ptr<TBLMonitorOfStgInstInfo>;
}  // namespace bq::db

namespace bq::stg {

class StgInstTaskHandlerImpl;
using StgInstTaskHandlerImplSPtr = std::shared_ptr<StgInstTaskHandlerImpl>;

class StgEngImpl;
using StgEngImplSPtr = std::shared_ptr<StgEngImpl>;

class StgEngImpl : public SvcBase {
 public:
  StgEngImpl(const StgEngImpl&) = delete;
  StgEngImpl& operator=(const StgEngImpl&) = delete;
  StgEngImpl(const StgEngImpl&&) = delete;
  StgEngImpl& operator=(const StgEngImpl&&) = delete;

  explicit StgEngImpl(const std::string& configFilename);

 private:
  int prepareInit() final;
  int doInit() final;

 private:
  int initDBEng();
  void initTBLMonitorOfSymbolInfo();
  void initTBLMonitorOfStgInstInfo();
  void initSubMgr();
  void initSHMCliOfTDSrv();
  void initSHMCliOfRiskMgr();
  void initOrdMgr();
  void initPosMgr();
  int initStgInstTaskDispatcher();
  void initScheduleTaskBundle();

 public:
  StgInstTaskHandlerImplSPtr getStgInstTaskHandler() {
    return stgInstTaskHandler_;
  }

  void installStgInstTaskHandler(const StgInstTaskHandlerImplSPtr& value) {
    stgInstTaskHandler_ = value;
  }

 private:
  int doRun();

 private:
  void sendStgStartSignal();
  void sendStgInstStartSignal();
  void sendStgReg();

 private:
  void doExit(const boost::system::error_code* ec, int signalNum) final;

 public:
  std::tuple<int, OrderId> order(const StgInstInfoSPtr& stgInstInfo,
                                 AcctId acctId, const std::string& symbolCode,
                                 Side side, PosSide posSide, Decimal orderPrice,
                                 Decimal orderSize);

  std::tuple<int, OrderId> order(OrderInfoSPtr& orderInfo);

  int cancelOrder(OrderId orderId);

 public:
  int sub(StgInstId subscriber, const std::string& topic);
  int unSub(StgInstId subscriber, const std::string& topic);

 public:
  std::tuple<int, std::string> queryHisMDBetween2Ts(
      const std::string& topic, std::uint64_t tsBegin, std::uint64_t tsEnd,
      std::uint32_t level = DEFAULT_DEPTH_LEVEL);

  std::tuple<int, std::string> queryHisMDBetween2Ts(
      MarketCode marketCode, SymbolType symbolType,
      const std::string& symbolCode, MDType mdType, std::uint64_t tsBegin,
      std::uint64_t tsEnd, std::uint32_t level = DEFAULT_DEPTH_LEVEL);

  std::tuple<int, std::string> querySpecificNumOfHisMDBeforeTs(
      const std::string& topic, std::uint64_t ts, int num,
      std::uint32_t level = DEFAULT_DEPTH_LEVEL);

  std::tuple<int, std::string> querySpecificNumOfHisMDBeforeTs(
      MarketCode marketCode, SymbolType symbolType,
      const std::string& symbolCode, MDType mdType, std::uint64_t ts, int num,
      std::uint32_t level = DEFAULT_DEPTH_LEVEL);

  std::tuple<int, std::string> querySpecificNumOfHisMDAfterTs(
      const std::string& topic, std::uint64_t ts, int num,
      std::uint32_t level = DEFAULT_DEPTH_LEVEL);

  std::tuple<int, std::string> querySpecificNumOfHisMDAfterTs(
      MarketCode marketCode, SymbolType symbolType,
      const std::string& symbolCode, MDType mdType, std::uint64_t ts, int num,
      std::uint32_t level = DEFAULT_DEPTH_LEVEL);

 public:
  void installStgInstTimer(StgInstId stgInstId, const std::string& timerName,
                           ExecAtStartup execAtStartUp,
                           std::uint32_t milliSecInterval,
                           std::uint64_t maxExecTimes = UINT64_MAX);

 public:
  bool saveStgPrivateData(StgInstId stgInstId, const std::string& jsonStr);
  std::string loadStgPrivateData(StgInstId stgInstId);

 public:
  void saveToDB(const PnlSPtr& pnl);

 public:
  const YAML::Node& getConfig() { return config_; }

  StgId getStgId() const { return stgId_; }
  std::string getAppName() const { return appName_; }

  db::DBEngSPtr getDBEng() const { return dbEng_; }

  db::TBLMonitorOfSymbolInfoSPtr getTBLMonitorOfSymbolInfo() const {
    return tblMonitorOfSymbolInfo_;
  }

  db::TBLMonitorOfStgInstInfoSPtr getTBLMonitorOfStgInstInfo() const {
    return tblMonitorOfStgInstInfo_;
  }

  MarketDataCacheSPtr getMarketDataCache() const { return marketDataCache_; }

  std::tuple<int, OrderInfoSPtr> getOrderInfo(OrderId orderId) const;
  OrdMgrSPtr getOrdMgr() const { return ordMgr_; }
  PosMgrSPtr getPosMgr() const { return posMgr_; }

  void resetBarrierOfStgStartSignal() {
    barrierOfStgStartSignal_ = std::make_shared<std::promise<void>>();
  }

  std::shared_ptr<std::promise<void>>& getBarrierOfStgStartSignal() {
    return barrierOfStgStartSignal_;
  }

  void cacheTaskOfSyncGroup(MsgId msgId, const std::any& task,
                            SyncToRiskMgr syncToRiskMgr, SyncToDB syncToDB);
  void handleTaskOfSyncGroup();

  ScheduleTaskBundleSPtr getScheduleTaskBundle();

 private:
  YAML::Node config_;

  StgId stgId_{1};
  std::string appName_;

  std::string rootDirOfStgPrivateData_;

  db::DBEngSPtr dbEng_{nullptr};

  db::TBLMonitorOfSymbolInfoSPtr tblMonitorOfSymbolInfo_{nullptr};
  db::TBLMonitorOfStgInstInfoSPtr tblMonitorOfStgInstInfo_{nullptr};

  AcctInfoCacheSPtr acctInfoCache_{nullptr};
  MarketDataCacheSPtr marketDataCache_{nullptr};

  OrdMgrSPtr ordMgr_{nullptr};
  PosMgrSPtr posMgr_{nullptr};
  SubMgrSPtr subMgr_{nullptr};

  SHMCliSPtr shmCliOfTDSrv_{nullptr};
  SHMCliSPtr shmCliOfRiskMgr_{nullptr};

  StgInstTaskHandlerImplSPtr stgInstTaskHandler_{nullptr};
  TaskDispatcherSPtr<SHMIPCTaskSPtr> stgInstTaskDispatcher_{nullptr};

  std::shared_ptr<std::promise<void>> barrierOfStgStartSignal_{nullptr};

  std::vector<TaskOfSyncSPtr> taskOfSyncGroup_;
  std::ext::spin_mutex mtxTaskOfSyncGroup_;

  ScheduleTaskBundleSPtr scheduleTaskBundle_{nullptr};
  std::ext::spin_mutex mtxScheduleTaskBundle_;
  SchedulerSPtr scheduleTaskBundleExecutor_{nullptr};
};

}  // namespace bq::stg
