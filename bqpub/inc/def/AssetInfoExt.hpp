#pragma once

#include "SHMHeader.hpp"
#include "def/BQDef.hpp"
#include "util/Pch.hpp"

namespace bq::db::assetInfo {
struct FieldGroupOfAll;
using Record = FieldGroupOfAll;
using RecordSPtr = std::shared_ptr<Record>;
using RecordWPtr = std::weak_ptr<Record>;
}  // namespace bq::db::assetInfo

namespace bq {

struct AssetInfo;
using AssetInfoSPtr = std::shared_ptr<AssetInfo>;

AssetInfoSPtr MakeAssetInfo(const db::assetInfo::RecordSPtr& recAssetInfo);

using AssetInfoGroup = absl::node_hash_map<std::uint64_t, AssetInfoSPtr>;
using AssetInfoGroupSPtr = std::shared_ptr<AssetInfoGroup>;

enum class AssetChgType { Add = 1, Del = 2, Chg = 3 };

struct AssetInfoNotify {
  SHMHeader shmHeader_{MSG_ID_SYNC_ASSETS};
  AcctId acctId_{0};
  std::uint16_t addNum_{0};
  std::uint16_t delNum_{0};
  std::uint16_t chgNum_{0};
  std::uint32_t extDataLen_{0};
  char extData_[0];
};
using AssetInfoNotifySPtr = std::shared_ptr<AssetInfoNotify>;

struct UpdateInfoOfAssetGroup {
  UpdateInfoOfAssetGroup();
  bool empty();
  void print();
  std::shared_ptr<std::vector<AssetInfoSPtr>> assetInfoGroupAdd_;
  std::shared_ptr<std::vector<AssetInfoSPtr>> assetInfoGroupDel_;
  std::shared_ptr<std::vector<AssetInfoSPtr>> assetInfoGroupChg_;
};
using UpdateInfoOfAssetGroupSPtr = std::shared_ptr<UpdateInfoOfAssetGroup>;
UpdateInfoOfAssetGroupSPtr GetUpdateInfoOfAssetGroup(
    const AssetInfoNotifySPtr& assetInfoNotify);

}  // namespace bq
