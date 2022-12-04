/*!
 * \file SimedOrderInfoHandler.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/12/01
 *
 * \brief
 */

#include "SimedOrderInfoHandler.hpp"

#include "Config.hpp"
#include "HttpCliOfExch.hpp"
#include "OrdMgr.hpp"
#include "PosMgr.hpp"
#include "SHMIPC.hpp"
#include "TDSvc.hpp"
#include "TDSvcDef.hpp"
#include "db/TBLMonitorOfSymbolInfo.hpp"
#include "db/TBLSymbolInfo.hpp"
#include "def/BQDef.hpp"
#include "def/Const.hpp"
#include "def/DataStruOfMD.hpp"
#include "def/DataStruOfOthers.hpp"
#include "def/DataStruOfTD.hpp"
#include "def/OrderInfoIF.hpp"
#include "def/SimedTDInfo.hpp"
#include "def/StatusCode.hpp"
#include "def/TaskOfSync.hpp"
#include "util/Datetime.hpp"
#include "util/Fee.hpp"
#include "util/Float.hpp"
#include "util/Logger.hpp"
#include "util/Random.hpp"

namespace bq::td::svc {

SimedOrderInfoHandler::SimedOrderInfoHandler(TDSvc* tdSvc) : tdSvc_(tdSvc) {
  defaultFeeCurrency_ = CONFIG["simedMode"]["feeCurrency"].as<std::string>("");
  feeRatioOfMaker_ = CONFIG["simedMode"]["feeRatioOfMaker"].as<Decimal>(0.002);
  feeRatioOfTaker_ = CONFIG["simedMode"]["feeRatioOfTaker"].as<Decimal>(0.003);
  milliSecIntervalOfSimOrderStatus_ =
      CONFIG["simedMode"]["milliSecIntervalOfSimOrderStatus"].as<std::uint32_t>(
          0);
}

void SimedOrderInfoHandler::simOnOrder(OrderInfoSPtr& ordReq) {
  const auto [statusCode, simedTDInfo] = MakeSimedTDInfo(ordReq->simedTDInfo_);
  if (statusCode != 0) {
    ordReq->orderStatus_ = OrderStatus::Failed;
    ordReq->statusCode_ = statusCode;
    LOG_W("Handle order in simed td mode failed. [{} - {}] {}", statusCode,
          GetStatusMsg(statusCode), ordReq->toShortStr());
    tdSvc_->getOrdMgr()->remove(ordReq->orderId_);
    tdSvc_->getSHMCliOfTDSrv()->asyncSendMsgWithZeroCopy(
        [&](void* shmBuf) {
          InitMsgBody(shmBuf, *ordReq);
#ifndef OPT_LOG
          LOG_I("Send simed order ret. {}",
                static_cast<OrderInfo*>(shmBuf)->toShortStr());
#endif
        },
        MSG_ID_ON_ORDER_RET, sizeof(OrderInfo));

    tdSvc_->cacheTaskOfSyncGroup(MSG_ID_ON_ORDER_RET,
                                 std::make_shared<OrderInfo>(*ordReq),
                                 SyncToRiskMgr::True, SyncToDB::True);
    return;
  }
  simOnOrder(ordReq, simedTDInfo);
}

void SimedOrderInfoHandler::simOnOrder(OrderInfoSPTr& ordReq,
                                       const SimedTDInfoSPtr& simedTDInfo) {
  switch (simedTDInfo->orderStatus_) {
    case OrderStatus::ConfirmedByExch:
      simOnOrderConfirmedByExch(ordReq, simedTDInfo);
      break;

    case OrderStatus::PartialFilled: {
      const auto symbolInfo = simOnOrderConfirmedByExch(ordReq, simedTDInfo);
      if (symbolInfo) {
        simOnOrderPartialFilled(ordReq, simedTDInfo, symbolInfo);
      }
    } break;

    case OrderStatus::Filled: {
      const auto symbolInfo = simOnOrderConfirmedByExch(ordReq, simedTDInfo);
      if (symbolInfo) {
        simOnOrderFilled(ordReq, simedTDInfo, symbolInfo);
      }
    } break;

    case OrderStatus::Failed:
      simOnOrderFailed(ordReq, simedTDInfo);
      break;

    default:
      assert(1 == 2 && "Unhandled order status");
      break;
  }
}

db::symbolInfo::RecordSPtr SimedOrderInfoHandler::simOnOrderConfirmedByExch(
    OrderInfoSPTr& ordReq, const SimedTDInfoSPtr& simedTDInfo) {
  const auto marketCode =
      std::string(magic_enum::enum_name(ordReq->marketCode_));
  const auto [statusCode, symbolInfo] =
      tdSvc_->getTBLMonitorOfSymbolInfo()->getRecSymbolInfoBySymbolCode(
          marketCode, ordReq->symbolCode_);
  if (statusCode != SCODE_SUCCESS) {
    LOG_W(
        "Handle on order confirmed by exch in simed td mode failed because of "
        "query symbol info of {} - {} failed. [{} - {}]",
        marketCode, ordReq->symbolCode_, statusCode, GetStatusMsg(statusCode));
    ordReq->orderStatus_ = OrderStatus::Failed;
    ordReq->statusCode_ = statusCode;
    tdSvc_->getOrdMgr()->remove(ordReq->orderId_);
    tdSvc_->getSHMCliOfTDSrv()->asyncSendMsgWithZeroCopy(
        [&](void* shmBuf) {
          InitMsgBody(shmBuf, *ordReq);
#ifndef OPT_LOG
          LOG_I("Send simed order ret. {}",
                static_cast<OrderInfo*>(shmBuf)->toShortStr());
#endif
        },
        MSG_ID_ON_ORDER_RET, sizeof(OrderInfo));
    tdSvc_->cacheTaskOfSyncGroup(MSG_ID_ON_ORDER_RET,
                                 std::make_shared<OrderInfo>(*ordReq),
                                 SyncToRiskMgr::True, SyncToDB::True);
    return nullptr;
  }

  ordReq->orderStatus_ = OrderStatus::ConfirmedByExch;
  ordReq->statusCode_ = SCODE_SUCCESS;
  ordReq->exchOrderId_ = GET_RAND_INT();
  ordReq->dealSize_ = 0;
  ordReq->avgDealPrice_ = 0;
  ordReq->lastTradeId_[0] = '\0';
  ordReq->lastDealPrice_ = 0;
  ordReq->lastDealSize_ = 0;
  ordReq->lastDealTime_ = GetTotalUSSince1970();

  ordReq->fee_ = 0;
  const auto feeCurrency = getFeeCurrency(ordReq, symbolInfo->baseCurrency,
                                          symbolInfo->quoteCurrency);
  strncpy(ordReq->feeCurrency_, feeCurrency.c_str(),
          sizeof(ordReq->feeCurrency_) - 1);

  const auto [isTheOrderInfoUpdated, orderInfoInOrdMgr] =
      tdSvc_->getOrdMgr()->updateByOrderInfoFromExch(
          ordReq, tdSvc_->getNextNoUsedToCalcPos(), DeepClone::True);

  tdSvc_->getSHMCliOfTDSrv()->asyncSendMsgWithZeroCopy(
      [&](void* shmBuf) {
        InitMsgBody(shmBuf, *orderInfoInOrdMgr);
#ifndef OPT_LOG
        LOG_I("Send simed order ret. {}",
              static_cast<OrderInfo*>(shmBuf)->toShortStr());
#endif
      },
      MSG_ID_ON_ORDER_RET, sizeof(OrderInfo));

  tdSvc_->cacheTaskOfSyncGroup(MSG_ID_ON_ORDER_RET, orderInfoInOrdMgr,
                               SyncToRiskMgr::True, SyncToDB::True);

  return symbolInfo;
}

void SimedOrderInfoHandler::simOnOrderFilled(
    OrderInfoSPTr& ordReq, const SimedTDInfoSPtr& simedTDInfo,
    const db::symbolInfo::RecordSPtr& symbolInfo) {
  ordReq->orderStatus_ = OrderStatus::PartialFilled;
  ordReq->statusCode_ = SCODE_SUCCESS;
  ordReq->exchOrderId_ = 0;

  for (std::size_t i = 0; i < simedTDInfo->transDetailGroup_.size(); ++i) {
    if (milliSecIntervalOfSimOrderStatus_ != 0) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(milliSecIntervalOfSimOrderStatus_));
    }

    if (i == simedTDInfo->transDetailGroup_.size() - 1) {
      ordReq->orderStatus_ = OrderStatus::Filled;
    }

    const auto lastTradeId = fmt::format("{}", GET_RAND_INT());
    strncpy(ordReq->lastTradeId_, lastTradeId.c_str(),
            sizeof(ordReq->lastTradeId_));
    if (ordReq->side_ == Side::Bid) {
      ordReq->lastDealSize_ =
          ordReq->orderSize_ * simedTDInfo->transDetailGroup_[i]->filledPer_;
      ordReq->lastDealPrice_ =
          ordReq->orderPrice_ -
          ordReq->orderPrice_ * simedTDInfo->transDetailGroup_[i]->slippage_;
    } else {
      ordReq->lastDealSize_ = ordReq->orderSize_ *
                              simedTDInfo->transDetailGroup_[i]->filledPer_ *
                              -1;
      ordReq->lastDealPrice_ =
          ordReq->orderPrice_ +
          ordReq->orderPrice_ * simedTDInfo->transDetailGroup_[i]->slippage_;
    }
    ordReq->lastDealTime_ = GetTotalUSSince1970();

    const auto prevDealAmt = ordReq->avgDealPrice_ * ordReq->dealSize_;
    const auto lastDealAmt = ordReq->lastDealPrice_ * ordReq->lastDealSize_;
    const auto totalDealSize = ordReq->dealSize_ + ordReq->lastDealSize_;
    if (!isApproximatelyZero(totalDealSize)) {
      ordReq->avgDealPrice_ = (prevDealAmt + lastDealAmt) / totalDealSize;
    }
    ordReq->dealSize_ = totalDealSize;

    const auto liquidityDirection =
        simedTDInfo->transDetailGroup_[i]->liquidityDirection_;
    const auto feeRatio = liquidityDirection == LiquidityDirection::Maker
                              ? feeRatioOfMaker_
                              : feeRatioOfTaker_;
    ordReq->fee_ = calcFee(ordReq, feeRatio, symbolInfo->parValue);

    const auto [isTheOrderInfoUpdated, orderInfoInOrdMgr] =
        tdSvc_->getOrdMgr()->updateByOrderInfoFromExch(
            ordReq, tdSvc_->getNextNoUsedToCalcPos(), DeepClone::True);

    tdSvc_->getSHMCliOfTDSrv()->asyncSendMsgWithZeroCopy(
        [&](void* shmBuf) {
          InitMsgBody(shmBuf, *orderInfoInOrdMgr);
#ifndef OPT_LOG
          LOG_I("Send simed order ret. {}",
                static_cast<OrderInfo*>(shmBuf)->toShortStr());
#endif
        },
        MSG_ID_ON_ORDER_RET, sizeof(OrderInfo));

    tdSvc_->cacheTaskOfSyncGroup(MSG_ID_ON_ORDER_RET, orderInfoInOrdMgr,
                                 SyncToRiskMgr::True, SyncToDB::True);
  }
}

void SimedOrderInfoHandler::simOnOrderPartialFilled(
    OrderInfoSPTr& ordReq, const SimedTDInfoSPtr& simedTDInfo,
    const db::symbolInfo::RecordSPtr& symbolInfo) {
  ordReq->orderStatus_ = OrderStatus::PartialFilled;
  ordReq->statusCode_ = SCODE_SUCCESS;
  ordReq->exchOrderId_ = 0;

  for (std::size_t i = 0; i < simedTDInfo->transDetailGroup_.size(); ++i) {
    if (milliSecIntervalOfSimOrderStatus_ != 0) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(milliSecIntervalOfSimOrderStatus_));
    }

    const auto lastTradeId = fmt::format("{}", GET_RAND_INT());
    strncpy(ordReq->lastTradeId_, lastTradeId.c_str(),
            sizeof(ordReq->lastTradeId_));
    if (ordReq->side_ == Side::Bid) {
      ordReq->lastDealSize_ =
          ordReq->orderSize_ * simedTDInfo->transDetailGroup_[i]->filledPer_;
      ordReq->lastDealPrice_ =
          ordReq->orderPrice_ -
          ordReq->orderPrice_ * simedTDInfo->transDetailGroup_[i]->slippage_;
    } else {
      ordReq->lastDealSize_ = ordReq->orderSize_ *
                              simedTDInfo->transDetailGroup_[i]->filledPer_ *
                              -1;
      ordReq->lastDealPrice_ =
          ordReq->orderPrice_ +
          ordReq->orderPrice_ * simedTDInfo->transDetailGroup_[i]->slippage_;
    }
    ordReq->lastDealTime_ = GetTotalUSSince1970();

    const auto prevDealAmt = ordReq->avgDealPrice_ * ordReq->dealSize_;
    const auto lastDealAmt = ordReq->lastDealPrice_ * ordReq->lastDealSize_;
    const auto totalDealSize = ordReq->dealSize_ + ordReq->lastDealSize_;
    if (!isApproximatelyZero(totalDealSize)) {
      ordReq->avgDealPrice_ = (prevDealAmt + lastDealAmt) / totalDealSize;
    }
    ordReq->dealSize_ = totalDealSize;

    const auto liquidityDirection =
        simedTDInfo->transDetailGroup_[i]->liquidityDirection_;
    const auto feeRatio = liquidityDirection == LiquidityDirection::Maker
                              ? feeRatioOfMaker_
                              : feeRatioOfTaker_;
    ordReq->fee_ = calcFee(ordReq, feeRatio, symbolInfo->parValue);

    const auto [isTheOrderInfoUpdated, orderInfoInOrdMgr] =
        tdSvc_->getOrdMgr()->updateByOrderInfoFromExch(
            ordReq, tdSvc_->getNextNoUsedToCalcPos(), DeepClone::True);

    tdSvc_->getSHMCliOfTDSrv()->asyncSendMsgWithZeroCopy(
        [&](void* shmBuf) {
          InitMsgBody(shmBuf, *orderInfoInOrdMgr);
#ifndef OPT_LOG
          LOG_I("Send simed order ret. {}",
                static_cast<OrderInfo*>(shmBuf)->toShortStr());
#endif
        },
        MSG_ID_ON_ORDER_RET, sizeof(OrderInfo));

    tdSvc_->cacheTaskOfSyncGroup(MSG_ID_ON_ORDER_RET, orderInfoInOrdMgr,
                                 SyncToRiskMgr::True, SyncToDB::True);
  }
}

void SimedOrderInfoHandler::simOnOrderFailed(
    OrderInfoSPTr& ordReq, const SimedTDInfoSPtr& simedTDInfo) {
  ordReq->orderStatus_ = OrderStatus::Failed;
  ordReq->statusCode_ = SCODE_TD_SVC_SIMED_ORDER_STATSU_FAILED;
  tdSvc_->getOrdMgr()->remove(ordReq->orderId_);
  tdSvc_->getSHMCliOfTDSrv()->asyncSendMsgWithZeroCopy(
      [&](void* shmBuf) {
        InitMsgBody(shmBuf, *ordReq);
#ifndef OPT_LOG
        LOG_I("Send simed order ret. {}",
              static_cast<OrderInfo*>(shmBuf)->toShortStr());
#endif
      },
      MSG_ID_ON_ORDER_RET, sizeof(OrderInfo));
  tdSvc_->cacheTaskOfSyncGroup(MSG_ID_ON_ORDER_RET, ordReq, SyncToRiskMgr::True,
                               SyncToDB::True);
}

void SimedOrderInfoHandler::simOnCancelOrder(OrderInfoSPtr& ordReq) {
  int statusCode = 0;
  OrderInfoSPtr orderInfoInOrdMgr{nullptr};
  std::tie(statusCode, orderInfoInOrdMgr) =
      tdSvc_->getOrdMgr()->getOrderInfo(ordReq->orderId_, DeepClone::False);
  if (statusCode != 0 || orderInfoInOrdMgr == nullptr) {
    ordReq->statusCode_ = -1;
    tdSvc_->getSHMCliOfTDSrv()->asyncSendMsgWithZeroCopy(
        [&](void* shmBuf) {
          InitMsgBody(shmBuf, *ordReq);
#ifndef OPT_LOG
          LOG_I("Send simed order ret. {}",
                static_cast<OrderInfo*>(shmBuf)->toShortStr());
#endif
        },
        MSG_ID_ON_CANCEL_ORDER_RET, sizeof(OrderInfo));
    return;
  }

  orderInfoInOrdMgr->orderStatus_ =
      isApproximatelyZero(orderInfoInOrdMgr->dealSize_)
          ? OrderStatus::Canceled
          : OrderStatus::PartialFilledCanceled;

  tdSvc_->getOrdMgr()->remove(orderInfoInOrdMgr->orderId_);
  tdSvc_->getSHMCliOfTDSrv()->asyncSendMsgWithZeroCopy(
      [&](void* shmBuf) {
        InitMsgBody(shmBuf, *orderInfoInOrdMgr);
#ifndef OPT_LOG
        LOG_I("Send simed order ret. {}",
              static_cast<OrderInfo*>(shmBuf)->toShortStr());
#endif
      },
      MSG_ID_ON_ORDER_RET, sizeof(OrderInfo));
  tdSvc_->cacheTaskOfSyncGroup(MSG_ID_ON_ORDER_RET, orderInfoInOrdMgr,
                               SyncToRiskMgr::True, SyncToDB::True);
}

}  // namespace bq::td::svc
