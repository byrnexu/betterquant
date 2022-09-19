#pragma once

#include "SHMHeader.hpp"
#include "def/BQConstIF.hpp"
#include "def/BQDefIF.hpp"
#include "util/PchBase.hpp"

namespace bq {

struct SymbolInfo;
using SymbolInfoSPtr = std::shared_ptr<SymbolInfo>;

struct Pnl;
using PnlSPtr = std::shared_ptr<Pnl>;

class MarketDataCache;
using MarketDataCacheSPtr = std::shared_ptr<MarketDataCache>;

struct PosInfo;
using PosInfoSPtr = std::shared_ptr<PosInfo>;

using PosInfoGroup = std::vector<PosInfoSPtr>;
using PosInfoGroupSPtr = std::shared_ptr<PosInfoGroup>;

struct PosInfo {
  PosInfo();

  std::uint64_t keyHash_;

  UserId userId_;
  AcctId acctId_;
  StgId stgId_;
  StgInstId stgInstId_;

  MarketCode marketCode_;
  SymbolType symbolType_;
  char symbolCode_[MAX_SYMBOL_CODE_LEN];

  Side side_{Side::Others};
  PosSide posSide_{PosSide::Others};

  std::uint32_t parValue_;
  char feeCurrency_[MAX_CURRENCY_LEN];

  Decimal fee_{0};
  Decimal pos_{0};
  Decimal prePos_{0};
  Decimal avgOpenPrice_{0};
  Decimal pnlUnReal_{0};
  Decimal pnlReal_{0};
  Decimal totalBidSize_{0};
  Decimal totalAskSize_{0};
  std::uint64_t lastNoUsedToCalcPos_{0};
  std::uint64_t updateTime_;

  bool isEqual(const PosInfoSPtr& posInfo);

  std::vector<SymbolInfoSPtr> getSymbolGroupUsedToCalcPnl(
      const std::string& quoteCurrency,
      const std::string& quoteCurrencyForConv) const;

  PnlSPtr calcPnl(const MarketDataCacheSPtr& marketDataCache,
                  const std::string& quoteCurrency,
                  const std::string& quoteCurrencyForConv,
                  const std::string& quoteCurrencyOfOrig) const;

  std::string toStr() const;
  std::string getKey() const;

  std::string getKeyWithoutFeeCurrency() const;
  std::string getTopicPrefix() const;

  bool oneMoreFeeCurrencyThanInput(const PosInfoSPtr& posInfo) const;

  std::string getSqlOfReplace() const;
  std::string getSqlOfInsert() const;
  std::string getSqlOfUpdate() const;
  std::string getSqlOfDelete() const;
};

using Key2PosInfoGroup = std::map<std::string, PosInfoSPtr>;
using Key2PosInfoGroupSPtr = std::shared_ptr<Key2PosInfoGroup>;
using AcctId2Key2PosInfoGroup = std::map<AcctId, Key2PosInfoGroupSPtr>;
using AcctId2Key2PosInfoGroupSPtr = std::shared_ptr<AcctId2Key2PosInfoGroup>;
bool isEqual(const Key2PosInfoGroupSPtr& lhs, const Key2PosInfoGroupSPtr& rhs);

struct PosUpdateOfAcctIdForPub {
  SHMHeader shmHeader_{MSG_ID_POS_UPDATE_OF_ACCT_ID};
  AcctId acctId_{0};
  std::uint16_t num_;
  char posInfoGroup_[0];
};
using PosUpdateOfAcctIdForPubSPtr = std::shared_ptr<PosUpdateOfAcctIdForPub>;
Key2PosInfoGroup MakePosUpdateOfAcctId(
    const PosUpdateOfAcctIdForPubSPtr& posUpdateOfAcctIdForPub);

struct PosUpdateOfStgIdForPub {
  SHMHeader shmHeader_{MSG_ID_POS_UPDATE_OF_STG_ID};
  StgId stgId_{0};
  std::uint16_t num_;
  char posInfoGroup_[0];
};
using PosUpdateOfStgIdForPubSPtr = std::shared_ptr<PosUpdateOfStgIdForPub>;
Key2PosInfoGroup MakePosUpdateOfStgId(
    const PosUpdateOfStgIdForPubSPtr& posUpdateOfStgIdForPub);

struct PosUpdateOfStgInstIdForPub {
  SHMHeader shmHeader_{MSG_ID_POS_UPDATE_OF_STG_INST_ID};
  StgId stgId_{0};
  StgInstId stgInstId_{0};
  std::uint16_t num_;
  char posInfoGroup_[0];
};
using PosUpdateOfStgInstIdForPubSPtr =
    std::shared_ptr<PosUpdateOfStgInstIdForPub>;
Key2PosInfoGroup MakePosUpdateOfStgInstId(
    const PosUpdateOfStgInstIdForPubSPtr& posUpdateOfStgInstIdForPub);

using Key2PosInfoBundle = std::map<std::string, PosInfoGroupSPtr>;
using Key2PosInfoBundleSPtr = std::shared_ptr<Key2PosInfoBundle>;

Decimal CalcAvgOpenPrice(const PosInfoSPtr& lhs, const PosInfoSPtr& rhs);

void MergePosInfoHasNoFeeCurrency(PosInfoGroup& posInfoGroup);

}  // namespace bq
