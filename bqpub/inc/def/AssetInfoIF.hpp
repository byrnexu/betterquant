/*!
 * \file AssetInfoIF.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "SHMHeader.hpp"
#include "def/BQConstIF.hpp"
#include "def/BQDefIF.hpp"
#include "util/PchBase.hpp"

namespace bq {

struct AssetInfo;
using AssetInfoSPtr = std::shared_ptr<AssetInfo>;

struct AssetInfo {
  AcctId acctId_;
  MarketCode marketCode_{MarketCode::Others};
  SymbolType symbolType_{SymbolType::Others};
  char assetName_[MAX_ASSETS_NAME_LEN];
  Decimal vol_;
  Decimal crossVol_;
  Decimal frozen_;
  Decimal available_;
  Decimal pnlUnreal_;
  Decimal maxWithdraw_;
  std::uint64_t updateTime_;

  mutable std::uint64_t keyHash_{0};

  bool isEqual(const AssetInfoSPtr& assetInfo);

  std::string toStr() const;
  std::string getKey() const;
  void initKeyHash() const;

  std::string getSqlOfInsert() const;
  std::string getSqlOfUpdate() const;
  std::string getSqlOfDelete() const;
};

using Key2AssetInfoGroup = std::map<std::string, AssetInfoSPtr>;
using Key2AssetInfoGroupSPtr = std::shared_ptr<Key2AssetInfoGroup>;
using AcctId2Key2AssetInfoGroup = std::map<AcctId, Key2AssetInfoGroupSPtr>;
using AcctId2Key2AssetInfoGroupSPtr =
    std::shared_ptr<AcctId2Key2AssetInfoGroup>;
bool isEqual(const Key2AssetInfoGroupSPtr& lhs,
             const Key2AssetInfoGroupSPtr& rhs);

struct AssetsUpdateForPub {
  SHMHeader shmHeader_{MSG_ID_ASSETS_UPDATE};
  AcctId acctId_{0};
  std::uint16_t num_;
  char assetInfoGroup_[0];
};
using AssetsSnapshot = std::map<std::string, AssetInfoSPtr>;
using AssetsSnapshotSPtr = std::shared_ptr<AssetsSnapshot>;
using AssetsUpdateForPubSPtr = std::shared_ptr<AssetsUpdateForPub>;
using AssetsUpdate = std::map<std::string, AssetInfoSPtr>;
using AssetsUpdateSPtr = std::shared_ptr<AssetsUpdate>;
AssetsUpdateSPtr MakeAssetsUpdate(
    const AssetsUpdateForPubSPtr& assetsUpdateForPub);

}  // namespace bq
