#include "OrdMgr.hpp"

#include "db/DBEng.hpp"
#include "db/TBLOrderInfo.hpp"
#include "db/TBLRecSetMaker.hpp"
#include "def/DataStruOfTD.hpp"
#include "def/Def.hpp"
#include "def/StatusCode.hpp"
#include "util/Datetime.hpp"
#include "util/Json.hpp"
#include "util/Logger.hpp"

namespace bq {

OrdMgr::OrdMgr() : orderInfoGroup_(std::make_shared<OrderInfoGroup>()) {}

int OrdMgr::init(const YAML::Node& node, const db::DBEngSPtr& dbEng,
                 const std::string& sql) {
  node_ = node;
  dbEng_ = dbEng;

  auto retOfInitOrd = initOrderInfoGroup(sql);
  if (retOfInitOrd != 0) {
    LOG_W("Init failed. [{}]", sql);
    return retOfInitOrd;
  }
  return 0;
}

int OrdMgr::initOrderInfoGroup(const std::string& sql) {
  auto [retOfMaker, tblRecSet] =
      db::TBLRecSetMaker<TBLOrderInfo>::ExecSql(dbEng_, sql);
  if (retOfMaker != 0) {
    LOG_W("Init order info group failed. {}", sql);
    return retOfMaker;
  }

  for (const auto& tblRec : *tblRecSet) {
    const auto recOrderInfo = tblRec.second->getRecWithAllFields();
    const auto orderInfo = MakeOrderInfo(recOrderInfo);
    orderInfoGroup_->emplace(orderInfo);
  }
  LOG_I("Init order info group success. [size = {}]", orderInfoGroup_->size());
  return 0;
}

int OrdMgr::add(const OrderInfoSPtr& orderInfo, DeepClone deepClone,
                LockFunc lockFunc) {
  const auto orderInfoClone = deepClone == DeepClone::True
                                  ? std::make_shared<OrderInfo>(*orderInfo)
                                  : orderInfo;
  decltype(std::declval<OrderInfoGroup>().emplace(orderInfo)) ret;
  {
    SPIN_LOCK(mtxOrderInfoGroup_);
    ret = orderInfoGroup_->emplace(orderInfoClone);
  }
  if (!ret.second) {
    LOG_W(
        "Add order info to order info group failed, may be key duplicated. {}",
        orderInfo->toShortStr());
    return SCODE_ORD_MGR_ADD_ORDER_INFO_FAILED;
  }
  return 0;
}

int OrdMgr::remove(OrderId orderId, LockFunc lockFunc) {
  {
    SPIN_LOCK(mtxOrderInfoGroup_);
    auto& idx = orderInfoGroup_->get<TagOrderId>();
    const auto iter = idx.find(orderId);
    if (iter != std::end(idx)) {
      LOG_D("Remove order info in order info group. {}", (*iter)->toShortStr());
      idx.erase(iter);
      return 0;
    }
  }
  LOG_W(
      "Remove order info in order info group failed "
      "because of no order info of order id {} in order info group. ",
      orderId);

  return SCODE_ORD_MGR_REMOVE_ORDER_INFO_FAILED;
}

std::tuple<int, OrderInfoSPtr> OrdMgr::get(OrderId orderId, DeepClone deepClone,
                                           LockFunc lockFunc) {
  {
    SPIN_LOCK(mtxOrderInfoGroup_);
    auto& idx = orderInfoGroup_->get<TagOrderId>();
    const auto iter = idx.find(orderId);
    if (iter != std::end(idx)) {
      const auto orderInfo = deepClone == DeepClone::True
                                 ? std::make_shared<OrderInfo>(**iter)
                                 : *iter;
      return {0, orderInfo};
    }
  }
  LOG_W(
      "Get order info from order info group failed "
      "because of no order info of order id {} in order info group. ",
      orderId);

  return {SCODE_ORD_MGR_CAN_NOT_FIND_ORDER, nullptr};
}

std::tuple<int, OrderInfoSPtr> OrdMgr::get(MarketCode marketCode,
                                           ExchOrderId exchOrderId,
                                           DeepClone deepClone,
                                           LockFunc lockFunc) {
  {
    SPIN_LOCK(mtxOrderInfoGroup_);
    auto& idx = orderInfoGroup_->get<TagMarketCodeExchOrderId>();
    const auto iter = idx.find(std::make_tuple(marketCode, exchOrderId));
    if (iter != std::end(idx)) {
      const auto orderInfo = deepClone == DeepClone::True
                                 ? std::make_shared<OrderInfo>(**iter)
                                 : *iter;
      return {0, orderInfo};
    }
  }
  LOG_W(
      "Get order info from order info group failed "
      "because of no order info of {} - {} in order info group. ",
      GetMarketName(marketCode), exchOrderId);

  return {0, nullptr};
}

std::vector<OrderInfoSPtr> OrdMgr::getOrderInfoGroup(
    std::uint32_t secAgoTheOrderNeedToBeSynced, DeepClone deepClone,
    LockFunc lockFunc) const {
  const auto now = GetTotalUSSince1970();
  std::vector<OrderInfoSPtr> ret;
  {
    SPIN_LOCK(mtxOrderInfoGroup_);
    for (const auto& rec : *orderInfoGroup_) {
      const auto td = now - rec->orderTime_;
      if (td > secAgoTheOrderNeedToBeSynced * 1000 * 1000) {
        ret.emplace_back(deepClone == DeepClone::True
                             ? std::make_shared<OrderInfo>(*rec)
                             : rec);
      }
    }
  }
  return ret;
}

std::tuple<IsSomeFieldOfOrderUpdated, OrderInfoSPtr>
OrdMgr::updateByOrderInfoFromExch(const OrderInfoSPtr& orderInfoFromExch,
                                  std::uint64_t noUsedToCalcPos,
                                  DeepClone deepClone, LockFunc lockFunc) {
  IsSomeFieldOfOrderUpdated isTheOrderInfoUpdated =
      IsSomeFieldOfOrderUpdated::False;
  {
    SPIN_LOCK(mtxOrderInfoGroup_);
    auto orderInfoInOrdMgr =
        getOrderInfo(orderInfoFromExch, DeepClone::False, LockFunc::False);
    if (!orderInfoInOrdMgr) {
      LOG_I(
          "Update by order info from exch failed, there may be unclosed orders "
          "that were closed during the process of sync unclosed orders. {}",
          orderInfoFromExch->toShortStr());
      return {isTheOrderInfoUpdated, nullptr};
    }

    isTheOrderInfoUpdated = orderInfoInOrdMgr->updateByOrderInfoFromExch(
        orderInfoFromExch, noUsedToCalcPos);
    if (orderInfoInOrdMgr->closed()) {
      remove(orderInfoInOrdMgr->orderId_, LockFunc::False);
    }

    const auto orderInfo = deepClone == DeepClone::True
                               ? std::make_shared<OrderInfo>(*orderInfoInOrdMgr)
                               : orderInfoInOrdMgr;

    return {isTheOrderInfoUpdated, orderInfo};
  }

  return {isTheOrderInfoUpdated, nullptr};
}

std::tuple<IsTheOrderCanBeUsedCalcPos, OrderInfoSPtr>
OrdMgr::updateByOrderInfoFromTDGW(const OrderInfoSPtr& orderInfoFromTDGW,
                                  LockFunc lockFunc) {
  {
    SPIN_LOCK(mtxOrderInfoGroup_);
    auto orderInfoInOrdMgr =
        getOrderInfo(orderInfoFromTDGW, DeepClone::False, LockFunc::False);
    if (!orderInfoInOrdMgr) {
      LOG_W(
          "Update by order info from tdsrv failed because of order info in "
          "ordmgr not exists. {}",
          orderInfoFromTDGW->toShortStr());
      return {IsTheOrderCanBeUsedCalcPos::False, nullptr};
    }

    const auto isTheOrderCanBeUsedCalcPos =
        orderInfoInOrdMgr->updateByOrderInfoFromTDGW(orderInfoFromTDGW);
    if (orderInfoInOrdMgr->closed()) {
      remove(orderInfoInOrdMgr->orderId_, LockFunc::False);
    }
    return {isTheOrderCanBeUsedCalcPos, orderInfoInOrdMgr};
  }

  return {IsTheOrderCanBeUsedCalcPos::False, nullptr};
}

OrderInfoSPtr OrdMgr::getOrderInfo(const OrderInfoSPtr& orderInfo,
                                   DeepClone deepClone, LockFunc lockFunc) {
  {
    SPIN_LOCK(mtxOrderInfoGroup_);

    if (orderInfo->orderId_ != 0) {
      auto& idx = orderInfoGroup_->get<TagOrderId>();
      const auto iter = idx.find(orderInfo->orderId_);
      if (iter != std::end(idx)) {
        const auto ret = deepClone == DeepClone::True
                             ? std::make_shared<OrderInfo>(**iter)
                             : *iter;
        return ret;
      }
    }

    if (orderInfo->marketCode_ != MarketCode::Others &&
        orderInfo->exchOrderId_ != 0) {
      auto& idx = orderInfoGroup_->get<TagMarketCodeExchOrderId>();
      const auto iter = idx.find(
          std::make_tuple(orderInfo->marketCode_, orderInfo->exchOrderId_));
      if (iter != std::end(idx)) {
        const auto ret = deepClone == DeepClone::True
                             ? std::make_shared<OrderInfo>(**iter)
                             : *iter;
        return ret;
      }
    }
  }

  LOG_W("Get order info by another order info failed. {}",
        orderInfo->toShortStr());
  return nullptr;
}

int OrdMgr::syncOrderGroupToDB() {
  std::vector<OrderInfoSPtr> orderGroupOfSyncToDB;
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxOrderGroupOfSyncToDB_);
    std::swap(orderGroupOfSyncToDB, orderGroupOfSyncToDB_);
  }

  for (const auto& orderInfo : orderGroupOfSyncToDB) {
    const auto identity = GET_RAND_STR();
    const auto sql = orderInfo->getSqlOfUSPOrderInfoUpdate();
    auto [ret, execRet] = dbEng_->asyncExec(identity, sql);
    if (ret != 0) {
      LOG_W("Sync order info to db failed. [{}]", sql);
    }
  }

  return 0;
}

void OrdMgr::cacheOrderOfSyncToDB(const OrderInfoSPtr& orderInfo) {
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxOrderGroupOfSyncToDB_);
    orderGroupOfSyncToDB_.emplace_back(orderInfo);
  }
}

}  // namespace bq
