/*!
 * \file StgInstTaskHandlerBase.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "BQPub.hpp"
#include "Pub.hpp"
#include "SHMIPCPub.hpp"

namespace bq::stg {

class StgEng;

class StgInstTaskHandlerBase {
  friend class StgEng;

 public:
  StgInstTaskHandlerBase(const StgInstTaskHandlerBase&) = delete;
  StgInstTaskHandlerBase& operator=(const StgInstTaskHandlerBase&) = delete;
  StgInstTaskHandlerBase(const StgInstTaskHandlerBase&&) = delete;
  StgInstTaskHandlerBase& operator=(const StgInstTaskHandlerBase&&) = delete;

  explicit StgInstTaskHandlerBase(StgEng* stgEng);

 public:
  StgEng* getStgEng() const { return stgEng_; }

 private:
  virtual void onOrderRet(const StgInstInfoSPtr& stgInstInfo,
                          const OrderInfoSPtr& orderInfo) {}

  virtual void onCancelOrderRet(const StgInstInfoSPtr& stgInstInfo,
                                const OrderInfoSPtr& orderInfo) {}

 private:
  virtual void onTrades(const StgInstInfoSPtr& stgInstInfo,
                        const TradesSPtr& trades) {}

  virtual void onBooks(const StgInstInfoSPtr& stgInstInfo,
                       const BooksSPtr& books) {}

  virtual void onCandle(const StgInstInfoSPtr& stgInstInfo,
                        const CandleSPtr& candle) {}

  virtual void onTickers(const StgInstInfoSPtr& stgInstInfo,
                         const TickersSPtr& tickers) {}

 private:
  virtual void onStgStart() {}
  virtual void onStgInstStart(const StgInstInfoSPtr& stgInstInfo) {}

 private:
  virtual void onStgInstAdd(const StgInstInfoSPtr& stgInstInfo) {}
  virtual void onStgInstDel(const StgInstInfoSPtr& stgInstInfo) {}
  virtual void onStgInstChg(const StgInstInfoSPtr& stgInstInfo) {}
  virtual void onStgInstTimer(const StgInstInfoSPtr& stgInstInfo) {}

 private:
  virtual void onPosUpdateOfAcctId(const StgInstInfoSPtr& stgInstInfo,
                                   const PosSnapshotSPtr& posSnapshot) {}

  virtual void onPosSnapshotOfAcctId(const StgInstInfoSPtr& stgInstInfo,
                                     const PosSnapshotSPtr& posSnapshot) {}

  virtual void onPosUpdateOfStgId(const StgInstInfoSPtr& stgInstInfo,
                                  const PosSnapshotSPtr& posSnapshot) {}

  virtual void onPosSnapshotOfStgId(const StgInstInfoSPtr& stgInstInfo,
                                    const PosSnapshotSPtr& posSnapshot) {}

  virtual void onPosUpdateOfStgInstId(const StgInstInfoSPtr& stgInstInfo,
                                      const PosSnapshotSPtr& posSnapshot) {}

  virtual void onPosSnapshotOfStgInstId(const StgInstInfoSPtr& stgInstInfo,
                                        const PosSnapshotSPtr& posSnapshot) {}

  virtual void onAssetsUpdate(const StgInstInfoSPtr& stgInstInfo,
                              const AssetsUpdateSPtr& assetsUpdate) {}

  virtual void onAssetsSnapshot(const StgInstInfoSPtr& stgInstInfo,
                                const AssetsSnapshotSPtr& assetsSnapshot) {}

 private:
  virtual void onOtherStgInstTask(const StgInstInfoSPtr& stgInstInfo,
                                  const SHMIPCAsyncTaskSPtr& asyncTask) {}

 private:
  StgEng* stgEng_;
};

}  // namespace bq::stg
