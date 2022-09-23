/*!
 * \file AssetsMgr.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "AssetsMgr.hpp"

#include "db/DBEng.hpp"
#include "db/TBLAssetInfo.hpp"
#include "db/TBLRecSetMaker.hpp"
#include "def/Def.hpp"
#include "def/StatusCode.hpp"
#include "util/Float.hpp"
#include "util/Json.hpp"
#include "util/Logger.hpp"

namespace bq {

AssetsMgr::AssetsMgr() : assetInfoGroup_(std::make_shared<AssetInfoGroup>()) {}

int AssetsMgr::init(const YAML::Node& node, const db::DBEngSPtr& dbEng,
                    const std::string& sql) {
  node_ = node;
  dbEng_ = dbEng;

  const auto ret = initAssetInfoGroup(sql);
  if (ret != 0) {
    LOG_W("Init failed. [{}]", sql);
    return ret;
  }
  return 0;
}

int AssetsMgr::initAssetInfoGroup(const std::string& sql) {
  const auto [ret, tblRecSet] =
      db::TBLRecSetMaker<TBLAssetInfo>::ExecSql(dbEng_, sql);
  if (ret != 0) {
    LOG_W("Init asset info group failed. {}", sql);
    return ret;
  }

  for (const auto& tblRec : *tblRecSet) {
    const auto recAssetInfo = tblRec.second->getRecWithAllFields();
    const auto assetInfo = MakeAssetInfo(recAssetInfo);
    assetInfoGroup_->emplace(assetInfo->keyHash_, assetInfo);
  }
  LOG_I("Init asset info group success. [size = {}]", assetInfoGroup_->size());
  return 0;
}

UpdateInfoOfAssetGroupSPtr AssetsMgr::compareWithAssetsSnapshot(
    const AssetInfoGroupSPtr& assetInfoGroupFromExch) {
  auto ret = std::make_shared<UpdateInfoOfAssetGroup>();

  for (const auto& rec : *assetInfoGroupFromExch) {
    const auto iter = assetInfoGroup_->find(rec.first);
    if (iter == std::end(*assetInfoGroup_)) {
      assetInfoGroup_->emplace(rec);
      ret->assetInfoGroupAdd_->emplace_back(
          std::make_shared<AssetInfo>(*rec.second));
      LOG_I("Find new asset. {}", rec.second->toStr());
    } else {
      auto& assetInfoInAssetsMgr = iter->second;
      const auto& assetInfoGroupFromExch = rec.second;
      const auto isTheAssetInfoUpdated = updateByAssetInfoFromExch(
          assetInfoInAssetsMgr, assetInfoGroupFromExch);
      if (isTheAssetInfoUpdated == IsTheAssetInfoUpdated::True) {
        ret->assetInfoGroupChg_->emplace_back(
            std::make_shared<AssetInfo>(*assetInfoInAssetsMgr));
        LOG_D("Find asset changed. {}", assetInfoInAssetsMgr->toStr());
      }
    }
  }

  for (const auto& rec : *assetInfoGroup_) {
    const auto iter = assetInfoGroupFromExch->find(rec.first);
    if (iter == std::end(*assetInfoGroupFromExch)) {
      auto& assetInfoInAssetsMgr = rec.second;
      ret->assetInfoGroupDel_->emplace_back(
          std::make_shared<AssetInfo>(*assetInfoInAssetsMgr));
      LOG_I("Find vol of asset changed to 0. {}",
            assetInfoInAssetsMgr->toStr());
    }
  }

  for (const auto& assetInfo : *ret->assetInfoGroupDel_) {
    assetInfoGroup_->erase(assetInfo->keyHash_);
  }

  return ret;
}

IsTheAssetInfoUpdated AssetsMgr::updateByAssetInfoFromExch(
    AssetInfoSPtr& assetInfoInAssetsMgr,
    const AssetInfoSPtr& assetInfoFromExch) {
  IsTheAssetInfoUpdated isTheAssetInfoUpdated = IsTheAssetInfoUpdated::False;

  if (!isApproximatelyEqual(assetInfoFromExch->vol_,
                            assetInfoInAssetsMgr->vol_)) {
    assetInfoInAssetsMgr->vol_ = assetInfoFromExch->vol_;
    isTheAssetInfoUpdated = IsTheAssetInfoUpdated::True;
  }

  if (!isApproximatelyEqual(assetInfoFromExch->crossVol_,
                            assetInfoInAssetsMgr->crossVol_)) {
    assetInfoInAssetsMgr->crossVol_ = assetInfoFromExch->crossVol_;
    isTheAssetInfoUpdated = IsTheAssetInfoUpdated::True;
  }

  if (!isApproximatelyEqual(assetInfoFromExch->frozen_,
                            assetInfoInAssetsMgr->frozen_)) {
    assetInfoInAssetsMgr->frozen_ = assetInfoFromExch->frozen_;
    isTheAssetInfoUpdated = IsTheAssetInfoUpdated::True;
  }

  if (!isApproximatelyEqual(assetInfoFromExch->available_,
                            assetInfoInAssetsMgr->available_)) {
    assetInfoInAssetsMgr->available_ = assetInfoFromExch->available_;
    isTheAssetInfoUpdated = IsTheAssetInfoUpdated::True;
  }

  if (!isApproximatelyEqual(assetInfoFromExch->pnlUnreal_,
                            assetInfoInAssetsMgr->pnlUnreal_)) {
    assetInfoInAssetsMgr->pnlUnreal_ = assetInfoFromExch->pnlUnreal_;
    isTheAssetInfoUpdated = IsTheAssetInfoUpdated::True;
  }

  if (!isApproximatelyEqual(assetInfoFromExch->maxWithdraw_,
                            assetInfoInAssetsMgr->maxWithdraw_)) {
    assetInfoInAssetsMgr->maxWithdraw_ = assetInfoFromExch->maxWithdraw_;
    isTheAssetInfoUpdated = IsTheAssetInfoUpdated::True;
  }


  return isTheAssetInfoUpdated;
}

AssetChgType AssetsMgr::compareWithAssetsUpdate(
    const AssetInfoSPtr& assetInfo) {
  assetInfo->initKeyHash();
  AssetChgType assetChgType;
  if (isApproximatelyZero(assetInfo->vol_)) {
    assetInfoGroup_->erase(assetInfo->keyHash_);
    assetChgType = AssetChgType::Del;
  } else {
    const auto iter = assetInfoGroup_->find(assetInfo->keyHash_);
    if (iter == std::end(*assetInfoGroup_)) {
      assetInfoGroup_->emplace(assetInfo->keyHash_, assetInfo);
      assetChgType = AssetChgType::Add;
    } else {
      (*assetInfoGroup_)[assetInfo->keyHash_] = assetInfo;
      assetChgType = AssetChgType::Chg;
    }
  }
  return assetChgType;
}

void AssetsMgr::add(const AssetInfoSPtr& assetInfo, LockFunc lockFunc) {
  assetInfo->initKeyHash();
  {
    SPIN_LOCK(mtxAssetInfoGroup_);
    const auto iter = assetInfoGroup_->find(assetInfo->keyHash_);
    if (iter == std::end(*assetInfoGroup_)) {
      assetInfoGroup_->emplace(assetInfo->keyHash_, assetInfo);
    } else {
      LOG_W("The asset info to be added already exists. {}",
            assetInfo->toStr());
    }
  }
}

void AssetsMgr::remove(const AssetInfoSPtr& assetInfo, LockFunc lockFunc) {
  assetInfo->initKeyHash();
  {
    SPIN_LOCK(mtxAssetInfoGroup_);
    const auto iter = assetInfoGroup_->find(assetInfo->keyHash_);
    if (iter != std::end(*assetInfoGroup_)) {
      assetInfoGroup_->erase(iter);
    } else {
      LOG_W("The asset info to be deleted does not exist. {}",
            assetInfo->toStr());
    }
  }
}

void AssetsMgr::update(const AssetInfoSPtr& assetInfo, LockFunc lockFunc) {
  assetInfo->initKeyHash();
  {
    SPIN_LOCK(mtxAssetInfoGroup_);
    const auto iter = assetInfoGroup_->find(assetInfo->keyHash_);
    if (iter != std::end(*assetInfoGroup_)) {
      iter->second = assetInfo;
    } else {
      LOG_W("The asset info to be update does not exist. {}",
            assetInfo->toStr());
    }
  }
}

std::vector<AssetInfoSPtr> AssetsMgr::getAssetInfoGroup(
    LockFunc lockFunc) const {
  std::vector<AssetInfoSPtr> ret;
  {
    SPIN_LOCK(mtxAssetInfoGroup_);
    for (const auto& rec : *assetInfoGroup_) {
      ret.emplace_back(std::make_shared<AssetInfo>(*rec.second));
    }
  }
  return ret;
}

std::string AssetsMgr::toStr(LockFunc lockFunc) const {
  std::string ret;
  {
    SPIN_LOCK(mtxAssetInfoGroup_);
    for (const auto& rec : *assetInfoGroup_) {
      const auto& assetInfo = rec.second;
      ret = ret + "\n" + assetInfo->toStr();
    }
  }
  return ret;
}

void AssetsMgr::cacheUpdateInfoOfAssetGroupOfSyncToDB(
    const UpdateInfoOfAssetGroupSPtr& updateInfoOfAssetGroup) {
  {
    std::lock_guard<std::ext::spin_mutex> guard(
        mtxUpdateInfoOfAssetGroupOfSyncToDB_);
    updateInfoOfAssetGroupOfSyncToDB_.emplace_back(updateInfoOfAssetGroup);
  }
}

int AssetsMgr::syncUpdateInfoOfAssetGroupToDB() {
  std::vector<UpdateInfoOfAssetGroupSPtr> updateInfoOfAssetGroupOfSyncToDB;
  {
    std::lock_guard<std::ext::spin_mutex> guard(
        mtxUpdateInfoOfAssetGroupOfSyncToDB_);
    std::swap(updateInfoOfAssetGroupOfSyncToDB,
              updateInfoOfAssetGroupOfSyncToDB_);
  }

  for (const auto& rec : updateInfoOfAssetGroupOfSyncToDB) {
    for (const auto& assetInfo : *rec->assetInfoGroupAdd_) {
      const auto identity = GET_RAND_STR();
      const auto sql = assetInfo->getSqlOfInsert();
      const auto [ret, execRet] = dbEng_->asyncExec(identity, sql);
      if (ret != 0) {
        LOG_W("Insert asset info to db failed. [{}]", sql);
      }
    }

    for (const auto& assetInfo : *rec->assetInfoGroupDel_) {
      const auto identity = GET_RAND_STR();
      const auto sql = assetInfo->getSqlOfDelete();
      const auto [ret, execRet] = dbEng_->asyncExec(identity, sql);
      if (ret != 0) {
        LOG_W("Del asset info from db failed. [{}]", sql);
      }
    }

    for (const auto& assetInfo : *rec->assetInfoGroupChg_) {
      const auto identity = GET_RAND_STR();
      const auto sql = assetInfo->getSqlOfUpdate();
      const auto [ret, execRet] = dbEng_->asyncExec(identity, sql);
      if (ret != 0) {
        LOG_W("Update asset info from db failed. [{}]", sql);
      }
    }
  }

  return 0;
}

}  // namespace bq
