#pragma once

#include "SHMIPCDef.hpp"
#include "SHMIPCMsgId.hpp"
#include "db/DBEngDef.hpp"
#include "util/Pch.hpp"
#include "util/StdExt.hpp"
#include "util/SvcBase.hpp"

namespace bq {
class PosMgr;
using PosMgrSPtr = std::shared_ptr<PosMgr>;
class AssetsMgr;
using AssetsMgrSPtr = std::shared_ptr<AssetsMgr>;
class OrdMgr;
using OrdMgrSPtr = std::shared_ptr<OrdMgr>;

struct SHMIPCTask;
using SHMIPCTaskSPtr = std::shared_ptr<SHMIPCTask>;
template <typename Task>
class TaskDispatcher;
template <typename Task>
using TaskDispatcherSPtr = std::shared_ptr<TaskDispatcher<Task>>;

class Scheduler;
using SchedulerSPtr = std::shared_ptr<Scheduler>;

struct ScheduleTask;
using ScheduleTaskSPtr = std::shared_ptr<ScheduleTask>;
using ScheduleTaskBundle = std::vector<ScheduleTaskSPtr>;
using ScheduleTaskBundleSPtr = std::shared_ptr<ScheduleTaskBundle>;

struct TaskOfSync;
using TaskOfSyncSPtr = std::shared_ptr<TaskOfSync>;

enum class SyncToRiskMgr;
enum class SyncToDB;
}  // namespace bq

namespace bq::db {
class TBLMonitorOfSymbolInfo;
using TBLMonitorOfSymbolInfoSPtr = std::shared_ptr<TBLMonitorOfSymbolInfo>;
}  // namespace bq::db

namespace bq::td::srv {

class TDSrvRiskPluginMgr;
using TDSrvRiskPluginMgrSPtr = std::shared_ptr<TDSrvRiskPluginMgr>;

class PosMgrRestorer;
using PosMgrRestorerSPtr = std::shared_ptr<PosMgrRestorer>;

class TDGWTaskHandler;
using TDGWTaskHandlerSPtr = std::shared_ptr<TDGWTaskHandler>;

class StgEngTaskHandler;
using StgEngTaskHandlerSPtr = std::shared_ptr<StgEngTaskHandler>;

class ClientChannelGroup;
using ClientChannelGroupSPtr = std::shared_ptr<ClientChannelGroup>;

class TDSrv : public SvcBase {
 public:
  using SvcBase::SvcBase;

 private:
  int prepareInit() final;
  int doInit() final;

 private:
  int initDBEng();
  void initTBLMonitorOfSymbolInfo();
  void initPosMgr();
  void initAssetsMgr();
  void initOrdMgr();
  int initTDSrvTaskDispatcher();
  void initSHMSrv();
  void initScheduleTaskBundle();

 public:
  int doRun() final;

 private:
  void doExit(const boost::system::error_code* ec, int signalNum) final;

 public:
  db::DBEngSPtr getDBEng() const { return dbEng_; }

  TDSrvRiskPluginMgrSPtr getTDSrvRiskPluginMgr() const {
    return tdSrvRiskPluginMgr_;
  }

  db::TBLMonitorOfSymbolInfoSPtr getTBLMonitorOfSymbolInfo() const {
    return tblMonitorOfSymbolInfo_;
  }

  ClientChannelGroupSPtr getTDGWGroup() const { return tdGWGroup_; }
  ClientChannelGroupSPtr getStgEngGroup() const { return stgEngGroup_; }

  TaskDispatcherSPtr<SHMIPCTaskSPtr> getTDSrvTaskDispatcher() const {
    return tdSrvTaskDispatcher_;
  }

  SHMSrvSPtr getSHMSrvOfTDGW() const { return shmSrvOfTDGW_; }
  SHMSrvSPtr getSHMSrvOfStgEng() const { return shmSrvOfStgEng_; }

  void cacheTaskOfSyncGroup(MsgId msgId, const std::any& task,
                            SyncToRiskMgr syncToRiskMgr, SyncToDB syncToDB);
  void handleTaskOfSyncGroup();

  ScheduleTaskBundleSPtr& getScheduleTaskBundle() {
    return scheduleTaskBundle_;
  };

 private:
  db::DBEngSPtr dbEng_{nullptr};
  db::TBLMonitorOfSymbolInfoSPtr tblMonitorOfSymbolInfo_{nullptr};

  TDSrvRiskPluginMgrSPtr tdSrvRiskPluginMgr_{nullptr};
  PosMgrRestorerSPtr posMgrRestorer_{nullptr};

  ClientChannelGroupSPtr tdGWGroup_{nullptr};
  ClientChannelGroupSPtr stgEngGroup_{nullptr};

  TDGWTaskHandlerSPtr tdGWTaskHandler_{nullptr};
  StgEngTaskHandlerSPtr stgEngTaskHandler_{nullptr};

  TaskDispatcherSPtr<SHMIPCTaskSPtr> tdSrvTaskDispatcher_{nullptr};

  SHMSrvSPtr shmSrvOfTDGW_{nullptr};
  SHMSrvSPtr shmSrvOfStgEng_{nullptr};

  std::vector<TaskOfSyncSPtr> taskOfSyncGroup_;
  std::ext::spin_mutex mtxTaskOfSyncGroup_;

  ScheduleTaskBundleSPtr scheduleTaskBundle_{nullptr};
  SchedulerSPtr scheduleTaskBundleExecutor_{nullptr};
};

}  // namespace bq::td::srv
