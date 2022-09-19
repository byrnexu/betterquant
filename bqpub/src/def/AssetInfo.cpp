#include "db/TBLAssetInfo.hpp"
#include "def/DataStruOfAssets.hpp"
#include "def/Def.hpp"
#include "util/Datetime.hpp"
#include "util/Json.hpp"
#include "util/Logger.hpp"

namespace bq {

std::string AssetInfo::toStr() const {
  std::string ret;
  ret = fmt::format(
      "{} vol={}; crossVol={}; frozen={}; "
      "available={}; pnlUnreal={}; maxWithdraw={}; updateTime={}",
      getKey(), vol_, crossVol_, frozen_, available_, pnlUnreal_, maxWithdraw_,
      ConvertTsToPtime(updateTime_));
  return ret;
}

std::string AssetInfo::getKey() const {
  const auto ret =
      fmt::format("{}/{}/{}/{}", acctId_, GetMarketName(marketCode_),
                  magic_enum::enum_name(symbolType_), assetName_);
  return ret;
}

void AssetInfo::initKeyHash() const {
  if (keyHash_ == 0) {
    const auto key = getKey();
    keyHash_ = XXH3_64bits(key.data(), key.size());
  }
}

bool AssetInfo::isEqual(const AssetInfoSPtr& assetInfo) {
  if (vol_ != assetInfo->vol_) return false;
  if (crossVol_ != assetInfo->crossVol_) return false;
  if (frozen_ != assetInfo->frozen_) return false;
  if (available_ != assetInfo->available_) return false;
  if (pnlUnreal_ != assetInfo->pnlUnreal_) return false;
  if (maxWithdraw_ != assetInfo->maxWithdraw_) return false;
  return true;
}

// clang-format off
std::string AssetInfo::getSqlOfInsert() const {
  const auto sql = fmt::format(
  "INSERT INTO `BetterQuant`.`assetInfo` ("
    "`acctId`,"
    "`marketCode`,"
    "`symbolType`,"
    "`assetName`,"
    "`vol`,"
    "`crossVol`,"
    "`frozen`,"
    "`available`,"
    "`pnlUnreal`,"
    "`maxWithdraw`,"
    "`updateTime`"
  ")"
  "VALUES"
  "("
    " {} ,"  // acctId
    "'{}',"  // marketCode
    "'{}',"  // symbolType
    "'{}',"  // assetName
    "'{}',"  // vol
    "'{}',"  // crossVol
    "'{}',"  // frozen
    "'{}',"  // available
    "'{}',"  // pnlUnreal
    "'{}',"  // maxWithdraw
    "'{}' "  // updateTime
  "); ",
    acctId_,
    GetMarketName(marketCode_),
    magic_enum::enum_name(symbolType_),
    assetName_,
    vol_,
    crossVol_,
    frozen_,
    available_,
    pnlUnreal_,
    maxWithdraw_,
    ConvertTsToDBTime(updateTime_)
  );
  return sql;
} ;

std::string AssetInfo::getSqlOfUpdate() const {
  const auto sql = fmt::format(
  "UPDATE `BetterQuant`.`assetInfo` SET "
    "`vol`         = {}, "
    "`crossVol`    = {}, "
    "`frozen`      = {}, "
    "`available`   = {}, "
    "`pnlUnreal`   = {}, "
    "`maxWithdraw` = {}, "
    "`updateTime`  ='{}' "
  "WHERE `acctId`     = {} "
    "AND `marketCode` ='{}' "
    "AND `symbolType` ='{}' "
    "AND `assetName`  ='{}';",
    vol_,
    crossVol_,
    frozen_,
    available_,
    pnlUnreal_,
    maxWithdraw_,
    ConvertTsToDBTime(updateTime_),
    acctId_,
    GetMarketName(marketCode_),
    magic_enum::enum_name(symbolType_),
    assetName_
  );
  return sql;
}

std::string AssetInfo::getSqlOfDelete() const{
  const auto sql = fmt::format(
    "DELETE FROM `BetterQuant`.`assetInfo` "
    "WHERE `acctId`     = {}  "
    "  AND `marketCode` ='{}' "
    "  AND `symbolType` ='{}' "
    "  AND `assetName`  ='{}';",
    acctId_,
    GetMarketName(marketCode_),
    magic_enum::enum_name(symbolType_),
    assetName_
  );
  return sql;
} ;
// clang-format on

AssetInfoSPtr MakeAssetInfo(const db::assetInfo::RecordSPtr& recAssetInfo) {
  auto assetInfo = std::make_shared<AssetInfo>();
  assetInfo->acctId_ = recAssetInfo->acctId;
  assetInfo->marketCode_ =
      magic_enum::enum_cast<MarketCode>(recAssetInfo->marketCode).value();
  assetInfo->symbolType_ =
      magic_enum::enum_cast<SymbolType>(recAssetInfo->symbolType).value();
  strncpy(assetInfo->assetName_, recAssetInfo->assetName.c_str(),
          sizeof(assetInfo->assetName_) - 1);
  assetInfo->vol_ = CONV(Decimal, recAssetInfo->vol);
  assetInfo->crossVol_ = CONV(Decimal, recAssetInfo->crossVol);
  assetInfo->frozen_ = CONV(Decimal, recAssetInfo->frozen);
  assetInfo->available_ = CONV(Decimal, recAssetInfo->available);
  assetInfo->pnlUnreal_ = CONV(Decimal, recAssetInfo->pnlUnreal);
  assetInfo->maxWithdraw_ = CONV(Decimal, recAssetInfo->maxWithdraw);
  assetInfo->updateTime_ = ConvertDBTimeToTS(recAssetInfo->updateTime);
  assetInfo->initKeyHash();
  return assetInfo;
}

UpdateInfoOfAssetGroup::UpdateInfoOfAssetGroup() {
  assetInfoGroupAdd_ = std::make_shared<std::vector<AssetInfoSPtr>>();
  assetInfoGroupDel_ = std::make_shared<std::vector<AssetInfoSPtr>>();
  assetInfoGroupChg_ = std::make_shared<std::vector<AssetInfoSPtr>>();
}

bool UpdateInfoOfAssetGroup::empty() {
  return assetInfoGroupAdd_->empty() && assetInfoGroupDel_->empty() &&
         assetInfoGroupChg_->empty();
}

void UpdateInfoOfAssetGroup::print() {
  for (const auto& assetInfo : *assetInfoGroupAdd_) {
    LOG_I("Add asset {}", assetInfo->toStr());
  }
  for (const auto& assetInfo : *assetInfoGroupDel_) {
    LOG_I("Del asset {}", assetInfo->toStr());
  }
  for (const auto& assetInfo : *assetInfoGroupChg_) {
    LOG_I("Chg asset {}", assetInfo->toStr());
  }
}

UpdateInfoOfAssetGroupSPtr GetUpdateInfoOfAssetGroup(
    const AssetInfoNotifySPtr& assetInfoNotify) {
  UpdateInfoOfAssetGroupSPtr ret = std::make_shared<UpdateInfoOfAssetGroup>();

  char* pos =
      reinterpret_cast<char*>(assetInfoNotify.get()) + sizeof(AssetInfoNotify);

  for (auto i = 0; i < assetInfoNotify->addNum_; ++i) {
    void* rawData = malloc(sizeof(AssetInfo));
    memcpy(rawData, pos, sizeof(AssetInfo));
    std::shared_ptr<AssetInfo> assetInfo(static_cast<AssetInfo*>(rawData));
    ret->assetInfoGroupAdd_->emplace_back(assetInfo);
    pos += sizeof(AssetInfo);
  }

  for (auto i = 0; i < assetInfoNotify->delNum_; ++i) {
    void* rawData = malloc(sizeof(AssetInfo));
    memcpy(rawData, pos, sizeof(AssetInfo));
    std::shared_ptr<AssetInfo> assetInfo(static_cast<AssetInfo*>(rawData));
    ret->assetInfoGroupDel_->emplace_back(assetInfo);
    pos += sizeof(AssetInfo);
  }

  for (auto i = 0; i < assetInfoNotify->chgNum_; ++i) {
    void* rawData = malloc(sizeof(AssetInfo));
    memcpy(rawData, pos, sizeof(AssetInfo));
    std::shared_ptr<AssetInfo> assetInfo(static_cast<AssetInfo*>(rawData));
    ret->assetInfoGroupChg_->emplace_back(assetInfo);
    pos += sizeof(AssetInfo);
  }

  return ret;
}

bool isEqual(const Key2AssetInfoGroupSPtr& lhs,
             const Key2AssetInfoGroupSPtr& rhs) {
  if (lhs->size() != rhs->size()) {
    return false;
  }
  auto lhsIter = std::begin(*lhs);
  auto rhsIter = std::begin(*rhs);
  while (true) {
    if (lhsIter == std::end(*lhs)) break;
    if (lhsIter->first != rhsIter->first) return false;
    ++lhsIter;
    ++rhsIter;
  }

  for (const auto& lhsRec : *lhs) {
    const auto& lhsAssetInfo = lhsRec.second;
    const auto& rhsAssetInfo = (*rhs)[lhsRec.first];
    if (lhsAssetInfo->isEqual(rhsAssetInfo) == false) {
      return false;
    }
  }

  return true;
}

AssetsUpdateSPtr MakeAssetsUpdate(
    const AssetsUpdateForPubSPtr& assetsUpdateForPub) {
  AssetsUpdateSPtr ret = std::make_shared<AssetsUpdate>();
  auto assetInfoAddr = assetsUpdateForPub->assetInfoGroup_;
  for (std::uint16_t i = 0; i < assetsUpdateForPub->num_; ++i) {
    const auto assetInfoRecv =
        reinterpret_cast<const AssetInfo*>(assetInfoAddr);
    const auto assetInfo = std::make_shared<AssetInfo>(*assetInfoRecv);
    ret->emplace(assetInfo->getKey(), assetInfo);
    assetInfoAddr += sizeof(AssetInfo);
  }
  return ret;
}

}  // namespace bq
