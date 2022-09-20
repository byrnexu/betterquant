/*!
 * \file PosSnapshot.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "util/PosSnapshot.hpp"

#include "def/Pnl.hpp"
#include "util/PosSnapshotImpl.hpp"

namespace bq {

PosSnapshot::PosSnapshot(const std::map<std::string, PosInfoSPtr>& posInfoGroup)
    : posSnapshotImpl_(std::make_shared<PosSnapshotImpl>(posInfoGroup)) {}

const std::map<std::string, PosInfoSPtr>& PosSnapshot::getPosInfoDetail()
    const {
  return posSnapshotImpl_->getPosInfoDetail();
}

std::tuple<int, PnlSPtr> PosSnapshot::queryPnl(
    const std::string& groupCond, const MarketDataCacheSPtr& marketDataCache,
    const std::string& quoteCurrencyForCalc,
    const std::string& quoteCurrencyForConv,
    const std::string& origQuoteCurrencyOfUBasedContract) {
  return posSnapshotImpl_->queryPnl(groupCond, marketDataCache,
                                    quoteCurrencyForCalc, quoteCurrencyForConv,
                                    origQuoteCurrencyOfUBasedContract);
}

std::tuple<int, Key2PnlGroupSPtr> PosSnapshot::queryPnlGroupBy(
    const std::string& groupCond, const MarketDataCacheSPtr& marketDataCache,
    const std::string& quoteCurrencyForCalc,
    const std::string& quoteCurrencyForConv,
    const std::string& origQuoteCurrencyOfUBasedContract) {
  return posSnapshotImpl_->queryPnlGroupBy(
      groupCond, marketDataCache, quoteCurrencyForCalc, quoteCurrencyForConv,
      origQuoteCurrencyOfUBasedContract);
}

std::tuple<int, PosInfoGroupSPtr> PosSnapshot::queryPosInfoGroup(
    const std::string& queryCond) {
  return posSnapshotImpl_->queryPosInfoGroup(queryCond);
}

std::tuple<int, Key2PosInfoBundleSPtr> PosSnapshot::queryPosInfoGroupBy(
    const std::string& groupCond) {
  return posSnapshotImpl_->queryPosInfoGroupBy(groupCond);
}

}  // namespace bq
