#pragma once

#include "SHMIPCMsgId.hpp"
#include "def/BQConst.hpp"
#include "def/BQDef.hpp"
#include "def/Const.hpp"
#include "def/Def.hpp"
#include "def/PosInfo.hpp"
#include "util/Pch.hpp"

namespace bq {
struct PosInfo;
using PosInfoSPtr = std::shared_ptr<PosInfo>;

using Key2PosInfoGroup = std::map<std::string, PosInfoSPtr>;
using Key2PosInfoGroupSPtr = std::shared_ptr<Key2PosInfoGroup>;

using AcctId2Key2PosInfoGroup = std::map<AcctId, Key2PosInfoGroupSPtr>;
using AcctId2Key2PosInfoGroupSPtr = std::shared_ptr<AcctId2Key2PosInfoGroup>;

using StgId2Key2PosInfoGroup = std::map<StgId, Key2PosInfoGroupSPtr>;
using StgId2Key2PosInfoGroupSPtr = std::shared_ptr<StgId2Key2PosInfoGroup>;

using StgInstId2Key2PosInfoGroup = std::map<std::string, Key2PosInfoGroupSPtr>;
using StgInstId2Key2PosInfoGroupSPtr =
    std::shared_ptr<StgInstId2Key2PosInfoGroup>;

struct AssetInfo;
using AssetInfoSPtr = std::shared_ptr<AssetInfo>;

using Key2AssetInfoGroup = std::map<std::string, AssetInfoSPtr>;
using Key2AssetInfoGroupSPtr = std::shared_ptr<Key2AssetInfoGroup>;

using AcctId2Key2AssetInfoGroup = std::map<AcctId, Key2AssetInfoGroupSPtr>;
using AcctId2Key2AssetInfoGroupSPtr =
    std::shared_ptr<AcctId2Key2AssetInfoGroup>;
}  // namespace bq

namespace bq::riskmgr {

class RiskMgr;

class PubSvc {
 public:
  PubSvc(const PubSvc&) = delete;
  PubSvc& operator=(const PubSvc&) = delete;
  PubSvc(const PubSvc&&) = delete;
  PubSvc& operator=(const PubSvc&&) = delete;

  explicit PubSvc(RiskMgr* riskMgr);

 private:
  void initPnlUnReal(PosInfoGroup& posInfoGroup);

 public:
  void pubPosUpdateOfAcctId();
  void pubPosSnapshotOfAcctId();

 private:
  AcctId2Key2PosInfoGroupSPtr posInfoGroupByAcctId(
      const PosInfoGroup& posInfoGroup);
  AcctId2Key2PosInfoGroupSPtr getAcctId2Key2PosInfoGroupUpdate(
      const AcctId2Key2PosInfoGroupSPtr& lastRecSet);
  void pushAcctId2Key2PosInfoGroup(
      MsgId msgId, const AcctId2Key2PosInfoGroupSPtr& acctId2Key2PosInfoGroup);

 public:
  void pubPosUpdateOfStgId();
  void pubPosSnapshotOfStgId();

 private:
  StgId2Key2PosInfoGroupSPtr posInfoGroupByStgId(
      const PosInfoGroup& posInfoGroup);
  StgId2Key2PosInfoGroupSPtr getStgId2Key2PosInfoGroupUpdate(
      const StgId2Key2PosInfoGroupSPtr& lastRecSet);
  void pushStgId2Key2PosInfoGroup(
      MsgId msgId, const StgId2Key2PosInfoGroupSPtr& stgId2Key2PosInfoGroup);

 public:
  void pubPosUpdateOfStgInstId();
  void pubPosSnapshotOfStgInstId();

 private:
  StgInstId2Key2PosInfoGroupSPtr posInfoGroupByStgInstId(
      const PosInfoGroup& posInfoGroup);
  StgInstId2Key2PosInfoGroupSPtr getStgInstId2Key2PosInfoGroupUpdate(
      const StgInstId2Key2PosInfoGroupSPtr& lastRecSet);
  void pushStgInstId2Key2PosInfoGroup(
      MsgId msgId,
      const StgInstId2Key2PosInfoGroupSPtr& stgInstId2Key2PosInfoGroup);

 public:
  void pubAssetsUpdate();
  void pubAssetsSnapshot();

 private:
  AcctId2Key2AssetInfoGroupSPtr assetInfoGroupByAcctId(
      const std::vector<AssetInfoSPtr>& assetInfoGroup);
  AcctId2Key2AssetInfoGroupSPtr getAcctId2Key2AssetInfoGroupUpdate(
      const AcctId2Key2AssetInfoGroupSPtr& lastRecSet);
  void pushAcctId2Key2AssetInfoGroup(
      MsgId msgId,
      const AcctId2Key2AssetInfoGroupSPtr& acctId2Key2AssetInfoGroup);

 private:
  RiskMgr* riskMgr_{nullptr};

  AcctId2Key2PosInfoGroupSPtr acctId2Key2PosInfoGroup_{nullptr};
  StgId2Key2PosInfoGroupSPtr stgId2Key2PosInfoGroup_{nullptr};
  StgInstId2Key2PosInfoGroupSPtr stgInstId2Key2PosInfoGroup_{nullptr};
  AcctId2Key2AssetInfoGroupSPtr acctId2Key2AssetInfoGroup_{nullptr};
};

}  // namespace bq::riskmgr
