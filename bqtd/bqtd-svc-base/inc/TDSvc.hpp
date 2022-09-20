/*!
 * \file TDSvc.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "SHMIPCMsgId.hpp"
#include "def/BQConst.hpp"
#include "def/BQDef.hpp"
#include "def/Const.hpp"
#include "util/Pch.hpp"
#include "util/StdExt.hpp"
#include "util/SvcBase.hpp"

namespace bq {
class Scheduler;
using SchedulerSPtr = std::shared_ptr<Scheduler>;

class SignalHandler;
using SignalHandlerSPtr = std::shared_ptr<SignalHandler>;

struct ScheduleTask;
using ScheduleTaskSPtr = std::shared_ptr<ScheduleTask>;
using ScheduleTaskBundle = std::vector<ScheduleTaskSPtr>;
using ScheduleTaskBundleSPtr = std::shared_ptr<ScheduleTaskBundle>;

class AssetsMgr;
using AssetsMgrSPtr = std::shared_ptr<AssetsMgr>;
class OrdMgr;
using OrdMgrSPtr = std::shared_ptr<OrdMgr>;

struct TaskOfSync;
using TaskOfSyncSPtr = std::shared_ptr<TaskOfSync>;

class FlowCtrlSvc;
using FlowCtrlSvcSPtr = std::shared_ptr<FlowCtrlSvc>;

class SHMCli;
using SHMCliSPtr = std::shared_ptr<SHMCli>;

struct SHMIPCTask;
using SHMIPCTaskSPtr = std::shared_ptr<SHMIPCTask>;

template <typename Task>
struct AsyncTask;

template <typename Task>
class TaskDispatcher;
template <typename Task>
using TaskDispatcherSPtr = std::shared_ptr<TaskDispatcher<Task>>;

enum class SyncToRiskMgr;
enum class SyncToDB;
}  // namespace bq

namespace bq::db {
class DBEng;
using DBEngSPtr = std::shared_ptr<DBEng>;
class TBLMonitorOfSymbolInfo;
using TBLMonitorOfSymbolInfoSPtr = std::shared_ptr<TBLMonitorOfSymbolInfo>;
}  // namespace bq::db

namespace bq::web {
class PingPongSvc;
using PingPongSvcSPtr = std::shared_ptr<PingPongSvc>;
}  // namespace bq::web

namespace bq::td {
class TrdSymbolCache;
using TrdSymbolCacheSPtr = std::shared_ptr<TrdSymbolCache>;
class ExternalStatusCodeCache;
using ExternalStatusCodeCacheSPtr = std::shared_ptr<ExternalStatusCodeCache>;
}  // namespace bq::td

namespace bq::td::svc {

struct AcctInfo;
using AcctInfoSPtr = std::shared_ptr<AcctInfo>;

class HttpCliOfExch;
using HttpCliOfExchSPtr = std::shared_ptr<HttpCliOfExch>;

class WSCliOfExch;
using WSCliOfExchSPtr = std::shared_ptr<WSCliOfExch>;

class TDSrvTaskHandler;
using TDSrvTaskHandlerSPtr = std::shared_ptr<TDSrvTaskHandler>;

class ExceedFlowCtrlHandler;
using ExceedFlowCtrlHandlerSPtr = std::shared_ptr<ExceedFlowCtrlHandler>;

class TDSvc : public SvcBase {
 public:
  using SvcBase::SvcBase;

 private:
  int prepareInit() final;
  int doInit() final;

 private:
  int initDBEng();

  void initTBLMonitorOfSymbolInfo();
  void initTrdSymbolCache();
  void initExternalStatusCodeCache();

  int queryMaxNoUsedToCalcPos();
  int queryAcctInfoFromDB();
  virtual void makeAcctData(const std::string& data);

  void initAssetsMgr();
  void initOrdMgr();

  int initTDSrvTaskDispatcher();
  void initSHMCliOfTDSrv();
  void initSHMCliOfRiskMgr();

 private:
  void initScheduleTaskBundle() {
    beforeInitScheduleTaskBundle();
    doInitScheduleTaskBundle();
  };

  virtual void beforeInitScheduleTaskBundle();
  virtual void doInitScheduleTaskBundle(){};

 public:
  int doRun() final;

 private:
  void sendTDGWReg();

 private:
  void doExit(const boost::system::error_code* ec, int signalNum) final;

 private:
  virtual std::tuple<int, std::string> getAddrOfWS();

 public:
  std::string getMarketCode() const { return marketCode_; }
  std::string getSymbolType() const { return symbolType_; }

  AcctId getAcctId() const { return acctId_; }

  MarketCode getMarketCodeEnum() const { return marketCodeEnum_; }
  SymbolType getSymbolTypeEnum() const { return symbolTypeEnum_; }

  std::uint64_t getNextNoUsedToCalcPos() { return ++maxNoUsedToCalcPos_; }

  std::string getAppName() const { return appName_; }

 public:
  db::DBEngSPtr getDBEng() const { return dbEng_; }

  db::TBLMonitorOfSymbolInfoSPtr getTBLMonitorOfSymbolInfo() const {
    return tblMonitorOfSymbolInfo_;
  }

  TrdSymbolCacheSPtr getTrdSymbolCache() const { return trdSymbolCache_; }
  ExternalStatusCodeCacheSPtr getExternalStatusCodeCache() const {
    return externalStatusCodeCache_;
  }

  std::any getAcctData() const { return acctData_; }

  AssetsMgrSPtr getAssetsMgr() const { return assetsMgr_; }
  OrdMgrSPtr getOrdMgr() const { return ordMgr_; }

  HttpCliOfExchSPtr getHttpCliOfExch() const { return httpCliOfExch_; }

  web::PingPongSvcSPtr getPingPongSvc() const { return pingPongSvc_; }
  WSCliOfExchSPtr getWSCliOfExch() const { return wsCliOfExch_; }

  TaskDispatcherSPtr<SHMIPCTaskSPtr> getTDSrvTaskDispatcher() const {
    return tdSrvTaskDispatcher_;
  }

  SHMCliSPtr getSHMCliOfTDSrv() const { return shmCliOfTDSrv_; }
  SHMCliSPtr getSHMCliOfRiskMgr() const { return shmCliOfRiskMgr_; }

  FlowCtrlSvcSPtr getFlowCtrlSvc() const { return flowCtrlSvc_; }
  ExceedFlowCtrlHandlerSPtr getExceedFlowCtrlHandler() const {
    return exceedFlowCtrlHandler_;
  }

  void cacheTaskOfSyncGroup(MsgId msgId, const std::any& task,
                            SyncToRiskMgr syncToRiskMgr, SyncToDB syncToDB);
  void handleTaskOfSyncGroup();

  ScheduleTaskBundleSPtr& getScheduleTaskBundle() {
    return scheduleTaskBundle_;
  };

 protected:
  void setHttpCliOfExch(const HttpCliOfExchSPtr& value) {
    httpCliOfExch_ = value;
  }

  void setPingPongSvc(const web::PingPongSvcSPtr& value) {
    pingPongSvc_ = value;
  }

  void setWSCliOfExch(const WSCliOfExchSPtr& value) { wsCliOfExch_ = value; }

 private:
  std::string marketCode_;
  std::string symbolType_;
  AcctId acctId_;

  MarketCode marketCodeEnum_;
  SymbolType symbolTypeEnum_;

  std::uint64_t maxNoUsedToCalcPos_{0};

  std::string appName_;

  db::DBEngSPtr dbEng_{nullptr};
  db::TBLMonitorOfSymbolInfoSPtr tblMonitorOfSymbolInfo_{nullptr};
  TrdSymbolCacheSPtr trdSymbolCache_{nullptr};
  ExternalStatusCodeCacheSPtr externalStatusCodeCache_{nullptr};

  AcctInfoSPtr acctInfo_{nullptr};
  std::any acctData_;

  AssetsMgrSPtr assetsMgr_{nullptr};
  OrdMgrSPtr ordMgr_{nullptr};

  HttpCliOfExchSPtr httpCliOfExch_{nullptr};

  web::PingPongSvcSPtr pingPongSvc_{nullptr};
  WSCliOfExchSPtr wsCliOfExch_{nullptr};

  TDSrvTaskHandlerSPtr tdSrvTaskHandler_{nullptr};
  TaskDispatcherSPtr<SHMIPCTaskSPtr> tdSrvTaskDispatcher_{nullptr};
  SHMCliSPtr shmCliOfTDSrv_{nullptr};
  SHMCliSPtr shmCliOfRiskMgr_{nullptr};

  FlowCtrlSvcSPtr flowCtrlSvc_{nullptr};
  ExceedFlowCtrlHandlerSPtr exceedFlowCtrlHandler_{nullptr};

  std::vector<TaskOfSyncSPtr> taskOfSyncGroup_;
  std::ext::spin_mutex mtxTaskOfSyncGroup_;

  ScheduleTaskBundleSPtr scheduleTaskBundle_{nullptr};
  SchedulerSPtr scheduleTaskBundleExecutor_{nullptr};
};

}  // namespace bq::td::svc
