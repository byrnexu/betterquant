/*!
 * \file StgEng.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "StgEng.hpp"

#include "StgEngImpl.hpp"
#include "StgInstTaskHandlerBase.hpp"
#include "StgInstTaskHandlerImpl.hpp"
#include "util/PosSnapshot.hpp"

namespace bq::stg {

StgEng::StgEng(const std::string& configFilename)
    : stgEngImpl_(std::make_shared<StgEngImpl>(configFilename)) {}

int StgEng::init(const StgInstTaskHandlerBaseSPtr& taskHandler) {
  const auto ret = stgEngImpl_->init();
  if (ret != 0) {
    return ret;
  }
  installStgInstTaskHandler(taskHandler);
  return ret;
}

int StgEng::run() { return stgEngImpl_->run(); }

void StgEng::installStgInstTaskHandler(
    const StgInstTaskHandlerBaseSPtr& taskHandler) {
  StgInstTaskHandlerBundle stgInstTaskHandlerBundle = {

      [&](const auto& stgInstInfo, const auto& orderInfo) {
        taskHandler->onOrderRet(stgInstInfo, orderInfo);
      },
      [&](const auto& stgInstInfo, const auto& orderInfo) {
        taskHandler->onCancelOrderRet(stgInstInfo, orderInfo);
      },

      [&](const auto& stgInstInfo, const auto& trades) {
        taskHandler->onTrades(stgInstInfo, trades);
      },
      [&](const auto& stgInstInfo, const auto& books) {
        taskHandler->onBooks(stgInstInfo, books);
      },
      [&](const auto& stgInstInfo, const auto& candle) {
        taskHandler->onCandle(stgInstInfo, candle);
      },
      [&](const auto& stgInstInfo, const auto& tickers) {
        taskHandler->onTickers(stgInstInfo, tickers);
      },

      [&]() { taskHandler->onStgStart(); },
      [&](const auto& stgInstInfo) {
        taskHandler->onStgInstStart(stgInstInfo);
      },

      [&](const auto& stgInstInfo) { taskHandler->onStgInstAdd(stgInstInfo); },
      [&](const auto& stgInstInfo) { taskHandler->onStgInstDel(stgInstInfo); },
      [&](const auto& stgInstInfo) { taskHandler->onStgInstChg(stgInstInfo); },
      [&](const auto& stgInstInfo) {
        taskHandler->onStgInstTimer(stgInstInfo);
      },

      [&](const auto& stgInstInfo, const auto& posUpdateOfAcctId) {
        taskHandler->onPosUpdateOfAcctId(stgInstInfo, posUpdateOfAcctId);
      },
      [&](const auto& stgInstInfo, const auto& posSnapshotOfAcctId) {
        taskHandler->onPosSnapshotOfAcctId(stgInstInfo, posSnapshotOfAcctId);
      },

      [&](const auto& stgInstInfo, const auto& posUpdateOfStgId) {
        taskHandler->onPosUpdateOfStgId(stgInstInfo, posUpdateOfStgId);
      },
      [&](const auto& stgInstInfo, const auto& posSnapshotOfStgId) {
        taskHandler->onPosSnapshotOfStgId(stgInstInfo, posSnapshotOfStgId);
      },

      [&](const auto& stgInstInfo, const auto& posUpdateOfStgInstId) {
        taskHandler->onPosUpdateOfStgInstId(stgInstInfo, posUpdateOfStgInstId);
      },
      [&](const auto& stgInstInfo, const auto& posSnapshotOfStgInstId) {
        taskHandler->onPosSnapshotOfStgInstId(stgInstInfo,
                                              posSnapshotOfStgInstId);
      },

      [&](const auto& stgInstInfo, const auto& assetsUpdate) {
        taskHandler->onAssetsUpdate(stgInstInfo, assetsUpdate);
      },
      [&](const auto& stgInstInfo, const auto& assetsSnapshot) {
        taskHandler->onAssetsSnapshot(stgInstInfo, assetsSnapshot);
      },

      [&](const auto& stgInstInfo, const auto& asyncTask) {
        taskHandler->onOtherStgInstTask(stgInstInfo, asyncTask);
      }};

  const auto stgInstTaskHandlerImpl = std::make_shared<StgInstTaskHandlerImpl>(
      stgEngImpl_.get(), stgInstTaskHandlerBundle);
  stgEngImpl_->installStgInstTaskHandler(stgInstTaskHandlerImpl);
}

std::tuple<int, OrderId> StgEng::order(const StgInstInfoSPtr& stgInstInfo,
                                       AcctId acctId,
                                       const std::string& symbolCode, Side side,
                                       PosSide posSide, Decimal orderPrice,
                                       Decimal orderSize) {
  return stgEngImpl_->order(stgInstInfo, acctId, symbolCode, side, posSide,
                            orderPrice, orderSize);
}

std::tuple<int, OrderId> StgEng::order(OrderInfoSPtr& orderInfo) {
  return stgEngImpl_->order(orderInfo);
}

int StgEng::cancelOrder(OrderId orderId) {
  return stgEngImpl_->cancelOrder(orderId);
}

MarketDataCacheSPtr StgEng::getMarketDataCache() const {
  return stgEngImpl_->getMarketDataCache();
}

std::tuple<int, OrderInfoSPtr> StgEng::getOrderInfo(OrderId orderId) const {
  return stgEngImpl_->getOrderInfo(orderId);
}

int StgEng::sub(StgInstId subscriber, const std::string& topic) {
  return stgEngImpl_->sub(subscriber, topic);
}

int StgEng::unSub(StgInstId subscriber, const std::string& topic) {
  return stgEngImpl_->unSub(subscriber, topic);
}

void StgEng::installStgInstTimer(StgInstId stgInstId,
                                 const std::string& timerName,
                                 ExecAtStartup execAtStartUp,
                                 std::uint32_t milliSecInterval,
                                 std::uint64_t maxExecTimes) {
  stgEngImpl_->installStgInstTimer(stgInstId, timerName, execAtStartUp,
                                   milliSecInterval, maxExecTimes);
}

bool StgEng::saveStgPrivateData(StgInstId stgInstId,
                                const std::string& jsonStr) {
  return stgEngImpl_->saveStgPrivateData(stgInstId, jsonStr);
}

std::string StgEng::loadStgPrivateData(StgInstId stgInstId) {
  return stgEngImpl_->loadStgPrivateData(stgInstId);
}

void StgEng::saveToDB(const PnlSPtr& pnl) {
  if (!pnl) return;
  stgEngImpl_->saveToDB(pnl);
}

}  // namespace bq::stg
