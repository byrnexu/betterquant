/*!
 * \file PosMgr.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "def/Const.hpp"
#include "def/DataStruOfAssets.hpp"
#include "def/Def.hpp"
#include "util/Pch.hpp"
#include "util/StdExt.hpp"

namespace bq::db {
class DBEng;
using DBEngSPtr = std::shared_ptr<DBEng>;
}  // namespace bq::db

namespace bq {
struct OrderInfo;
using OrderInfoSPtr = std::shared_ptr<OrderInfo>;
}  // namespace bq

namespace bq {

class PosMgr {
  struct TagMain {};
  struct KeyMain : boost::multi_index::composite_key<
                       PosInfo, MIDX_MEMER(PosInfo, std::uint64_t, keyHash_)> {
  };
  using MIdxMain = boost::multi_index::ordered_unique<
      boost::multi_index::tag<TagMain>, KeyMain,
      boost::multi_index::composite_key_result_less<KeyMain::result_type>>;

  using PosInfoTable = boost::multi_index::multi_index_container<
      PosInfoSPtr, boost::multi_index::indexed_by<MIdxMain>>;
  using PosInfoTableSPtr = std::shared_ptr<PosInfoTable>;

 public:
  PosMgr(const PosMgr&) = delete;
  PosMgr& operator=(const PosMgr&) = delete;
  PosMgr(const PosMgr&&) = delete;
  PosMgr& operator=(const PosMgr&&) = delete;

  PosMgr();

 public:
  int init(const YAML::Node& node, const db::DBEngSPtr& dbEng,
           const std::string& sql);

  void setSyncToDB(SyncToDB value) { syncToDB_ = value; }

 private:
  int initPosInfoTable(const std::string& sql);

 public:
  PosChgInfoSPtr updateByOrderInfoFromTDGW(const OrderInfoSPtr& orderInfo,
                                           LockFunc lockFunc = LockFunc::False);

 private:
  PosChgInfoSPtr updateByOrderInfo(const OrderInfoSPtr& orderInfo);

  PosChgInfoSPtr updateByOrderInfoSinglePosSide(const OrderInfoSPtr& orderInfo);
  PosChgInfoSPtr updateByOrderInfoSinglePosSideOfBid(
      const OrderInfoSPtr& orderInfo);
  PosChgInfoSPtr updateByOrderInfoSinglePosSideOfAsk(
      const OrderInfoSPtr& orderInfo);

  PosChgInfoSPtr updateByOrderInfoDoublePosSide(const OrderInfoSPtr& orderInfo);
  PosChgInfoSPtr updateByOrderInfoDoublePosSideOfBidLong(
      const OrderInfoSPtr& orderInfo);
  PosChgInfoSPtr updateByOrderInfoDoublePosSideOfAskLong(
      const OrderInfoSPtr& orderInfo);
  PosChgInfoSPtr updateByOrderInfoDoublePosSideOfAskShort(
      const OrderInfoSPtr& orderInfo);
  PosChgInfoSPtr updateByOrderInfoDoublePosSideOfBidShort(
      const OrderInfoSPtr& orderInfo);

 private:
  Decimal recalcAvgOpenPrice(const OrderInfoSPtr& orderInfo,
                             const PosInfoSPtr& origPosInfo, Decimal newPos);

  Decimal calcCurPnlRealOfCloseShort(const OrderInfoSPtr& orderInfo,
                                     const PosInfoSPtr& origPosInfo,
                                     Decimal newPos);

  Decimal calcCurPnlRealOfCloseLong(const OrderInfoSPtr& orderInfo,
                                    const PosInfoSPtr& origPosInfo,
                                    Decimal newPos);

 public:
  PosInfoGroup getPosInfoGroup(LockFunc lockFunc) const;

 public:
  std::string toStr() const;

 public:
  void syncToDB(const PosInfoSPtr& posInfo);

 public:
  YAML::Node& getNode() { return node_; }

 private:
  YAML::Node node_;

  db::DBEngSPtr dbEng_{nullptr};
  SyncToDB syncToDB_{SyncToDB::True};

  PosInfoTableSPtr posInfoTable_{nullptr};
  mutable std::ext::spin_mutex mtxPosInfoTable_;
};

}  // namespace bq
