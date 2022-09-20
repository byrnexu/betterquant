/*!
 * \file PosMgrRestorer.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "def/BQDef.hpp"
#include "util/Pch.hpp"

namespace bq {
struct OrderInfo;
using OrderInfoSPtr = std::shared_ptr<OrderInfo>;
}  // namespace bq

namespace bq::td::srv {

class TDSrv;

class PosMgrRestorer {
 public:
  PosMgrRestorer(const PosMgrRestorer&) = delete;
  PosMgrRestorer& operator=(const PosMgrRestorer&) = delete;
  PosMgrRestorer(const PosMgrRestorer&&) = delete;
  PosMgrRestorer& operator=(const PosMgrRestorer&&) = delete;

  explicit PosMgrRestorer(TDSrv* tdSrv);

 public:
  int exec();

 private:
  std::tuple<int, std::map<AcctId, std::uint64_t>>
  makeAcctId2lastNoUsedToCalcPos();
  std::tuple<int, std::map<AcctId, std::uint64_t>> queryAllAcctId();
  std::tuple<int, std::map<AcctId, std::uint64_t>>
  queryAcctId2lastNoUsedToCalcPos();

  std::tuple<int, std::map<AcctId, std::map<std::uint64_t, OrderInfoSPtr>>>
  getAcct2OrderInfoForPosMgrNotCalc(
      const std::map<AcctId, std::uint64_t>& acctId2lastNoUsedToCalcPos);

  void restorePosMgr(
      const std::map<AcctId, std::map<std::uint64_t, OrderInfoSPtr>>&
          acctId2OrderInfoForPosNotCalc);

 private:
  TDSrv* tdSrv_;
};

}  // namespace bq::td::srv
