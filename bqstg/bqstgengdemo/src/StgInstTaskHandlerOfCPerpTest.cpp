#include "StgInstTaskHandlerOfCPerpTest.hpp"

#include "StgEng.hpp"
#include "util/Literal.hpp"
#include "util/Logger.hpp"

namespace bq::stg {

void StgInstTaskHandlerOfCPerpTest::onStgStart() {
  getStgEng()->installStgInstTimer(1, "stgInst1Timer", ExecAtStartup::True,
                                   MilliSecInterval(1000), ExecTimes(100));
}

void StgInstTaskHandlerOfCPerpTest::onStgInstStart(
    const StgInstInfoSPtr& stgInstInfo) {
  getStgEng()->sub(stgInstInfo->stgInstId_,
                   "MD@Binance@CPerp@DOT-USD-CPerp@Trades");
  getStgEng()->sub(stgInstInfo->stgInstId_,
                   "MD@Binance@CPerp@DOT-USD-CPerp@Tickers");
  getStgEng()->sub(stgInstInfo->stgInstId_,
                   "MD@Binance@CPerp@DOT-USD-CPerp@Books@400");
  getStgEng()->sub(stgInstInfo->stgInstId_,
                   "MD@Binance@CPerp@DOT-USD-CPerp@Candle");

  getStgEng()->sub(stgInstInfo->stgInstId_,
                   "shm://RISK.PubChannel.Trade/AssetInfo/AcctId/10015");
  getStgEng()->sub(stgInstInfo->stgInstId_,
                   "RISK@PubChannel@Trade@PosInfo@StgId@10000");
}

void StgInstTaskHandlerOfCPerpTest::onStgInstAdd(
    const StgInstInfoSPtr& stgInstInfo) {}
void StgInstTaskHandlerOfCPerpTest::onStgInstDel(
    const StgInstInfoSPtr& stgInstInfo) {}
void StgInstTaskHandlerOfCPerpTest::onStgInstChg(
    const StgInstInfoSPtr& stgInstInfo) {}

void StgInstTaskHandlerOfCPerpTest::onStgInstTimer(
    const StgInstInfoSPtr& stgInstInfo) {
  static bool alreadyOrder = false;
  if (StgInstIdOfTriggerSignal(stgInstInfo) == 1 && alreadyOrder == false) {
    const auto symbolCode = "DOT-USD-CPerp";
    Decimal price = 0.0;
    {
      std::lock_guard<std::ext::spin_mutex> guard(mtxSymbol2Price_);
      const auto iter = symbol2Price_.find(symbolCode);
      if (iter == std::end(symbol2Price_)) {
        return;
      } else {
        price = iter->second;
      }
    }
    auto side = Side::Ask;  //
    if (side == Side::Bid) {
      price *= 1.01;
    } else {
      price *= 0.99;
    }
    price = static_cast<int>(price * 100) / 100.0;
    getStgEng()->order(stgInstInfo, 10015, symbolCode, side, PosSide::Both,
                       price, 2);
    alreadyOrder = true;
  }
}

void StgInstTaskHandlerOfCPerpTest::onTrades(const StgInstInfoSPtr& stgInstInfo,
                                             const TradesSPtr& trades) {
#ifdef PERF_TEST
  EXEC_PERF_TEST("Trades", trades->mdHeader_.localTs_, 10000, 100);
  return;
#endif
  LOG_D("{}: {}", stgInstInfo->stgInstId_, trades->toStr());
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxSymbol2Price_);
    symbol2Price_[trades->mdHeader_.symbolCode_] = trades->price_;
  }
}

void StgInstTaskHandlerOfCPerpTest::onBooks(const StgInstInfoSPtr& stgInstInfo,
                                            const BooksSPtr& books) {
#ifdef PERF_TEST
  EXEC_PERF_TEST("Books", books->mdHeader_.localTs_, 10000, 100);
  return;
#endif
  LOG_D("{}: {}", stgInstInfo->stgInstId_, books->toStr());
}

void StgInstTaskHandlerOfCPerpTest::onCandle(const StgInstInfoSPtr& stgInstInfo,
                                             const CandleSPtr& candle) {
#ifdef PERF_TEST
  EXEC_PERF_TEST("Candle", candle->mdHeader_.localTs_, 1000, 10);
  return;
#endif
  LOG_D("{}: {}", stgInstInfo->stgInstId_, candle->toStr());
}

void StgInstTaskHandlerOfCPerpTest::onTickers(
    const StgInstInfoSPtr& stgInstInfo, const TickersSPtr& tickers) {
#ifdef PERF_TEST
  EXEC_PERF_TEST("Tickers", tickers->mdHeader_.localTs_, 1000, 10);
  return;
#endif
  LOG_D("{}: {}", stgInstInfo->stgInstId_, tickers->toStr());
}

void StgInstTaskHandlerOfCPerpTest::onOrderRet(
    const StgInstInfoSPtr& stgInstInfo, const OrderInfoSPtr& orderInfo) {
  if (orderInfo->orderStatus_ == OrderStatus::ConfirmedByExch) {
    getStgEng()->cancelOrder(orderInfo->orderId_);
  }
}

void StgInstTaskHandlerOfCPerpTest::onCancelOrderRet(
    const StgInstInfoSPtr& stgInstInfo, const OrderInfoSPtr& orderInfo) {}

void StgInstTaskHandlerOfCPerpTest::onPosSnapshotOfAcctId(
    const StgInstInfoSPtr& stgInstInfo, const PosSnapshotSPtr& posSnapshot) {
  for (const auto& rec : posSnapshot->getPosInfoDetail()) {
    LOG_D("StgInstId: {} {} {}", stgInstInfo->stgInstId_, __func__,
          rec.second->toStr());
  }
}

void StgInstTaskHandlerOfCPerpTest::onPosUpdateOfAcctId(
    const StgInstInfoSPtr& stgInstInfo, const PosSnapshotSPtr& posUpdate) {
  for (const auto& rec : posUpdate->getPosInfoDetail()) {
    LOG_I("StgInstId: {} {} {}", stgInstInfo->stgInstId_, __func__,
          rec.second->toStr());
  }
}

void StgInstTaskHandlerOfCPerpTest::onPosSnapshotOfStgId(
    const StgInstInfoSPtr& stgInstInfo, const PosSnapshotSPtr& posSnapshot) {
  for (const auto& rec : posSnapshot->getPosInfoDetail()) {
    LOG_D("StgInstId: {} {} {}", stgInstInfo->stgInstId_, __func__,
          rec.second->toStr());
  }
}

void StgInstTaskHandlerOfCPerpTest::onPosUpdateOfStgId(
    const StgInstInfoSPtr& stgInstInfo, const PosSnapshotSPtr& posUpdate) {
  for (const auto& rec : posUpdate->getPosInfoDetail()) {
    LOG_I("StgInstId: {} {} {}", stgInstInfo->stgInstId_, __func__,
          rec.second->toStr());
  }
}

void StgInstTaskHandlerOfCPerpTest::onPosSnapshotOfStgInstId(
    const StgInstInfoSPtr& stgInstInfo, const PosSnapshotSPtr& posSnapshot) {
  for (const auto& rec : posSnapshot->getPosInfoDetail()) {
    LOG_D("StgInstId: {} {} {}", stgInstInfo->stgInstId_, __func__,
          rec.second->toStr());
  }
}

void StgInstTaskHandlerOfCPerpTest::onPosUpdateOfStgInstId(
    const StgInstInfoSPtr& stgInstInfo, const PosSnapshotSPtr& posUpdate) {
  for (const auto& rec : posUpdate->getPosInfoDetail()) {
    LOG_I("StgInstId: {} {} {}", stgInstInfo->stgInstId_, __func__,
          rec.second->toStr());
  }
}

void StgInstTaskHandlerOfCPerpTest::onAssetsSnapshot(
    const StgInstInfoSPtr& stgInstInfo,
    const AssetsSnapshotSPtr& assetsSnapshot) {
  for (const auto& rec : *assetsSnapshot) {
    LOG_D("StgInstId: {} {} {}", stgInstInfo->stgInstId_, __func__,
          rec.second->toStr());
  }
}

void StgInstTaskHandlerOfCPerpTest::onAssetsUpdate(
    const StgInstInfoSPtr& stgInstInfo, const AssetsUpdateSPtr& assetsUpdate) {
  for (const auto& rec : *assetsUpdate) {
    LOG_I("StgInstId: {} {} {}", stgInstInfo->stgInstId_, __func__,
          rec.second->toStr());
  }
}

void StgInstTaskHandlerOfCPerpTest::onOtherStgInstTask(
    const StgInstInfoSPtr& stgInstInfo, const SHMIPCAsyncTaskSPtr& asyncTask) {}

}  // namespace bq::stg
