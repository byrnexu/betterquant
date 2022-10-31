/*!
 * \file PubSvc.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "PubSvc.hpp"

#include "AssetsMgr.hpp"
#include "Config.hpp"
#include "PosMgr.hpp"
#include "PosMgrUtil.hpp"
#include "RiskMgr.hpp"
#include "SHMIPC.hpp"
#include "def/DataStruOfAssets.hpp"
#include "def/DataStruOfMD.hpp"
#include "def/DataStruOfOthers.hpp"
#include "util/Datetime.hpp"
#include "util/Logger.hpp"
#include "util/MarketDataCache.hpp"
#include "util/String.hpp"

namespace bq::riskmgr {

PubSvc::PubSvc(RiskMgr* riskMgr)
    : riskMgr_(riskMgr),
      acctId2Key2PosInfoGroup_(std::make_shared<AcctId2Key2PosInfoGroup>()),
      stgId2Key2PosInfoGroup_(std::make_shared<StgId2Key2PosInfoGroup>()),
      stgInstId2Key2PosInfoGroup_(
          std::make_shared<StgInstId2Key2PosInfoGroup>()),
      acctId2Key2AssetInfoGroup_(
          std::make_shared<AcctId2Key2AssetInfoGroup>()) {}

void PubSvc::initPnlUnReal(PosInfoGroup& posInfoGroup) {
  for (auto& posInfo : posInfoGroup) {
    const auto topicPrefix = posInfo->getTopicPrefix();
    const auto topic =
        fmt::format("{}{}", topicPrefix, magic_enum::enum_name(MDType::Trades));
    const auto trades = riskMgr_->getMarketDataCache()->getLastTrades(topic);
    if (trades == nullptr) {
      posInfo->updateTime_ = UNDEFINED_FIELD_MIN_TS;
      continue;
    }

    posInfo->updateTime_ = trades->tradeTs_;
    if (posInfo->pos_ > 0) {
      posInfo->pnlUnReal_ =
          calcPnlOfCloseLong(posInfo->symbolType_, posInfo->avgOpenPrice_,
                             trades->price_, posInfo->pos_, posInfo->parValue_);
    } else if (posInfo->pos_ < 0) {
      posInfo->pnlUnReal_ = calcPnlOfCloseShort(
          posInfo->symbolType_, posInfo->avgOpenPrice_, trades->price_,
          posInfo->pos_ * -1, posInfo->parValue_);
    } else {
      posInfo->pnlUnReal_ = 0;
    }
  }
}

void PubSvc::pubPosUpdateOfAcctId() {
  auto posInfoGroup = riskMgr_->getPosMgr()->getPosInfoGroup(LockFunc::True);
  MergePosInfoHasNoFeeCurrency(posInfoGroup);

  initPnlUnReal(posInfoGroup);

  const auto acctId2Key2PosInfoGroup = posInfoGroupByAcctId(posInfoGroup);

  const auto acctId2Key2PosInfoGroupUpdate =
      getAcctId2Key2PosInfoGroupUpdate(acctId2Key2PosInfoGroup);

  pushAcctId2Key2PosInfoGroup(MSG_ID_POS_UPDATE_OF_ACCT_ID,
                              acctId2Key2PosInfoGroupUpdate);

  acctId2Key2PosInfoGroup_ = acctId2Key2PosInfoGroup;
}

void PubSvc::pubPosSnapshotOfAcctId() {
  auto posInfoGroup = riskMgr_->getPosMgr()->getPosInfoGroup(LockFunc::True);
  MergePosInfoHasNoFeeCurrency(posInfoGroup);

  initPnlUnReal(posInfoGroup);

  const auto acctId2Key2PosInfoGroup = posInfoGroupByAcctId(posInfoGroup);

  pushAcctId2Key2PosInfoGroup(MSG_ID_POS_SNAPSHOT_OF_ACCT_ID,
                              acctId2Key2PosInfoGroup);
}

AcctId2Key2PosInfoGroupSPtr PubSvc::posInfoGroupByAcctId(
    const PosInfoGroup& posInfoGroup) {
  AcctId2Key2PosInfoGroupSPtr ret = std::make_shared<AcctId2Key2PosInfoGroup>();
  for (const auto& posInfo : posInfoGroup) {
    const auto iter = ret->find(posInfo->acctId_);
    if (iter != std::end(*ret)) {
      (*ret)[posInfo->acctId_]->emplace(posInfo->getKey(), posInfo);
    } else {
      auto key2PosInfoGroup = std::make_shared<Key2PosInfoGroup>();
      key2PosInfoGroup->emplace(posInfo->getKey(), posInfo);
      (*ret)[posInfo->acctId_] = key2PosInfoGroup;
    }
  }
  return ret;
}

AcctId2Key2PosInfoGroupSPtr PubSvc::getAcctId2Key2PosInfoGroupUpdate(
    const AcctId2Key2PosInfoGroupSPtr& acctId2Key2PosInfoGroupNew) {
  AcctId2Key2PosInfoGroupSPtr ret = std::make_shared<AcctId2Key2PosInfoGroup>();

  for (const auto& acctId2Key2PosInfoNew : *acctId2Key2PosInfoGroupNew) {
    const auto acctId = acctId2Key2PosInfoNew.first;
    const auto iter = acctId2Key2PosInfoGroup_->find(acctId);
    if (iter == std::end(*acctId2Key2PosInfoGroup_)) {
      ret->emplace(acctId2Key2PosInfoNew);

    } else {
      const auto& key2PosInfoGroupNew = acctId2Key2PosInfoNew.second;
      const auto& Key2PosInfoGroup = iter->second;
      if (isEqual(key2PosInfoGroupNew, Key2PosInfoGroup) == false) {
        ret->emplace(acctId2Key2PosInfoNew);
      }
    }
  }

  for (const auto& acctId2Key2PosInfo : *acctId2Key2PosInfoGroup_) {
    const auto acctId = acctId2Key2PosInfo.first;
    const auto iter = acctId2Key2PosInfoGroupNew->find(acctId);
    if (iter == std::end(*acctId2Key2PosInfoGroupNew)) {
      auto key2PosInfoGroup = std::make_shared<Key2PosInfoGroup>();
      (*ret)[acctId] = key2PosInfoGroup;
    }
  }

  return ret;
}

// topic = "RISK@PubChannel@Trade@PosInfo@AcctId@10001"
void PubSvc::pushAcctId2Key2PosInfoGroup(
    MsgId msgId, const AcctId2Key2PosInfoGroupSPtr& acctId2Key2PosInfoGroup) {
  const auto makeTopicHash = [&](AcctId acctId) {
    const auto pubChannel = CONFIG["pubChannel"].as<std::string>();
    const auto topic =
        fmt::format("{}{}PosInfo{}AcctId{}{}", pubChannel, SEP_OF_TOPIC,
                    SEP_OF_TOPIC, SEP_OF_TOPIC, acctId);
    if (msgId == MSG_ID_POS_UPDATE_OF_ACCT_ID) LOG_I("Pub {}", topic);
    const auto topicHash = XXH3_64bits(topic.data(), topic.size());
    return topicHash;
  };

  for (const auto& acctId2Key2PosInfo : *acctId2Key2PosInfoGroup) {
    const auto acctId = acctId2Key2PosInfo.first;
    const auto& key2PosInfoGroup = acctId2Key2PosInfo.second;
    riskMgr_->getSHMSrvOfPub()->pushMsgWithZeroCopy(
        [&](void* shmBuf) {
          auto posUpdateOfAcctIdForPub =
              static_cast<PosUpdateOfAcctIdForPub*>(shmBuf);
          posUpdateOfAcctIdForPub->shmHeader_.topicHash_ =
              makeTopicHash(acctId);
          posUpdateOfAcctIdForPub->acctId_ = acctId;
          posUpdateOfAcctIdForPub->num_ = key2PosInfoGroup->size();
          auto posInfoAddr = posUpdateOfAcctIdForPub->posInfoGroup_;
          for (const auto& key2PosInfo : *key2PosInfoGroup) {
            memcpy(posInfoAddr, key2PosInfo.second.get(), sizeof(PosInfo));
            posInfoAddr += sizeof(PosInfo);
          }
        },
        PUB_CHANNEL, msgId,
        sizeof(PosUpdateOfAcctIdForPub) +
            key2PosInfoGroup->size() * sizeof(PosInfo));
  }
}

void PubSvc::pubPosUpdateOfStgId() {
  auto posInfoGroup = riskMgr_->getPosMgr()->getPosInfoGroup(LockFunc::True);
  MergePosInfoHasNoFeeCurrency(posInfoGroup);

  initPnlUnReal(posInfoGroup);

  const auto stgId2Key2PosInfoGroup = posInfoGroupByStgId(posInfoGroup);

  const auto stgId2Key2PosInfoGroupUpdate =
      getStgId2Key2PosInfoGroupUpdate(stgId2Key2PosInfoGroup);

  pushStgId2Key2PosInfoGroup(MSG_ID_POS_UPDATE_OF_STG_ID,
                             stgId2Key2PosInfoGroupUpdate);

  stgId2Key2PosInfoGroup_ = stgId2Key2PosInfoGroup;
}

void PubSvc::pubPosSnapshotOfStgId() {
  auto posInfoGroup = riskMgr_->getPosMgr()->getPosInfoGroup(LockFunc::True);
  MergePosInfoHasNoFeeCurrency(posInfoGroup);

  initPnlUnReal(posInfoGroup);

  const auto stgId2Key2PosInfoGroup = posInfoGroupByStgId(posInfoGroup);

  pushStgId2Key2PosInfoGroup(MSG_ID_POS_SNAPSHOT_OF_STG_ID,
                             stgId2Key2PosInfoGroup);
}

StgId2Key2PosInfoGroupSPtr PubSvc::posInfoGroupByStgId(
    const PosInfoGroup& posInfoGroup) {
  StgId2Key2PosInfoGroupSPtr ret = std::make_shared<StgId2Key2PosInfoGroup>();
  for (const auto& posInfo : posInfoGroup) {
    const auto iter = ret->find(posInfo->stgId_);
    if (iter != std::end(*ret)) {
      (*ret)[posInfo->stgId_]->emplace(posInfo->getKey(), posInfo);
    } else {
      auto key2PosInfoGroup = std::make_shared<Key2PosInfoGroup>();
      key2PosInfoGroup->emplace(posInfo->getKey(), posInfo);
      (*ret)[posInfo->stgId_] = key2PosInfoGroup;
    }
  }
  return ret;
}

StgId2Key2PosInfoGroupSPtr PubSvc::getStgId2Key2PosInfoGroupUpdate(
    const StgId2Key2PosInfoGroupSPtr& stgId2Key2PosInfoGroupNew) {
  StgId2Key2PosInfoGroupSPtr ret = std::make_shared<StgId2Key2PosInfoGroup>();

  for (const auto& stgId2Key2PosInfoNew : *stgId2Key2PosInfoGroupNew) {
    const auto stgId = stgId2Key2PosInfoNew.first;
    const auto iter = stgId2Key2PosInfoGroup_->find(stgId);
    if (iter == std::end(*stgId2Key2PosInfoGroup_)) {
      ret->emplace(stgId2Key2PosInfoNew);

    } else {
      const auto& key2PosInfoGroupNew = stgId2Key2PosInfoNew.second;
      const auto& Key2PosInfoGroup = iter->second;
      if (isEqual(key2PosInfoGroupNew, Key2PosInfoGroup) == false) {
        ret->emplace(stgId2Key2PosInfoNew);
      }
    }
  }

  for (const auto& stgId2Key2PosInfo : *stgId2Key2PosInfoGroup_) {
    const auto stgId = stgId2Key2PosInfo.first;
    const auto iter = stgId2Key2PosInfoGroupNew->find(stgId);
    if (iter == std::end(*stgId2Key2PosInfoGroupNew)) {
      auto key2PosInfoGroup = std::make_shared<Key2PosInfoGroup>();
      (*ret)[stgId] = key2PosInfoGroup;
    }
  }

  return ret;
}

// topic = "RISK@PubChannel@Trade@PosInfo@StgId@10000"
void PubSvc::pushStgId2Key2PosInfoGroup(
    MsgId msgId, const StgId2Key2PosInfoGroupSPtr& stgId2Key2PosInfoGroup) {
  const auto makeTopicHash = [&](StgId stgId) {
    const auto pubChannel = CONFIG["pubChannel"].as<std::string>();
    const auto topic =
        fmt::format("{}{}PosInfo{}StgId{}{}", pubChannel, SEP_OF_TOPIC,
                    SEP_OF_TOPIC, SEP_OF_TOPIC, stgId);
    if (msgId == MSG_ID_POS_UPDATE_OF_STG_ID) LOG_I("Pub {}", topic);
    const auto topicHash = XXH3_64bits(topic.data(), topic.size());
    return topicHash;
  };

  for (const auto& stgId2Key2PosInfo : *stgId2Key2PosInfoGroup) {
    const auto stgId = stgId2Key2PosInfo.first;
    const auto& key2PosInfoGroup = stgId2Key2PosInfo.second;
    riskMgr_->getSHMSrvOfPub()->pushMsgWithZeroCopy(
        [&](void* shmBuf) {
          auto posUpdateOfStgIdForPub =
              static_cast<PosUpdateOfStgIdForPub*>(shmBuf);
          posUpdateOfStgIdForPub->shmHeader_.topicHash_ = makeTopicHash(stgId);
          posUpdateOfStgIdForPub->stgId_ = stgId;
          posUpdateOfStgIdForPub->num_ = key2PosInfoGroup->size();
          auto posInfoAddr = posUpdateOfStgIdForPub->posInfoGroup_;
          for (const auto& key2PosInfo : *key2PosInfoGroup) {
            memcpy(posInfoAddr, key2PosInfo.second.get(), sizeof(PosInfo));
            posInfoAddr += sizeof(PosInfo);
          }
        },
        PUB_CHANNEL, msgId,
        sizeof(PosUpdateOfStgIdForPub) +
            key2PosInfoGroup->size() * sizeof(PosInfo));
  }
}

void PubSvc::pubPosUpdateOfStgInstId() {
  auto posInfoGroup = riskMgr_->getPosMgr()->getPosInfoGroup(LockFunc::True);
  MergePosInfoHasNoFeeCurrency(posInfoGroup);

  initPnlUnReal(posInfoGroup);

  const auto stgInstId2Key2PosInfoGroup = posInfoGroupByStgInstId(posInfoGroup);

  const auto stgInstId2Key2PosInfoGroupUpdate =
      getStgInstId2Key2PosInfoGroupUpdate(stgInstId2Key2PosInfoGroup);

  pushStgInstId2Key2PosInfoGroup(MSG_ID_POS_UPDATE_OF_STG_INST_ID,
                                 stgInstId2Key2PosInfoGroupUpdate);

  stgInstId2Key2PosInfoGroup_ = stgInstId2Key2PosInfoGroup;
}

void PubSvc::pubPosSnapshotOfStgInstId() {
  auto posInfoGroup = riskMgr_->getPosMgr()->getPosInfoGroup(LockFunc::True);
  MergePosInfoHasNoFeeCurrency(posInfoGroup);

  initPnlUnReal(posInfoGroup);

  const auto stgId2Key2PosInfoGroup = posInfoGroupByStgInstId(posInfoGroup);

  pushStgInstId2Key2PosInfoGroup(MSG_ID_POS_SNAPSHOT_OF_STG_INST_ID,
                                 stgId2Key2PosInfoGroup);
}

StgInstId2Key2PosInfoGroupSPtr PubSvc::posInfoGroupByStgInstId(
    const PosInfoGroup& posInfoGroup) {
  StgInstId2Key2PosInfoGroupSPtr ret =
      std::make_shared<StgInstId2Key2PosInfoGroup>();
  for (const auto& posInfo : posInfoGroup) {
    const auto instKey =
        fmt::format("{}-{}", posInfo->stgId_, posInfo->stgInstId_);
    const auto iter = ret->find(instKey);
    if (iter != std::end(*ret)) {
      (*ret)[instKey]->emplace(posInfo->getKey(), posInfo);
    } else {
      auto key2PosInfoGroup = std::make_shared<Key2PosInfoGroup>();
      key2PosInfoGroup->emplace(posInfo->getKey(), posInfo);
      (*ret)[instKey] = key2PosInfoGroup;
    }
  }
  return ret;
}

StgInstId2Key2PosInfoGroupSPtr PubSvc::getStgInstId2Key2PosInfoGroupUpdate(
    const StgInstId2Key2PosInfoGroupSPtr& stgInstId2Key2PosInfoGroupNew) {
  StgInstId2Key2PosInfoGroupSPtr ret =
      std::make_shared<StgInstId2Key2PosInfoGroup>();

  for (const auto& stgInstId2Key2PosInfoNew : *stgInstId2Key2PosInfoGroupNew) {
    const auto& instKey = stgInstId2Key2PosInfoNew.first;
    const auto iter = stgInstId2Key2PosInfoGroup_->find(instKey);
    if (iter == std::end(*stgInstId2Key2PosInfoGroup_)) {
      ret->emplace(stgInstId2Key2PosInfoNew);

    } else {
      const auto& key2PosInfoGroupNew = stgInstId2Key2PosInfoNew.second;
      const auto& Key2PosInfoGroup = iter->second;
      if (isEqual(key2PosInfoGroupNew, Key2PosInfoGroup) == false) {
        ret->emplace(stgInstId2Key2PosInfoNew);
      }
    }
  }

  for (const auto& stgInstId2Key2PosInfo : *stgInstId2Key2PosInfoGroup_) {
    const auto instKey = stgInstId2Key2PosInfo.first;
    const auto iter = stgInstId2Key2PosInfoGroupNew->find(instKey);
    if (iter == std::end(*stgInstId2Key2PosInfoGroupNew)) {
      auto key2PosInfoGroup = std::make_shared<Key2PosInfoGroup>();
      (*ret)[instKey] = key2PosInfoGroup;
    }
  }

  return ret;
}

// topic = "RISK@PubChannel@Trade@PosInfo@StgId@10000@StgInstId@1"
void PubSvc::pushStgInstId2Key2PosInfoGroup(
    MsgId msgId,
    const StgInstId2Key2PosInfoGroupSPtr& stgInstId2Key2PosInfoGroup) {
  const auto makeTopicHash = [&](StgId stgId, StgInstId stgInstId) {
    const auto pubChannel = CONFIG["pubChannel"].as<std::string>();
    const auto topic =
        fmt::format("{}{}PosInfo{}StgId{}{}{}StgInstId{}{}", pubChannel,
                    SEP_OF_TOPIC, SEP_OF_TOPIC, SEP_OF_TOPIC, stgId,
                    SEP_OF_TOPIC, SEP_OF_TOPIC, stgInstId);
    if (msgId == MSG_ID_POS_UPDATE_OF_STG_INST_ID) LOG_I("Pub {}", topic);
    const auto topicHash = XXH3_64bits(topic.data(), topic.size());
    return topicHash;
  };

  for (const auto& stgInstId2Key2PosInfo : *stgInstId2Key2PosInfoGroup) {
    const auto instKey = stgInstId2Key2PosInfo.first;
    const auto [ret, stgIdInStrFmt, stgInstIdInStrFmt] =
        SplitStrIntoTwoParts(instKey, "-");
    const auto stgId = CONV(StgId, stgIdInStrFmt);
    const auto stgInstId = CONV(StgInstId, stgInstIdInStrFmt);
    const auto& key2PosInfoGroup = stgInstId2Key2PosInfo.second;
    riskMgr_->getSHMSrvOfPub()->pushMsgWithZeroCopy(
        [&](void* shmBuf) {
          auto posUpdateOfStgInstIdForPub =
              static_cast<PosUpdateOfStgInstIdForPub*>(shmBuf);
          posUpdateOfStgInstIdForPub->shmHeader_.topicHash_ =
              makeTopicHash(stgId, stgInstId);
          posUpdateOfStgInstIdForPub->stgId_ = stgId;
          posUpdateOfStgInstIdForPub->stgInstId_ = stgInstId;
          posUpdateOfStgInstIdForPub->num_ = key2PosInfoGroup->size();
          auto posInfoAddr = posUpdateOfStgInstIdForPub->posInfoGroup_;
          for (const auto& key2PosInfo : *key2PosInfoGroup) {
            memcpy(posInfoAddr, key2PosInfo.second.get(), sizeof(PosInfo));
            posInfoAddr += sizeof(PosInfo);
          }
        },
        PUB_CHANNEL, msgId,
        sizeof(PosUpdateOfStgInstIdForPub) +
            key2PosInfoGroup->size() * sizeof(PosInfo));
  }
}

void PubSvc::pubAssetsUpdate() {
  auto assetInfoGroup =
      riskMgr_->getAssetsMgr()->getAssetInfoGroup(LockFunc::True);

  const auto acctId2Key2AssetInfoGroup = assetInfoGroupByAcctId(assetInfoGroup);

  const auto acctId2Key2AssetInfoGroupUpdate =
      getAcctId2Key2AssetInfoGroupUpdate(acctId2Key2AssetInfoGroup);

  pushAcctId2Key2AssetInfoGroup(MSG_ID_ASSETS_UPDATE,
                                acctId2Key2AssetInfoGroupUpdate);

  acctId2Key2AssetInfoGroup_ = acctId2Key2AssetInfoGroup;
}

void PubSvc::pubAssetsSnapshot() {
  auto assetInfoGroup =
      riskMgr_->getAssetsMgr()->getAssetInfoGroup(LockFunc::True);

  const auto acctId2Key2AssetInfoGroup = assetInfoGroupByAcctId(assetInfoGroup);

  pushAcctId2Key2AssetInfoGroup(MSG_ID_ASSETS_SNAPSHOT,
                                acctId2Key2AssetInfoGroup);
}

AcctId2Key2AssetInfoGroupSPtr PubSvc::assetInfoGroupByAcctId(
    const std::vector<AssetInfoSPtr>& assetInfoGroup) {
  AcctId2Key2AssetInfoGroupSPtr ret =
      std::make_shared<AcctId2Key2AssetInfoGroup>();
  for (const auto& assetInfo : assetInfoGroup) {
    const auto iter = ret->find(assetInfo->acctId_);
    if (iter != std::end(*ret)) {
      (*ret)[assetInfo->acctId_]->emplace(assetInfo->getKey(), assetInfo);
    } else {
      auto key2AssetInfoGroup = std::make_shared<Key2AssetInfoGroup>();
      key2AssetInfoGroup->emplace(assetInfo->getKey(), assetInfo);
      (*ret)[assetInfo->acctId_] = key2AssetInfoGroup;
    }
  }
  return ret;
}

AcctId2Key2AssetInfoGroupSPtr PubSvc::getAcctId2Key2AssetInfoGroupUpdate(
    const AcctId2Key2AssetInfoGroupSPtr& acctId2Key2AssetInfoGroupNew) {
  AcctId2Key2AssetInfoGroupSPtr ret =
      std::make_shared<AcctId2Key2AssetInfoGroup>();

  for (const auto& acctId2Key2AssetInfoNew : *acctId2Key2AssetInfoGroupNew) {
    const auto acctId = acctId2Key2AssetInfoNew.first;
    const auto iter = acctId2Key2AssetInfoGroup_->find(acctId);
    if (iter == std::end(*acctId2Key2AssetInfoGroup_)) {
      ret->emplace(acctId2Key2AssetInfoNew);

    } else {
      const auto& key2AssetInfoGroupNew = acctId2Key2AssetInfoNew.second;
      const auto& Key2AssetInfoGroup = iter->second;
      if (isEqual(key2AssetInfoGroupNew, Key2AssetInfoGroup) == false) {
        ret->emplace(acctId2Key2AssetInfoNew);
      }
    }
  }

  for (const auto& acctId2Key2AssetInfo : *acctId2Key2AssetInfoGroup_) {
    const auto acctId = acctId2Key2AssetInfo.first;
    const auto iter = acctId2Key2AssetInfoGroupNew->find(acctId);
    if (iter == std::end(*acctId2Key2AssetInfoGroupNew)) {
      auto key2AssetInfoGroup = std::make_shared<Key2AssetInfoGroup>();
      (*ret)[acctId] = key2AssetInfoGroup;
    }
  }

  return ret;
}

// topic = "RISK@PubChannel@Trade@AssetInfo@AcctId@10001"
void PubSvc::pushAcctId2Key2AssetInfoGroup(
    MsgId msgId,
    const AcctId2Key2AssetInfoGroupSPtr& acctId2Key2AssetInfoGroup) {
  const auto makeTopicHash = [&](AcctId acctId) {
    const auto pubChannel = CONFIG["pubChannel"].as<std::string>();
    const auto topic =
        fmt::format("{}{}AssetInfo{}AcctId{}{}", pubChannel, SEP_OF_TOPIC,
                    SEP_OF_TOPIC, SEP_OF_TOPIC, acctId);
    if (msgId == MSG_ID_ASSETS_UPDATE) LOG_I("Pub {}", topic);
    const auto topicHash = XXH3_64bits(topic.data(), topic.size());
    return topicHash;
  };

  for (const auto& acctId2Key2AssetInfo : *acctId2Key2AssetInfoGroup) {
    const auto acctId = acctId2Key2AssetInfo.first;
    const auto& key2AssetInfoGroup = acctId2Key2AssetInfo.second;
    riskMgr_->getSHMSrvOfPub()->pushMsgWithZeroCopy(
        [&](void* shmBuf) {
          auto assetsUpdateForPub = static_cast<AssetsUpdateForPub*>(shmBuf);
          assetsUpdateForPub->shmHeader_.topicHash_ = makeTopicHash(acctId);
          assetsUpdateForPub->acctId_ = acctId;
          assetsUpdateForPub->num_ = key2AssetInfoGroup->size();
          auto assetInfoAddr = assetsUpdateForPub->assetInfoGroup_;
          for (const auto& key2AssetInfo : *key2AssetInfoGroup) {
            memcpy(assetInfoAddr, key2AssetInfo.second.get(),
                   sizeof(AssetInfo));
            assetInfoAddr += sizeof(AssetInfo);
          }
        },
        PUB_CHANNEL, msgId,
        sizeof(AssetsUpdateForPub) +
            key2AssetInfoGroup->size() * sizeof(AssetInfo));
  }
}

}  // namespace bq::riskmgr
