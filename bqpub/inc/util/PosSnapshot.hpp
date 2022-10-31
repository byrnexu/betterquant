/*!
 * \file PosSnapshot.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "def/DefIF.hpp"
#include "def/PnlIF.hpp"
#include "def/PosInfoIF.hpp"

namespace bq {

struct Pnl;
using PnlSPtr = std::shared_ptr<Pnl>;

class PosSnapshotImpl;
using PosSnapshotImplSPtr = std::shared_ptr<PosSnapshotImpl>;

class PosSnapshot;
using PosSnapshotSPtr = std::shared_ptr<PosSnapshot>;

class PosSnapshot {
 public:
  PosSnapshot(const PosSnapshot&) = delete;
  PosSnapshot& operator=(const PosSnapshot&) = delete;
  PosSnapshot(const PosSnapshot&&) = delete;
  PosSnapshot& operator=(const PosSnapshot&&) = delete;

  explicit PosSnapshot(const std::map<std::string, PosInfoSPtr>& posInfoDetail,
                       const MarketDataCacheSPtr& marketDataCache);

 public:
  const std::map<std::string, PosInfoSPtr>& getPosInfoDetail() const;

  std::tuple<int, PnlSPtr> queryPnl(
      const std::string& queryCond, const std::string& quoteCurrencyForCalc,
      const std::string& quoteCurrencyForConv = "USDT",
      const std::string& origQuoteCurrencyOfUBasedContract = "USDT");

  std::tuple<int, Key2PnlGroupSPtr> queryPnlGroupBy(
      const std::string& groupCond, const std::string& quoteCurrencyForCalc,
      const std::string& quoteCurrencyForConv = "USDT",
      const std::string& origQuoteCurrencyOfUBasedContract = "USDT");

 public:
  std::tuple<int, PosInfoGroupSPtr> queryPosInfoGroup(
      const std::string& queryCond);

  std::tuple<int, Key2PosInfoBundleSPtr> queryPosInfoGroupBy(
      const std::string& groupCond);

 private:
  PosSnapshotImplSPtr posSnapshotImpl_{nullptr};
};

}  // namespace bq
