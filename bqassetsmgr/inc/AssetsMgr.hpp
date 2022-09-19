#pragma once

#include "def/BQConst.hpp"
#include "def/Const.hpp"
#include "def/DataStruOfAssets.hpp"
#include "util/Pch.hpp"
#include "util/StdExt.hpp"

namespace bq::db {
class DBEng;
using DBEngSPtr = std::shared_ptr<DBEng>;
}  // namespace bq::db

namespace bq {

class AssetsMgr {
 public:
  AssetsMgr(const AssetsMgr&) = delete;
  AssetsMgr& operator=(const AssetsMgr&) = delete;
  AssetsMgr(const AssetsMgr&&) = delete;
  AssetsMgr& operator=(const AssetsMgr&&) = delete;

  AssetsMgr();

 public:
  int init(const YAML::Node& node, const db::DBEngSPtr& dbEng,
           const std::string& sql);

 private:
  int initAssetInfoGroup(const std::string& sql);

 public:
  UpdateInfoOfAssetGroupSPtr compareWithAssetsSnapshot(
      const AssetInfoGroupSPtr& assetInfoGroupFromExch);

 private:
  IsTheAssetInfoUpdated updateByAssetInfoFromExch(
      AssetInfoSPtr& assetInfoInAssetsMgr,
      const AssetInfoSPtr& assetInfoFromExch);

 public:
  AssetChgType compareWithAssetsUpdate(const AssetInfoSPtr& assetInfo);

 public:
  void add(const AssetInfoSPtr& assetInfo, LockFunc lockFunc = LockFunc::True);
  void remove(const AssetInfoSPtr& assetInfo,
              LockFunc lockFunc = LockFunc::True);
  void update(const AssetInfoSPtr& assetInfo,
              LockFunc lockFunc = LockFunc::True);

 public:
  std::vector<AssetInfoSPtr> getAssetInfoGroup(LockFunc lockFunc) const;

 public:
  std::string toStr(LockFunc lockFunc = LockFunc::True) const;

 public:
  void cacheUpdateInfoOfAssetGroupOfSyncToDB(
      const UpdateInfoOfAssetGroupSPtr& updateInfoOfAssetGroup);
  int syncUpdateInfoOfAssetGroupToDB();

 private:
  YAML::Node node_;
  db::DBEngSPtr dbEng_{nullptr};

  AssetInfoGroupSPtr assetInfoGroup_{nullptr};
  mutable std::ext::spin_mutex mtxAssetInfoGroup_;

  std::vector<UpdateInfoOfAssetGroupSPtr> updateInfoOfAssetGroupOfSyncToDB_;
  mutable std::ext::spin_mutex mtxUpdateInfoOfAssetGroupOfSyncToDB_;
};

}  // namespace bq
