#pragma once

#include "def/BQConst.hpp"
#include "def/BQDef.hpp"
#include "def/Const.hpp"
#include "def/DataStruOfTD.hpp"
#include "def/Def.hpp"
#include "util/Pch.hpp"

namespace bq::db {
class DBEng;
using DBEngSPtr = std::shared_ptr<DBEng>;
}  // namespace bq::db

namespace bq {

class OrdMgr {
  struct TagOrderId {};
  struct KeyOrderId : boost::multi_index::composite_key<
                          OrderInfo, MIDX_MEMER(OrderInfo, OrderId, orderId_)> {
  };
  using MIdxOrderId = boost::multi_index::ordered_unique<
      boost::multi_index::tag<TagOrderId>, KeyOrderId,
      boost::multi_index::composite_key_result_less<KeyOrderId::result_type>>;

  struct TagMarketCodeExchOrderId {};
  struct KeyMarketCodeExchOrderId
      : boost::multi_index::composite_key<
            OrderInfo, MIDX_MEMER(OrderInfo, MarketCode, marketCode_),
            MIDX_MEMER(OrderInfo, ExchOrderId, exchOrderId_)> {};
  using MIdxMarketCodeExchOrderId = boost::multi_index::ordered_non_unique<
      boost::multi_index::tag<TagMarketCodeExchOrderId>,
      KeyMarketCodeExchOrderId,
      boost::multi_index::composite_key_result_less<
          KeyMarketCodeExchOrderId ::result_type>>;

  using OrderInfoGroup = boost::multi_index::multi_index_container<
      OrderInfoSPtr,
      boost::multi_index::indexed_by<MIdxOrderId, MIdxMarketCodeExchOrderId>>;
  using OrderInfoGroupSPtr = std::shared_ptr<OrderInfoGroup>;

 public:
  OrdMgr(const OrdMgr&) = delete;
  OrdMgr& operator=(const OrdMgr&) = delete;
  OrdMgr(const OrdMgr&&) = delete;
  OrdMgr& operator=(const OrdMgr&&) = delete;

  OrdMgr();

 public:
  int init(const YAML::Node& node, const db::DBEngSPtr& dbEng,
           const std::string& sql);

 private:
  int initOrderInfoGroup(const std::string& sql);

 public:
  int add(const OrderInfoSPtr& orderInfo, DeepClone deepClone,
          LockFunc lockFunc = LockFunc::True);

  int remove(OrderId orderId, LockFunc lockFunc = LockFunc::True);

  std::tuple<int, OrderInfoSPtr> get(OrderId orderId, DeepClone deepClone,
                                     LockFunc lockFunc = LockFunc::True);

  std::tuple<int, OrderInfoSPtr> get(MarketCode marketCode,
                                     ExchOrderId exchOrderId,
                                     DeepClone deepClone,
                                     LockFunc lockFunc = LockFunc::True);

  std::vector<OrderInfoSPtr> getOrderInfoGroup(
      std::uint32_t secAgoTheOrderNeedToBeSynced,
      DeepClone deepClone = DeepClone::True,
      LockFunc lockFunc = LockFunc::True) const;

  std::tuple<IsSomeFieldOfOrderUpdated, OrderInfoSPtr>
  updateByOrderInfoFromExch(const OrderInfoSPtr& orderInfoFromExch,
                            std::uint64_t noUsedToCalcPos, DeepClone deepClone,
                            LockFunc lockFunc = LockFunc::True);

  std::tuple<IsTheOrderCanBeUsedCalcPos, OrderInfoSPtr>
  updateByOrderInfoFromTDGW(const OrderInfoSPtr& orderInfoFromTDGW,
                            LockFunc lockFunc = LockFunc::True);

 private:
  OrderInfoSPtr getOrderInfo(const OrderInfoSPtr& orderInfo,
                             DeepClone deepClone,
                             LockFunc lockFunc = LockFunc::True);

 public:
  void cacheOrderOfSyncToDB(const OrderInfoSPtr& orderInfo);
  int syncOrderGroupToDB();

 public:
  YAML::Node& getNode() { return node_; }

 private:
  YAML::Node node_;

  db::DBEngSPtr dbEng_{nullptr};

  OrderInfoGroupSPtr orderInfoGroup_{nullptr};
  mutable std::ext::spin_mutex mtxOrderInfoGroup_;

  std::vector<OrderInfoSPtr> orderGroupOfSyncToDB_;
  mutable std::ext::spin_mutex mtxOrderGroupOfSyncToDB_;
};

}  // namespace bq
