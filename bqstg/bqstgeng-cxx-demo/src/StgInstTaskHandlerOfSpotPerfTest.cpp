/*!
 * \file StgInstTaskHandlerOfSpotPerfTest.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "StgInstTaskHandlerOfSpotPerfTest.hpp"

#include "StgEng.hpp"
#include "util/Literal.hpp"
#include "util/Logger.hpp"
#include "util/PosSnapshot.hpp"

namespace bq::stg {

void StgInstTaskHandlerOfSpotPerfTest::onStgStart() {
  getStgEng()->installStgInstTimer(1, "stgInst1Timer", ExecAtStartup::True,
                                   MilliSecInterval(1000), ExecTimes(100));
}

void StgInstTaskHandlerOfSpotPerfTest::onStgInstStart(
    const StgInstInfoSPtr& stgInstInfo) {
  if (StgInstIdOfTriggerSignal(stgInstInfo) == 1) {
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://RISK.PubChannel.Trade/AssetInfo/AcctId/10005");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://RISK.PubChannel.Trade/PosInfo/StgId/10000");
    getStgEng()->sub(
        stgInstInfo->stgInstId_,
        "shm://RISK.PubChannel.Trade/PosInfo/StgId/10000/StgInstId/2");

    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/ADA-USDT/Trades");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/ADA-USDT/Tickers");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/ADA-USDT/Books/400");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/ADA-USDT/Candle");

    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/BTC-USDT/Trades");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/BTC-USDT/Tickers");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/BTC-USDT/Books/400");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/BTC-USDT/Candle");

    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/DOGE-USDT/Trades");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/DOGE-USDT/Tickers");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/DOGE-USDT/Books/400");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/DOGE-USDT/Candle");

    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/ETH-USDT/Trades");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/ETH-USDT/Tickers");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/ETH-USDT/Books/400");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/ETH-USDT/Candle");
  }

  if (StgInstIdOfTriggerSignal(stgInstInfo) == 2) {
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://RISK.PubChannel.Trade/PosInfo/AcctId/10001");
    getStgEng()->sub(
        stgInstInfo->stgInstId_,
        "shm://RISK.PubChannel.Trade/PosInfo/StgId/10000/StgInstId/1");

    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/ETH-USDT/Trades");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/ETH-USDT/Tickers");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/ETH-USDT/Books/400");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/ETH-USDT/Candle");

    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/SOL-USDT/Trades");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/SOL-USDT/Tickers");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/SOL-USDT/Books/400");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/SOL-USDT/Candle");

    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/BNB-USDT/Trades");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/BNB-USDT/Tickers");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/BNB-USDT/Books/400");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/BNB-USDT/Candle");

    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/BTC-USDT/Trades");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/BTC-USDT/Tickers");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/BTC-USDT/Books/400");
    getStgEng()->sub(stgInstInfo->stgInstId_,
                     "shm://MD.Binance.Spot/BTC-USDT/Candle");
  }
}

void StgInstTaskHandlerOfSpotPerfTest::onStgInstAdd(
    const StgInstInfoSPtr& stgInstInfo) {}
void StgInstTaskHandlerOfSpotPerfTest::onStgInstDel(
    const StgInstInfoSPtr& stgInstInfo) {}
void StgInstTaskHandlerOfSpotPerfTest::onStgInstChg(
    const StgInstInfoSPtr& stgInstInfo) {}

void StgInstTaskHandlerOfSpotPerfTest::onStgInstTimer(
    const StgInstInfoSPtr& stgInstInfo, const std::string& timerName) {
  static bool alreadyOrder = false;
  if (StgInstIdOfTriggerSignal(stgInstInfo) == 1 && alreadyOrder == false) {
    const auto symbolCode = "ADA-USDT";
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
    auto side = Side::Bid;  //
    if (side == Side::Bid) {
      price *= 1.01;
    } else {
      price *= 0.99;
    }
    price = static_cast<int>(price * 10000) / 10000.0;
    for (int i = 0; i < 100; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      getStgEng()->order(stgInstInfo, 10001, symbolCode, side, PosSide::Both,
                         price, 30.0);
    }
    alreadyOrder = true;
  }
}

void StgInstTaskHandlerOfSpotPerfTest::onTrades(
    const StgInstInfoSPtr& stgInstInfo, const TradesSPtr& trades) {
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

void StgInstTaskHandlerOfSpotPerfTest::onBooks(
    const StgInstInfoSPtr& stgInstInfo, const BooksSPtr& books) {
#ifdef PERF_TEST
  EXEC_PERF_TEST("Books", books->mdHeader_.localTs_, 10000, 100);
  return;
#endif
  LOG_D("{}: {}", stgInstInfo->stgInstId_, books->toStr());
}

void StgInstTaskHandlerOfSpotPerfTest::onCandle(
    const StgInstInfoSPtr& stgInstInfo, const CandleSPtr& candle) {
#ifdef PERF_TEST
  EXEC_PERF_TEST("Candle", candle->mdHeader_.localTs_, 1000, 10);
  return;
#endif
  LOG_D("{}: {}", stgInstInfo->stgInstId_, candle->toStr());
}

void StgInstTaskHandlerOfSpotPerfTest::onTickers(
    const StgInstInfoSPtr& stgInstInfo, const TickersSPtr& tickers) {
#ifdef PERF_TEST
  EXEC_PERF_TEST("Tickers", tickers->mdHeader_.localTs_, 1000, 10);
  return;
#endif
  LOG_D("{}: {}", stgInstInfo->stgInstId_, tickers->toStr());
}

void StgInstTaskHandlerOfSpotPerfTest::onOrderRet(
    const StgInstInfoSPtr& stgInstInfo, const OrderInfoSPtr& orderInfo) {
  if (orderInfo->orderStatus_ == OrderStatus::ConfirmedByExch) {
    getStgEng()->cancelOrder(orderInfo->orderId_);
  }
}

void StgInstTaskHandlerOfSpotPerfTest::onCancelOrderRet(
    const StgInstInfoSPtr& stgInstInfo, const OrderInfoSPtr& orderInfo) {}

void StgInstTaskHandlerOfSpotPerfTest::onPosSnapshotOfAcctId(
    const StgInstInfoSPtr& stgInstInfo, const PosSnapshotSPtr& posSnapshot) {
  for (const auto& rec : posSnapshot->getPosInfoDetail()) {
    LOG_D("StgInstId: {} {} {}", stgInstInfo->stgInstId_, __func__,
          rec.second->toStr());
  }
}

void StgInstTaskHandlerOfSpotPerfTest::onPosUpdateOfAcctId(
    const StgInstInfoSPtr& stgInstInfo, const PosSnapshotSPtr& posUpdate) {
  for (const auto& rec : posUpdate->getPosInfoDetail()) {
    LOG_D("StgInstId: {} {} {}", stgInstInfo->stgInstId_, __func__,
          rec.second->toStr());
  }
}

void StgInstTaskHandlerOfSpotPerfTest::onPosSnapshotOfStgId(
    const StgInstInfoSPtr& stgInstInfo, const PosSnapshotSPtr& posSnapshot) {
  for (const auto& rec : posSnapshot->getPosInfoDetail()) {
    LOG_D("StgInstId: {} {} {}", stgInstInfo->stgInstId_, __func__,
          rec.second->toStr());
  }
}

void StgInstTaskHandlerOfSpotPerfTest::onPosUpdateOfStgId(
    const StgInstInfoSPtr& stgInstInfo, const PosSnapshotSPtr& posUpdate) {
  for (const auto& rec : posUpdate->getPosInfoDetail()) {
    LOG_D("StgInstId: {} {} {}", stgInstInfo->stgInstId_, __func__,
          rec.second->toStr());
  }

  if (StgInstIdOfTriggerSignal(stgInstInfo) == 1) {
    const auto queryCond =
        "stgId=10000&stgInstId=1&marketCode=Binance&"
        "symbolType=Spot&symbolCode=ADA-USDT";
    const auto [statusCode, posInfoGroup] =
        posUpdate->queryPosInfoGroup(queryCond);
    if (statusCode == 0 && !posInfoGroup->empty()) {
      LOG_I("Query posInfoGroup of {}. [size = {}] [{}]", queryCond,
            posInfoGroup->size(), (*posInfoGroup)[0]->toStr());
    } else {
      LOG_W("Query posInfoGroup of {} failed. [{}]", queryCond);
    }
  }

  if (StgInstIdOfTriggerSignal(stgInstInfo) == 1) {
    const auto queryCond = "stgId=10000";
    const auto [statusCode, pnl] =
        posUpdate->queryPnl(queryCond, QuoteCurrencyForCalc("USDT"));
    if (statusCode == 0 && pnl) {
      getStgEng()->saveToDB(pnl);
      LOG_I("Query pnl of [{}]", pnl->toStr());
    } else {
      LOG_W("Query pnl of {} failed. [{} - {}]", queryCond, statusCode,
            GetStatusMsg(statusCode));
    }
  }

  if (StgInstIdOfTriggerSignal(stgInstInfo) == 1) {
    const auto queryCond = "stgId=10000&stgInstId=1";
    const auto [statusCode, pnl] =
        posUpdate->queryPnl(queryCond, QuoteCurrencyForCalc("USDT"));
    if (statusCode == 0 && pnl) {
      getStgEng()->saveToDB(pnl);
      LOG_I("Query pnl of [{}]", pnl->toStr());
    } else {
      LOG_W("Query pnl of {} failed. [{} - {}]", queryCond, statusCode,
            GetStatusMsg(statusCode));
    }
  }

  if (StgInstIdOfTriggerSignal(stgInstInfo) == 1) {
    const auto queryCond = "stgId=10000&stgInstId=2";
    const auto [statusCode, pnl] =
        posUpdate->queryPnl(queryCond, QuoteCurrencyForCalc("USDT"));
    if (statusCode == 0 && pnl) {
      getStgEng()->saveToDB(pnl);
      LOG_I("Query pnl of [{}]", pnl->toStr());
    } else {
      LOG_W("Query pnl of {} failed. [{} - {}]", queryCond, statusCode,
            GetStatusMsg(statusCode));
    }
  }

  if (StgInstIdOfTriggerSignal(stgInstInfo) == 1) {
    const auto queryCond = "stgId=10000&stgInstId=2&symbolCode=DOT-USDT-Perp";
    const auto [statusCode, pnl] =
        posUpdate->queryPnl(queryCond, QuoteCurrencyForCalc("USDT"));
    if (statusCode == 0 && pnl) {
      getStgEng()->saveToDB(pnl);
      LOG_I("Query pnl of [{}]", pnl->toStr());
    } else {
      LOG_W("Query pnl of {} failed. [{} - {}]", queryCond, statusCode,
            GetStatusMsg(statusCode));
    }
  }

  if (StgInstIdOfTriggerSignal(stgInstInfo) == 1) {
    const auto queryCond = "stgId=10000&stgInstId=2&symbolCode=ETH-USDT-Perp";
    const auto [statusCode, pnl] =
        posUpdate->queryPnl(queryCond, QuoteCurrencyForCalc("USDT"));
    if (statusCode == 0 && pnl) {
      getStgEng()->saveToDB(pnl);
      LOG_I("Query pnl of [{}]", pnl->toStr());
    } else {
      LOG_W("Query pnl of {} failed. [{} - {}]", queryCond, statusCode,
            GetStatusMsg(statusCode));
    }
  }
}

void StgInstTaskHandlerOfSpotPerfTest::onPosSnapshotOfStgInstId(
    const StgInstInfoSPtr& stgInstInfo, const PosSnapshotSPtr& posSnapshot) {
  for (const auto& rec : posSnapshot->getPosInfoDetail()) {
    LOG_D("StgInstId: {} {} {}", stgInstInfo->stgInstId_, __func__,
          rec.second->toStr());
  }
}

void StgInstTaskHandlerOfSpotPerfTest::onPosUpdateOfStgInstId(
    const StgInstInfoSPtr& stgInstInfo, const PosSnapshotSPtr& posUpdate) {
  for (const auto& rec : posUpdate->getPosInfoDetail()) {
    LOG_D("StgInstId: {} {} {}", stgInstInfo->stgInstId_, __func__,
          rec.second->toStr());
  }
}

void StgInstTaskHandlerOfSpotPerfTest::onAssetsSnapshot(
    const StgInstInfoSPtr& stgInstInfo,
    const AssetsSnapshotSPtr& assetsSnapshot) {
  for (const auto& rec : *assetsSnapshot) {
    LOG_D("StgInstId: {} {} {}", stgInstInfo->stgInstId_, __func__,
          rec.second->toStr());
  }
}

void StgInstTaskHandlerOfSpotPerfTest::onAssetsUpdate(
    const StgInstInfoSPtr& stgInstInfo, const AssetsUpdateSPtr& assetsUpdate) {
  for (const auto& rec : *assetsUpdate) {
    LOG_D("StgInstId: {} {} {}", stgInstInfo->stgInstId_, __func__,
          rec.second->toStr());
  }
}

void StgInstTaskHandlerOfSpotPerfTest::onOtherStgInstTask(
    const StgInstInfoSPtr& stgInstInfo, const SHMIPCAsyncTaskSPtr& asyncTask) {}

}  // namespace bq::stg
