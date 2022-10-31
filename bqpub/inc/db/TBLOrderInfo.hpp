/*!
 * \file TBLOrderInfo.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "def/BQConst.hpp"
#include "def/Def.hpp"
#include "util/Pch.hpp"

namespace bq::db::orderInfo {

struct FieldGroupOfKey {
  std::string orderId;
  JSER(FieldGroupOfKey, orderId)
};

struct FieldGroupOfVal {
  std::string exchOrderId;

  std::string fee;
  std::string feeCurrency;

  std::string dealSize;
  std::string avgDealPrice;

  std::string lastTradeId;
  std::string lastDealPrice;
  std::string lastDealSize;
  std::string lastDealTime;

  std::uint8_t orderStatus;
  std::uint64_t noUsedToCalcPos;
  std::int32_t statusCode;

  JSER(FieldGroupOfVal, exchOrderId, fee, feeCurrency, dealSize, avgDealPrice,
       lastTradeId, lastDealPrice, lastDealSize, lastDealTime, orderStatus,
       noUsedToCalcPos, statusCode)
};

struct FieldGroupOfAll {
  ProductId productId;
  UserId userId;
  AcctId acctId;

  StgId stgId;
  StgInstId stgInstId;
  AlgoId algoId;

  std::string orderId;
  std::string exchOrderId;
  std::string parentOrderId;

  std::string marketCode;
  std::string symbolType;
  std::string symbolCode;
  std::string exchSymbolCode;

  std::string side;
  std::string posSide;

  std::string orderPrice;
  std::string orderSize;
  std::uint32_t parValue;

  std::string orderType;
  std::string orderTypeExtra;
  std::string orderTime;

  std::string fee;
  std::string feeCurrency;

  std::string dealSize;
  std::string avgDealPrice;

  std::string lastTradeId;
  std::string lastDealPrice;
  std::string lastDealSize;
  std::string lastDealTime;

  std::uint8_t orderStatus;
  std::uint64_t noUsedToCalcPos;
  std::int32_t statusCode;

  JSER(FieldGroupOfAll, productId, userId, acctId, stgId, stgInstId, algoId,
       orderId, exchOrderId, parentOrderId, marketCode, symbolType, symbolCode,
       exchSymbolCode, side, posSide, orderPrice, orderSize, parValue,
       orderType, orderTypeExtra, orderTime, fee, feeCurrency, dealSize,
       avgDealPrice, lastTradeId, lastDealPrice, lastDealSize, lastDealTime,
       orderStatus, noUsedToCalcPos, statusCode)
};

struct TableSchema {
  inline const static std::string TableName = "`BetterQuant`.`orderInfo`";
  using KeyFields = FieldGroupOfKey;
  using ValFields = FieldGroupOfVal;
  using AllFields = FieldGroupOfAll;
};

using Record = FieldGroupOfAll;
using RecordSPtr = std::shared_ptr<Record>;
using RecordWPtr = std::weak_ptr<Record>;

}  // namespace bq::db::orderInfo

using TBLOrderInfo = bq::db::orderInfo::TableSchema;
