#include "PosMgr.hpp"

#include "PosMgrUtil.hpp"
#include "db/DBEng.hpp"
#include "db/TBLPosInfo.hpp"
#include "db/TBLRecSetMaker.hpp"
#include "def/BQConst.hpp"
#include "def/DataStruOfTD.hpp"
#include "def/Def.hpp"
#include "def/StatusCode.hpp"
#include "util/Float.hpp"
#include "util/Json.hpp"
#include "util/Logger.hpp"

namespace bq {

PosMgr::PosMgr() : posInfoTable_(std::make_shared<PosInfoTable>()) {}

int PosMgr::init(const YAML::Node& node, const db::DBEngSPtr& dbEng,
                 const std::string& sql) {
  node_ = node;
  dbEng_ = dbEng;

  const auto ret = initPosInfoTable(sql);
  if (ret != 0) {
    LOG_W("Init failed. [{}]", sql);
    return ret;
  }
  return 0;
}

int PosMgr::initPosInfoTable(const std::string& sql) {
  const auto [ret, tblRecSet] =
      db::TBLRecSetMaker<TBLPosInfo>::ExecSql(dbEng_, sql);
  if (ret != 0) {
    LOG_W("Init pos info group failed. {}", sql);
    return ret;
  }

  for (const auto& tblRec : *tblRecSet) {
    const auto recPosInfo = tblRec.second->getRecWithAllFields();
    const auto posInfo = MakePosInfo(recPosInfo);
    posInfoTable_->emplace(posInfo);
  }
  LOG_I("Init pos info group success. [size = {}]", posInfoTable_->size());

  return 0;
}

std::string PosMgr::toStr() const {
  std::string ret;
  {
    for (const auto& rec : *posInfoTable_) {
      ret = ret + "\n" + rec->toStr();
    }
  }
  return ret;
}

PosChgInfoSPtr PosMgr::updateByOrderInfoFromTDGW(const OrderInfoSPtr& orderInfo,
                                                 LockFunc lockFunc) {
  if (orderInfo->orderStatus_ != OrderStatus::PartialFilled &&
      orderInfo->orderStatus_ != OrderStatus::Filled) {
    return std::make_shared<PosChgInfo>();
  }

  {
    SPIN_LOCK(mtxPosInfoTable_);

    if (orderInfo->symbolType_ == SymbolType::Spot ||
        orderInfo->symbolType_ == SymbolType::Perp ||
        orderInfo->symbolType_ == SymbolType::Futures ||
        orderInfo->symbolType_ == SymbolType::CPerp ||
        orderInfo->symbolType_ == SymbolType::CFutures) {
      return updateByOrderInfo(orderInfo);

    } else {
      LOG_W("Unhandled symbolType {}.",
            magic_enum::enum_name(orderInfo->symbolType_));
      return std::make_shared<PosChgInfo>();
    }
  }
}

PosChgInfoSPtr PosMgr::updateByOrderInfo(const OrderInfoSPtr& orderInfo) {
  if (orderInfo->posSide_ == PosSide::Both) {
    return updateByOrderInfoSinglePosSide(orderInfo);

  } else if (orderInfo->posSide_ == PosSide::Long ||
             orderInfo->posSide_ == PosSide::Short) {
    return updateByOrderInfoDoublePosSide(orderInfo);

  } else {
    return std::make_shared<PosChgInfo>();
    LOG_W("Unhandled posSide {}.", magic_enum::enum_name(orderInfo->posSide_));
  }
}

PosChgInfoSPtr PosMgr::updateByOrderInfoSinglePosSide(
    const OrderInfoSPtr& orderInfo) {
  if (orderInfo->side_ == Side::Bid) {
    return updateByOrderInfoSinglePosSideOfBid(orderInfo);

  } else if (orderInfo->side_ == Side::Ask) {
    return updateByOrderInfoSinglePosSideOfAsk(orderInfo);

  } else {
    return std::make_shared<PosChgInfo>();
    LOG_W("Unhandled side {}.", magic_enum::enum_name(orderInfo->posSide_));
  }
}

PosChgInfoSPtr PosMgr::updateByOrderInfoSinglePosSideOfBid(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<PosChgInfo>();

  PosInfoSPtr origPosInfoOfBid;
  PosInfoSPtr origPosInfoOfAsk;
  auto& idx = posInfoTable_->get<TagMain>();

  const auto posKeyOfBid = orderInfo->getPosKeyOfBid();
  const auto posKeyOfBidHash =
      XXH3_64bits(posKeyOfBid.data(), posKeyOfBid.size());
  auto iterBid = idx.find(posKeyOfBidHash);
  if (iterBid != std::end(idx)) {
    origPosInfoOfBid = *iterBid;
  } else {
    origPosInfoOfBid = MakePosInfoOfContractWithKeyFields(orderInfo, Side::Bid);
    posInfoTable_->emplace(origPosInfoOfBid);
  }

  const auto posKeyOfAsk = orderInfo->getPosKeyOfAsk();
  const auto posKeyOfAskHash =
      XXH3_64bits(posKeyOfAsk.data(), posKeyOfAsk.size());
  auto iterAsk = idx.find(posKeyOfAskHash);
  if (iterAsk != std::end(idx)) {
    origPosInfoOfAsk = *iterAsk;
  } else {
    origPosInfoOfAsk = MakePosInfoOfContractWithKeyFields(orderInfo, Side::Ask);
    posInfoTable_->emplace(origPosInfoOfAsk);
  }

  const auto origPosSide =
      origPosInfoOfBid->pos_ >= (-1.0 * origPosInfoOfAsk->pos_)
          ? PosSide::Long
          : PosSide::Short;

  if (origPosSide == PosSide::Long) {
    const auto newPos = origPosInfoOfBid->pos_ + orderInfo->lastDealSize_;
    origPosInfoOfBid->avgOpenPrice_ =
        recalcAvgOpenPrice(orderInfo, origPosInfoOfBid, newPos);
    origPosInfoOfBid->pos_ = newPos;
    origPosInfoOfBid->fee_ += orderInfo->getFeeOfLastTrade();
    origPosInfoOfBid->totalBidSize_ += orderInfo->lastDealSize_;
    origPosInfoOfBid->lastNoUsedToCalcPos_ = orderInfo->noUsedToCalcPos_;
    ret->emplace_back(origPosInfoOfBid);

  } else {
    auto newPos = origPosInfoOfAsk->pos_ + orderInfo->lastDealSize_;
    if (isApproximatelyZero(newPos)) newPos = 0;
    if (newPos <= 0) {
      const auto curPnlReal =
          calcCurPnlRealOfCloseShort(orderInfo, origPosInfoOfAsk, newPos);
      origPosInfoOfAsk->pnlReal_ += curPnlReal;
      origPosInfoOfAsk->pos_ = newPos;
      origPosInfoOfAsk->fee_ += orderInfo->getFeeOfLastTrade();
      origPosInfoOfAsk->totalBidSize_ += orderInfo->lastDealSize_;
      origPosInfoOfAsk->lastNoUsedToCalcPos_ = orderInfo->noUsedToCalcPos_;
      ret->emplace_back(origPosInfoOfAsk);

    } else {
      const auto curPnlRealOfAsk =
          calcCurPnlRealOfCloseShort(orderInfo, origPosInfoOfAsk, newPos);
      origPosInfoOfAsk->pnlReal_ += curPnlRealOfAsk;
      origPosInfoOfAsk->pos_ = 0;
      origPosInfoOfAsk->lastNoUsedToCalcPos_ = orderInfo->noUsedToCalcPos_;
      ret->emplace_back(origPosInfoOfAsk);

      origPosInfoOfBid->avgOpenPrice_ = orderInfo->lastDealPrice_;
      origPosInfoOfBid->pos_ = newPos;
      origPosInfoOfBid->fee_ += orderInfo->getFeeOfLastTrade();
      origPosInfoOfAsk->totalBidSize_ += orderInfo->lastDealSize_;
      origPosInfoOfBid->lastNoUsedToCalcPos_ = orderInfo->noUsedToCalcPos_;
      ret->emplace_back(origPosInfoOfBid);
    }
  }

  return ret;
}

PosChgInfoSPtr PosMgr::updateByOrderInfoSinglePosSideOfAsk(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<PosChgInfo>();

  PosInfoSPtr origPosInfoOfBid;
  PosInfoSPtr origPosInfoOfAsk;
  auto& idx = posInfoTable_->get<TagMain>();

  const auto posKeyOfBid = orderInfo->getPosKeyOfBid();
  const auto posKeyOfBidHash =
      XXH3_64bits(posKeyOfBid.data(), posKeyOfBid.size());
  auto iterBid = idx.find(posKeyOfBidHash);
  if (iterBid != std::end(idx)) {
    origPosInfoOfBid = *iterBid;
  } else {
    origPosInfoOfBid = MakePosInfoOfContractWithKeyFields(orderInfo, Side::Bid);
    posInfoTable_->emplace(origPosInfoOfBid);
  }

  const auto posKeyOfAsk = orderInfo->getPosKeyOfAsk();
  const auto posKeyOfAskHash =
      XXH3_64bits(posKeyOfAsk.data(), posKeyOfAsk.size());
  auto iterAsk = idx.find(posKeyOfAskHash);
  if (iterAsk != std::end(idx)) {
    origPosInfoOfAsk = *iterAsk;
  } else {
    origPosInfoOfAsk = MakePosInfoOfContractWithKeyFields(orderInfo, Side::Ask);
    posInfoTable_->emplace(origPosInfoOfAsk);
  }

  const auto origPosSide =
      (-1.0 * origPosInfoOfAsk->pos_) >= origPosInfoOfBid->pos_ ? PosSide::Short
                                                                : PosSide::Long;

  if (origPosSide == PosSide::Short) {
    const auto newPos = origPosInfoOfAsk->pos_ + orderInfo->lastDealSize_;
    origPosInfoOfAsk->avgOpenPrice_ =
        recalcAvgOpenPrice(orderInfo, origPosInfoOfAsk, newPos);
    origPosInfoOfAsk->pos_ = newPos;
    origPosInfoOfAsk->fee_ += orderInfo->getFeeOfLastTrade();
    origPosInfoOfAsk->totalAskSize_ += orderInfo->lastDealSize_;
    origPosInfoOfAsk->lastNoUsedToCalcPos_ = orderInfo->noUsedToCalcPos_;
    ret->emplace_back(origPosInfoOfAsk);

  } else {
    auto newPos = origPosInfoOfBid->pos_ + orderInfo->lastDealSize_;
    if (isApproximatelyZero(newPos)) newPos = 0;
    if (newPos >= 0) {
      const auto curPnlReal =
          calcCurPnlRealOfCloseLong(orderInfo, origPosInfoOfBid, newPos);
      origPosInfoOfBid->pnlReal_ += curPnlReal;
      origPosInfoOfBid->pos_ = newPos;
      origPosInfoOfBid->fee_ += orderInfo->getFeeOfLastTrade();
      origPosInfoOfBid->totalAskSize_ += orderInfo->lastDealSize_;
      origPosInfoOfBid->lastNoUsedToCalcPos_ = orderInfo->noUsedToCalcPos_;
      ret->emplace_back(origPosInfoOfBid);

    } else {
      const auto curPnlRealOfBid =
          calcCurPnlRealOfCloseLong(orderInfo, origPosInfoOfBid, newPos);
      origPosInfoOfBid->pnlReal_ += curPnlRealOfBid;
      origPosInfoOfBid->pos_ = 0;
      origPosInfoOfBid->lastNoUsedToCalcPos_ = orderInfo->noUsedToCalcPos_;
      ret->emplace_back(origPosInfoOfBid);

      origPosInfoOfAsk->avgOpenPrice_ = orderInfo->lastDealPrice_;
      origPosInfoOfAsk->pos_ = newPos;
      origPosInfoOfAsk->fee_ += orderInfo->getFeeOfLastTrade();
      origPosInfoOfBid->totalAskSize_ += orderInfo->lastDealSize_;
      origPosInfoOfAsk->lastNoUsedToCalcPos_ = orderInfo->noUsedToCalcPos_;
      ret->emplace_back(origPosInfoOfAsk);
    }
  }

  return ret;
}

PosChgInfoSPtr PosMgr::updateByOrderInfoDoublePosSide(
    const OrderInfoSPtr& orderInfo) {
  if (orderInfo->posSide_ == PosSide::Long) {
    if (orderInfo->side_ == Side::Bid) {
      return updateByOrderInfoDoublePosSideOfBidLong(orderInfo);

    } else if (orderInfo->side_ == Side::Ask) {
      return updateByOrderInfoDoublePosSideOfAskLong(orderInfo);

    } else {
      return std::make_shared<PosChgInfo>();
      LOG_W("Unhandled side {}.", magic_enum::enum_name(orderInfo->side_));
    }

  } else if (orderInfo->posSide_ == PosSide::Short) {
    if (orderInfo->side_ == Side::Ask) {
      return updateByOrderInfoDoublePosSideOfAskShort(orderInfo);

    } else if (orderInfo->side_ == Side::Bid) {
      return updateByOrderInfoDoublePosSideOfBidShort(orderInfo);

    } else {
      return std::make_shared<PosChgInfo>();
      LOG_W("Unhandled side {}.", magic_enum::enum_name(orderInfo->side_));
    }

  } else {
    return std::make_shared<PosChgInfo>();
    LOG_W("Unhandled posSide {}.", magic_enum::enum_name(orderInfo->side_));
  }
}

PosChgInfoSPtr PosMgr::updateByOrderInfoDoublePosSideOfBidLong(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<PosChgInfo>();

  auto& idx = posInfoTable_->get<TagMain>();
  const auto posKey = orderInfo->getPosKey();
  const auto posKeyHash = XXH3_64bits(posKey.data(), posKey.size());
  auto iter = idx.find(posKeyHash);
  if (iter == std::end(idx)) {
    const auto posInfo = MakePosInfoOfContract(orderInfo);
    posInfoTable_->emplace(posInfo);
    posInfo->totalBidSize_ += orderInfo->lastDealSize_;
    posInfo->lastNoUsedToCalcPos_ = orderInfo->noUsedToCalcPos_;
    ret->emplace_back(posInfo);
    return ret;
  }

  auto& origPosInfo = *iter;
  const auto newPos = origPosInfo->pos_ + orderInfo->lastDealSize_;
  origPosInfo->fee_ = origPosInfo->fee_ + orderInfo->getFeeOfLastTrade();
  origPosInfo->totalBidSize_ += orderInfo->lastDealSize_;
  origPosInfo->avgOpenPrice_ =
      recalcAvgOpenPrice(orderInfo, origPosInfo, newPos);
  origPosInfo->pos_ = newPos;
  origPosInfo->updateTime_ = GetTotalUSSince1970();
  origPosInfo->lastNoUsedToCalcPos_ = orderInfo->noUsedToCalcPos_;
  ret->emplace_back(origPosInfo);

  return ret;
}

PosChgInfoSPtr PosMgr::updateByOrderInfoDoublePosSideOfAskLong(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<PosChgInfo>();

  auto& idx = posInfoTable_->get<TagMain>();
  const auto posKeyOfBid = orderInfo->getPosKeyOfBid();
  const auto posKeyOfBidHash =
      XXH3_64bits(posKeyOfBid.data(), posKeyOfBid.size());
  auto iter = idx.find(posKeyOfBidHash);
  if (iter == std::end(idx) || (*iter)->pos_ == 0) {
    LOG_W("Can not find long pos when ask long. {}", posKeyOfBid);
    return ret;
  }

  auto& origPosInfo = *iter;

  auto newPos = origPosInfo->pos_ + orderInfo->lastDealSize_;
  if (newPos < 0) {  // like -3.469446951953614e-18 or invalid value
    LOG_I("Long pos {} less than 0 after ask long. {}", newPos, posKeyOfBid);
    newPos = 0;
  }
  origPosInfo->fee_ = origPosInfo->fee_ + orderInfo->getFeeOfLastTrade();
  origPosInfo->totalAskSize_ += orderInfo->lastDealSize_;
  const auto pnlReal =
      calcCurPnlRealOfCloseLong(orderInfo, origPosInfo, newPos);
  origPosInfo->pnlReal_ += pnlReal;
  origPosInfo->pos_ = newPos;
  origPosInfo->updateTime_ = GetTotalUSSince1970();
  origPosInfo->lastNoUsedToCalcPos_ = orderInfo->noUsedToCalcPos_;
  ret->emplace_back(origPosInfo);

  return ret;
}

PosChgInfoSPtr PosMgr::updateByOrderInfoDoublePosSideOfAskShort(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<PosChgInfo>();

  auto& idx = posInfoTable_->get<TagMain>();
  const auto posKey = orderInfo->getPosKey();
  const auto posKeyHash = XXH3_64bits(posKey.data(), posKey.size());
  auto iter = idx.find(posKeyHash);
  if (iter == std::end(idx)) {
    const auto posInfo = MakePosInfoOfContract(orderInfo);
    posInfoTable_->emplace(posInfo);
    posInfo->totalAskSize_ += orderInfo->lastDealSize_;
    posInfo->lastNoUsedToCalcPos_ = orderInfo->noUsedToCalcPos_;
    ret->emplace_back(posInfo);
    return ret;
  }

  auto& origPosInfo = *iter;
  const auto newPos = origPosInfo->pos_ + orderInfo->lastDealSize_;
  origPosInfo->fee_ = origPosInfo->fee_ + orderInfo->getFeeOfLastTrade();
  origPosInfo->totalAskSize_ += orderInfo->lastDealSize_;
  origPosInfo->avgOpenPrice_ =
      recalcAvgOpenPrice(orderInfo, origPosInfo, newPos);
  origPosInfo->pos_ = newPos;
  origPosInfo->updateTime_ = GetTotalUSSince1970();
  origPosInfo->lastNoUsedToCalcPos_ = orderInfo->noUsedToCalcPos_;
  ret->emplace_back(origPosInfo);

  return ret;
}

PosChgInfoSPtr PosMgr::updateByOrderInfoDoublePosSideOfBidShort(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<PosChgInfo>();

  auto& idx = posInfoTable_->get<TagMain>();
  const auto posKeyOfAsk = orderInfo->getPosKeyOfAsk();
  const auto posKeyOfAskHash =
      XXH3_64bits(posKeyOfAsk.data(), posKeyOfAsk.size());
  auto iter = idx.find(posKeyOfAskHash);
  if (iter == std::end(idx) || (*iter)->pos_ == 0) {
    LOG_W("Can not find short pos when bid short. {}", posKeyOfAsk);
    return ret;
  }

  auto& origPosInfo = *iter;

  auto newPos = origPosInfo->pos_ + orderInfo->lastDealSize_;
  if (newPos > 0) {  // like 3.469446951953614e-18 or invalid value
    LOG_I("Short pos {} greater than 0 after bid short. {}", newPos,
          posKeyOfAsk);
    newPos = 0;
  }
  origPosInfo->fee_ = origPosInfo->fee_ + orderInfo->getFeeOfLastTrade();
  origPosInfo->totalBidSize_ += orderInfo->lastDealSize_;
  const auto pnlReal =
      calcCurPnlRealOfCloseShort(orderInfo, origPosInfo, newPos);
  origPosInfo->pnlReal_ += pnlReal;
  origPosInfo->pos_ = newPos;
  origPosInfo->updateTime_ = GetTotalUSSince1970();
  origPosInfo->lastNoUsedToCalcPos_ = orderInfo->noUsedToCalcPos_;
  ret->emplace_back(origPosInfo);

  return ret;
}

Decimal PosMgr::recalcAvgOpenPrice(const OrderInfoSPtr& orderInfo,
                                   const PosInfoSPtr& origPosInfo,
                                   Decimal newPos) {
  Decimal ret = 0;
  if (orderInfo->symbolType_ == SymbolType::Spot) {
    ret = (origPosInfo->pos_ * origPosInfo->avgOpenPrice_ +
           orderInfo->lastDealSize_ * orderInfo->lastDealPrice_) /
          newPos;

  } else if (orderInfo->symbolType_ == SymbolType::Perp ||
             orderInfo->symbolType_ == SymbolType::Futures) {
    ret = (origPosInfo->pos_ * origPosInfo->avgOpenPrice_ +
           orderInfo->lastDealSize_ * orderInfo->lastDealPrice_) /
          newPos;

  } else if (orderInfo->symbolType_ == SymbolType::CPerp ||
             orderInfo->symbolType_ == SymbolType::CFutures) {
    if (!isApproximatelyZero(origPosInfo->avgOpenPrice_)) {
      ret = newPos / (origPosInfo->pos_ / origPosInfo->avgOpenPrice_ +
                      orderInfo->lastDealSize_ / orderInfo->lastDealPrice_);
    } else {
      ret = newPos / (orderInfo->lastDealSize_ / orderInfo->lastDealPrice_);
    }

  } else {
    LOG_W("Unhandled symbolType {}.",
          magic_enum::enum_name(orderInfo->symbolType_));
  }
  return ret;
}

Decimal PosMgr::calcCurPnlRealOfCloseShort(const OrderInfoSPtr& orderInfo,
                                           const PosInfoSPtr& origPosInfo,
                                           Decimal newPos) {
  const auto closePos =
      newPos > 0 ? (origPosInfo->pos_ * -1) : orderInfo->lastDealSize_;

  const auto ret = calcPnlOfCloseShort(
      orderInfo->symbolType_, origPosInfo->avgOpenPrice_,
      orderInfo->lastDealPrice_, closePos, orderInfo->parValue_);

  return ret;
}

Decimal PosMgr::calcCurPnlRealOfCloseLong(const OrderInfoSPtr& orderInfo,
                                          const PosInfoSPtr& origPosInfo,
                                          Decimal newPos) {
  const auto closePos =
      newPos < 0 ? origPosInfo->pos_ : (orderInfo->lastDealSize_ * -1);

  const auto ret = calcPnlOfCloseLong(
      orderInfo->symbolType_, origPosInfo->avgOpenPrice_,
      orderInfo->lastDealPrice_, closePos, orderInfo->parValue_);

  return ret;
}

PosInfoGroup PosMgr::getPosInfoGroup(LockFunc lockFunc) const {
  PosInfoGroup ret;
  {
    SPIN_LOCK(mtxPosInfoTable_);
    for (const auto& posInfo : *posInfoTable_) {
      ret.emplace_back(std::make_shared<PosInfo>(*posInfo));
    }
  }
  return ret;
}

void PosMgr::syncToDB(const PosInfoSPtr& posInfo) {
  if (syncToDB_ == SyncToDB::False) {
    return;
  }
  const auto identity = GET_RAND_STR();
  const auto sql = posInfo->getSqlOfReplace();
  const auto [ret, execRet] = dbEng_->asyncExec(identity, sql);
  if (ret != 0) {
    LOG_W("Replace pos info from db failed. [{}]", sql);
  }
}

}  // namespace bq
