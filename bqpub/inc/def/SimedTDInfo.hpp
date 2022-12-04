/*!
 * \file SimedTDInfo.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/12/01
 *
 * \brief
 */

#pragma once

#include "def/BQConstIF.hpp"
#include "def/BQDefIF.hpp"

namespace bq {

inline Decimal Slippage(Decimal value) { return value; }
inline Decimal FilledPer(Decimal value) { return value; }

const std::string SEP_OF_TRANS_DETAIL_REC = ";";
const std::string SEP_OF_TRANS_DETAIL_FIELD = ",";

struct TransDetail {
  TransDetail() = default;
  TransDetail(Decimal slippage, Decimal filledPer,
              LiquidityDirection liquidityDirection)
      : slippage_(slippage),
        filledPer_(filledPer),
        liquidityDirection_(liquidityDirection) {}
  Decimal slippage_{0};
  Decimal filledPer_{1};
  LiquidityDirection liquidityDirection_{LiquidityDirection::Taker};
  std::string toStr() const;
};
using TransDetailSPtr = std::shared_ptr<TransDetail>;
using TransDetailGroup = std::vector<TransDetailSPtr>;

TransDetailSPtr MakeTransDetail(const std::string& transDetailInJsonFmt);

struct SimedTDInfo {
  SimedTDInfo() = default;
  SimedTDInfo(
      OrderStatus orderStatus,
      const std::vector<std::tuple<Decimal, Decimal, LiquidityDirection>>&
          transDetail);
  SimedTDInfo(OrderStatus orderStatus,
              const TransDetailGroup& transDetailGroup);
  OrderStatus orderStatus_{OrderStatus::Filled};
  TransDetailGroup transDetailGroup_;
};
using SimedTDInfoSPtr = std::shared_ptr<SimedTDInfo>;

std::string MakeSimedTDInfoInJsonFmt(
    OrderStatus orderStatus,
    const std::vector<std::tuple<Decimal, Decimal, LiquidityDirection>>&
        transDetail);

std::tuple<int, SimedTDInfoSPtr> MakeSimedTDInfo(
    const std::string& simedTDInfoInJsonFmt);

int checkIfSimedTDInfoValid(const SimedTDInfoSPtr& simedTDInfo);

std::string ConvertSimedTDInfoToJsonFmt(const SimedTDInfoSPtr& simedTDInfo);

}  // namespace bq
