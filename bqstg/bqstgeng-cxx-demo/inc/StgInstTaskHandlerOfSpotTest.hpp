/*!
 * \file StgInstTaskHandlerOfSpotTest.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "StgInstTaskHandlerBase.hpp"
#include "util/StdExt.hpp"

namespace bq::stg {

class StgInstTaskHandlerOfSpotTest : public StgInstTaskHandlerBase {
 public:
  StgInstTaskHandlerOfSpotTest(const StgInstTaskHandlerOfSpotTest&) = delete;
  StgInstTaskHandlerOfSpotTest& operator=(const StgInstTaskHandlerOfSpotTest&) =
      delete;
  StgInstTaskHandlerOfSpotTest(const StgInstTaskHandlerOfSpotTest&&) = delete;
  StgInstTaskHandlerOfSpotTest& operator=(
      const StgInstTaskHandlerOfSpotTest&&) = delete;

  using StgInstTaskHandlerBase::StgInstTaskHandlerBase;

 private:
  void onStgStart() final;
  void onStgInstStart(const StgInstInfoSPtr& stgInstInfo) final;

 private:
  void onStgInstAdd(const StgInstInfoSPtr& stgInstInfo) final;
  void onStgInstDel(const StgInstInfoSPtr& stgInstInfo) final;
  void onStgInstChg(const StgInstInfoSPtr& stgInstInfo) final;
  void onStgInstTimer(const StgInstInfoSPtr& stgInstInfo,
                      const std::string& timerName) final;

 private:
  void onTrades(const StgInstInfoSPtr& stgInstInfo,
                const TradesSPtr& trades) final;

  void onBooks(const StgInstInfoSPtr& stgInstInfo,
               const BooksSPtr& books) final;

  void onCandle(const StgInstInfoSPtr& stgInstInfo,
                const CandleSPtr& candle) final;

  void onTickers(const StgInstInfoSPtr& stgInstInfo,
                 const TickersSPtr& tickers) final;

 private:
  void onStgManualIntervention(const StgInstInfoSPtr& stgInstInfo,
                               const CommonIPCDataSPtr& commonIPCData) final;

  void onOrderRet(const StgInstInfoSPtr& stgInstInfo,
                  const OrderInfoSPtr& orderInfo) final;

  void onCancelOrderRet(const StgInstInfoSPtr& stgInstInfo,
                        const OrderInfoSPtr& orderInfo) final;

 private:
  void onPosSnapshotOfAcctId(const StgInstInfoSPtr& stgInstInfo,
                             const PosSnapshotSPtr& posSnapshotOfAcctId) final;

  void onPosUpdateOfAcctId(const StgInstInfoSPtr& stgInstInfo,
                           const PosSnapshotSPtr& posUpdateOfAcctId) final;

  void onPosSnapshotOfStgId(const StgInstInfoSPtr& stgInstInfo,
                            const PosSnapshotSPtr& posSnapshotOfStgId) final;

  void onPosUpdateOfStgId(const StgInstInfoSPtr& stgInstInfo,
                          const PosSnapshotSPtr& posUpdateOfStgId) final;

  void onPosSnapshotOfStgInstId(
      const StgInstInfoSPtr& stgInstInfo,
      const PosSnapshotSPtr& posSnapshotOfStgInstId) final;

  void onPosUpdateOfStgInstId(
      const StgInstInfoSPtr& stgInstInfo,
      const PosSnapshotSPtr& posUpdateOfStgInstId) final;

  void onAssetsSnapshot(const StgInstInfoSPtr& stgInstInfo,
                        const AssetsSnapshotSPtr& assetsSnapshot) final;

  void onAssetsUpdate(const StgInstInfoSPtr& stgInstInfo,
                      const AssetsUpdateSPtr& assetsUpdate) final;

 private:
  void onOtherStgInstTask(const StgInstInfoSPtr& stgInstInfo,
                          const SHMIPCAsyncTaskSPtr& asyncTask) final;

 private:
  std::map<std::string, double> symbol2Price_;
  std::ext::spin_mutex mtxSymbol2Price_;
};

}  // namespace bq::stg
