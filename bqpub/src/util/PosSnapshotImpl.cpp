#include "util/PosSnapshotImpl.hpp"

#include "def/Pnl.hpp"
#include "def/PosInfo.hpp"
#include "def/StatusCode.hpp"
#include "def/SymbolInfo.hpp"
#include "util/Logger.hpp"

namespace bq {

PosSnapshotImpl::PosSnapshotImpl(
    const std::map<std::string, PosInfoSPtr>& posInfoDetail)
    : posInfoDetail_(posInfoDetail) {}

const std::map<std::string, PosInfoSPtr>& PosSnapshotImpl::getPosInfoDetail()
    const {
  return posInfoDetail_;
}

std::tuple<int, PnlSPtr> PosSnapshotImpl::queryPnl(
    const std::string& queryCond, const MarketDataCacheSPtr& marketDataCache,
    const std::string& quoteCurrencyForCalc,
    const std::string& quoteCurrencyForConv,
    const std::string& origQuoteCurrencyOfUBasedContract) {
  const auto keyGroup2Str = [](const auto& key2PnlGroup) {
    std::string ret;
    for (const auto& rec : *key2PnlGroup) {
      ret = ret + rec.first + ", ";
    }
    if (ret.size() > 1) ret = ret.substr(0, ret.length() - 2);
    return ret;
  };

  const auto groupCond = convQueryCond(queryCond);
  if (groupCond.empty()) {
    LOG_W("Query pnl by {} failed, maybe the query cond is invalid.",
          queryCond);
    return {SCODE_BQPUB_INVALID_QRY_COND, nullptr};
  }

  const auto [statusCode, key2PnlGroup] =
      queryPnlGroupBy(groupCond, marketDataCache, quoteCurrencyForCalc,
                      quoteCurrencyForConv, origQuoteCurrencyOfUBasedContract);
  if (statusCode != 0) {
    return {statusCode, nullptr};
  }

  const auto iter = key2PnlGroup->find(queryCond);
  if (iter != std::end(*key2PnlGroup)) {
    return {0, iter->second};
  }

  LOG_W("Query pnl of {} failed, all pnl is {{{}}}", queryCond,
        keyGroup2Str(key2PnlGroup));
  return {SCODE_BQPUB_PNL_NOT_EXISTS, nullptr};
}

std::tuple<int, Key2PnlGroupSPtr> PosSnapshotImpl::queryPnlGroupBy(
    const std::string& groupCond, const MarketDataCacheSPtr& marketDataCache,
    const std::string& quoteCurrencyForCalc,
    const std::string& quoteCurrencyForConv,
    const std::string& origQuoteCurrencyOfUBasedContract) {
  const auto iter = cond2Key2PnlGroup_.find(groupCond);
  if (iter != std::end(cond2Key2PnlGroup_)) {
    LOG_D("Query key2PnlGroup of {} from cache success.", groupCond);
    return {0, iter->second};
  }

  const auto [statusCode, key2PosInfoBundle] = queryPosInfoGroupBy(groupCond);
  if (statusCode != 0) {
    return {statusCode, nullptr};
  }

  auto key2PnlGroup = std::make_shared<Key2PnlGroup>();
  for (const auto& rec : *key2PosInfoBundle) {
    auto pnl = std::make_shared<Pnl>();
    pnl->queryCond_ = rec.first;
    pnl->quoteCurrencyForCalc_ = quoteCurrencyForCalc;

    const auto& posInfoGroup = rec.second;
    for (const auto& posInfo : *posInfoGroup) {
      const auto curPnl = posInfo->calcPnl(
          marketDataCache, quoteCurrencyForCalc, quoteCurrencyForConv,
          origQuoteCurrencyOfUBasedContract);
      pnl->fee_ += curPnl->fee_;
      pnl->pnlUnReal_ += curPnl->pnlUnReal_;
      pnl->pnlReal_ += curPnl->pnlReal_;

      if (curPnl->updateTime_ < pnl->updateTime_ || 0 == pnl->updateTime_) {
        pnl->updateTime_ = curPnl->updateTime_;
      }

      if (curPnl->statusCode_ != 0) {
        pnl->statusCode_ = curPnl->statusCode_;
        pnl->symbolGroupForCalc_ = std::move(curPnl->symbolGroupForCalc_);
        LOG_W(
            "Query pnl group by {} failed, "
            "maybe some of the symbolcode below is not sub. [{}] {}",
            groupCond, SymbolInfoGroup2Str(pnl->symbolGroupForCalc_),
            posInfo->toStr());
      }
    }
    const auto& keyOfCond = rec.first;
    key2PnlGroup->emplace(keyOfCond, pnl);
  }

  cond2Key2PnlGroup_[groupCond] = key2PnlGroup;
  return {0, key2PnlGroup};
}

std::tuple<int, PosInfoGroupSPtr> PosSnapshotImpl::queryPosInfoGroup(
    const std::string& queryCond) {
  const auto keyGroup2Str = [](const auto& key2PosInfoBundle) {
    std::string ret;
    for (const auto& rec : *key2PosInfoBundle) {
      ret = ret + rec.first + ", ";
    }
    if (ret.size() > 1) ret = ret.substr(0, ret.length() - 2);
    return ret;
  };

  const auto groupCond = convQueryCond(queryCond);
  if (groupCond.empty()) {
    LOG_W("Query posinfo group by {} failed, maybe the query cond is invalid.",
          groupCond);
    return {SCODE_BQPUB_INVALID_QRY_COND, nullptr};
  }

  const auto [statusCode, key2PosInfoBundle] = queryPosInfoGroupBy(groupCond);
  if (statusCode != 0) {
    return {statusCode, nullptr};
  }

  const auto iter = key2PosInfoBundle->find(queryCond);
  if (iter != std::end(*key2PosInfoBundle)) {
    return {0, iter->second};
  }

  LOG_W("Query posinfo by {} failed, all posinfo is {{{}}}", queryCond,
        keyGroup2Str(key2PosInfoBundle));
  return {SCODE_BQPUB_POS_INFO_GROUP_NOT_EXISTS, nullptr};
}

std::tuple<int, Key2PosInfoBundleSPtr> PosSnapshotImpl::queryPosInfoGroupBy(
    const std::string& groupCond) {
  const auto makeKeyOfCond =
      [this](const std::vector<std::string>& fieldNameGroupInCond,
             const std::vector<std::string>& fieldValueGroupInKey) {
        std::string ret;
        for (const auto& fieldNameInCond : fieldNameGroupInCond) {
          const auto iter = FieldName2NoInPosInfo.find(fieldNameInCond);
          if (iter == std::end(FieldName2NoInPosInfo)) return std::string("");
          const auto fieldValue = fieldValueGroupInKey[iter->second];
          ret = ret + fmt::format("{}={}{}", fieldNameInCond, fieldValue,
                                  SEP_OF_GROUP_COND);
        }
        if (!ret.empty()) ret.pop_back();
        return ret;
      };

  const auto iter = cond2Key2PosInfoBundle_.find(groupCond);
  if (iter != std::end(cond2Key2PosInfoBundle_)) {
    LOG_D("Query Key2PosInfoBundle of {} from cache success.", groupCond);
    return {0, iter->second};
  }

  std::vector<std::string> fieldNameGroupInCond;
  boost::algorithm::split(fieldNameGroupInCond, groupCond,
                          boost::is_any_of(SEP_OF_GROUP_COND));
  if (fieldNameGroupInCond.empty()) {
    LOG_W(
        "Query posinfo group by {} failed "
        "because of the query cond is invalid.",
        groupCond);
    return {SCODE_BQPUB_INVALID_QRY_COND, nullptr};
  }

  auto key2PosInfoBundle = std::make_shared<Key2PosInfoBundle>();
  for (const auto& rec : posInfoDetail_) {
    std::vector<std::string> fieldValueGroupInKey;
    const auto& key = rec.first;
    boost::algorithm::split(fieldValueGroupInKey, key, boost::is_any_of("/"));

    const auto keyOfCond =
        makeKeyOfCond(fieldNameGroupInCond, fieldValueGroupInKey);
    if (keyOfCond.empty()) {
      LOG_W(
          "Query posinfo group by {} failed, "
          "maybe the query cond is invalid.",
          groupCond);
      return {SCODE_BQPUB_INVALID_QRY_COND, nullptr};
    }

    if (key2PosInfoBundle->find(keyOfCond) == std::end(*key2PosInfoBundle)) {
      (*key2PosInfoBundle)[keyOfCond] = std::make_shared<PosInfoGroup>();
    }
    (*key2PosInfoBundle)[keyOfCond]->emplace_back(rec.second);
  }

  cond2Key2PosInfoBundle_[groupCond] = key2PosInfoBundle;
  return {0, key2PosInfoBundle};
}

std::string PosSnapshotImpl::convQueryCond(const std::string& queryCond) {
  std::vector<std::string> fieldName2ValueGroup;
  boost::split(fieldName2ValueGroup, queryCond,
               boost::is_any_of(SEP_OF_GROUP_COND));
  if (fieldName2ValueGroup.empty()) {
    return "";
  }

  std::string groupCond;
  for (const auto& rec : fieldName2ValueGroup) {
    std::vector<std::string> fieldName2Value;
    boost::split(fieldName2Value, rec, boost::is_any_of("="));
    if (fieldName2Value.size() != 2) {
      return "";
    }
    groupCond = groupCond + fieldName2Value[0] + SEP_OF_GROUP_COND;
  }
  if (!groupCond.empty()) groupCond.pop_back();

  return groupCond;
}

}  // namespace bq
