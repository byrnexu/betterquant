/*!
 * \file MDSvc.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "db/DBEngDef.hpp"
#include "def/BQConst.hpp"
#include "util/Pch.hpp"
#include "util/SvcBase.hpp"

namespace bq {
class FlowCtrlSvc;
using FlowCtrlSvcSPtr = std::shared_ptr<FlowCtrlSvc>;
class SHMSrv;
using SHMSrvSPtr = std::shared_ptr<SHMSrv>;
}  // namespace bq

namespace bq::db {
class TBLMonitorOfSymbolInfo;
using TBLMonitorOfSymbolInfoSPtr = std::shared_ptr<TBLMonitorOfSymbolInfo>;
}  // namespace bq::db

namespace bq::web {
class PingPongSvc;
using PingPongSvcSPtr = std::shared_ptr<PingPongSvc>;
}  // namespace bq::web

namespace bq::md::svc {

class ReqParser;
using ReqParserSPtr = std::shared_ptr<ReqParser>;

class RspParser;
using RspParserSPtr = std::shared_ptr<RspParser>;

class SymbolTableMaint;
using SymbolTableMaintSPtr = std::shared_ptr<SymbolTableMaint>;

class TopicGroupMustSubMaint;
using TopicGroupMustSubMaintSPtr = std::shared_ptr<TopicGroupMustSubMaint>;

class SubAndUnSubSvc;
using SubAndUnSubSvcSPtr = std::shared_ptr<SubAndUnSubSvc>;

class WSCliOfExch;
using WSCliOfExchSPtr = std::shared_ptr<WSCliOfExch>;

class SHMSrvMsgHandler;
using SHMSrvMsgHandlerSPtr = std::shared_ptr<SHMSrvMsgHandler>;

class MDStorageSvc;
using MDStorageSvcSPtr = std::shared_ptr<MDStorageSvc>;

class MDSvc : public SvcBase {
 public:
  MDSvc(const MDSvc&) = delete;
  MDSvc& operator=(const MDSvc&) = delete;
  MDSvc(const MDSvc&&) = delete;
  MDSvc& operator=(const MDSvc&&) = delete;

  using SvcBase::SvcBase;

 private:
  int prepareInit() final;
  int doInit() final;

 private:
  int initDBEng();
  void initTBLMonitorOfSymbolInfo();

 public:
  int doRun() final;

 private:
  void doExit(const boost::system::error_code* ec, int signalNum) final;

 public:
  std::string getMarketCode() const { return marketCode_; }
  std::string getSymbolType() const { return symbolType_; }

  MarketCode getMarketCodeEnum() const { return marketCodeEnum_; }
  SymbolType getSymbolTypeEnum() const { return symbolTypeEnum_; }

  bool saveMarketData() const { return saveMarketData_; }
  std::uint32_t getBooksDepthLevelOfSave() const {
    return booksDepthLevelOfSave_;
  }

 public:
  db::DBEngSPtr getDBEng() const { return dbEng_; }

  ReqParserSPtr getReqParser() const { return ReqParser_; }
  RspParserSPtr getRspParser() const { return RspParser_; }

  MDStorageSvcSPtr getMDStorageSvc() const { return mdStorageSvc_; }

  db::TBLMonitorOfSymbolInfoSPtr getTBLMonitorOfSymbolInfo() const {
    return tblMonitorOfSymbolInfo_;
  }

  FlowCtrlSvcSPtr getFlowCtrlSvc() const { return flowCtrlSvc_; }
  web::PingPongSvcSPtr getPingPongSvc() const { return pingPongSvc_; }
  WSCliOfExchSPtr getWSCliOfExch() const { return wsCliOfExch_; }
  SubAndUnSubSvcSPtr getSubAndUnSubSvc() const { return subAndUnSubSvc_; }

  TopicGroupMustSubMaintSPtr getTopicGroupMustSubMaint() const {
    return topicGroupMustSubMaint_;
  }
  SHMSrvSPtr getSHMSrv() const { return shmSrv_; }

 protected:
  void setReqParser(const ReqParserSPtr& value) { ReqParser_ = value; }
  void setRspParser(const RspParserSPtr& value) { RspParser_ = value; }

  void setSymbolTableMaint(const SymbolTableMaintSPtr& value) {
    symbolTableMaint_ = value;
  }

  void setPingPongSvc(const web::PingPongSvcSPtr& value) {
    pingPongSvc_ = value;
  }

  void setWSCliOfExch(const WSCliOfExchSPtr& value) { wsCliOfExch_ = value; }

  void setSubAndUnSubSvc(const SubAndUnSubSvcSPtr& value) {
    subAndUnSubSvc_ = value;
  }

 private:
  std::string marketCode_;
  std::string symbolType_;

  MarketCode marketCodeEnum_;
  SymbolType symbolTypeEnum_;

  db::DBEngSPtr dbEng_;

  ReqParserSPtr ReqParser_{nullptr};
  RspParserSPtr RspParser_{nullptr};

  bool saveMarketData_{false};
  std::uint32_t booksDepthLevelOfSave_{10};
  MDStorageSvcSPtr mdStorageSvc_{nullptr};

  SymbolTableMaintSPtr symbolTableMaint_{nullptr};

  db::TBLMonitorOfSymbolInfoSPtr tblMonitorOfSymbolInfo_{nullptr};

  FlowCtrlSvcSPtr flowCtrlSvc_{nullptr};
  web::PingPongSvcSPtr pingPongSvc_{nullptr};

  WSCliOfExchSPtr wsCliOfExch_{nullptr};
  SubAndUnSubSvcSPtr subAndUnSubSvc_{nullptr};
  TopicGroupMustSubMaintSPtr topicGroupMustSubMaint_{nullptr};

  SHMSrvMsgHandlerSPtr shmSrvMsgHandler_{nullptr};
  SHMSrvSPtr shmSrv_{nullptr};
};

}  // namespace bq::md::svc
