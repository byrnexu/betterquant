/*!
 * \file TBLPosInfo.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "def/BQDef.hpp"
#include "def/Def.hpp"
#include "util/Pch.hpp"

namespace bq::db::posInfo {

struct FieldGroupOfKey {
  ProductId productId;
  UserId userId;
  AcctId acctId;
  StgId stgId;
  StgInstId stgInstId;
  AlgoId algoId;
  std::string marketCode;
  std::string symbolType;
  std::string symbolCode;
  std::string side;
  std::string posSide;
  std::uint32_t parValue;
  std::string feeCurrency;

  JSER(FieldGroupOfKey, productId, userId, acctId, stgId, stgInstId, algoId,
       marketCode, symbolType, symbolCode, side, posSide, parValue, feeCurrency)
};

struct FieldGroupOfVal {
  std::string fee;
  std::string pos;
  std::string prePos;
  std::string avgOpenPrice;
  std::string pnlUnReal;
  std::string pnlReal;
  std::string totalBidSize;
  std::string totalAskSize;
  std::string updateTime;

  JSER(FieldGroupOfVal, fee, pos, prePos, avgOpenPrice, pnlUnReal, pnlReal,
       totalBidSize, totalAskSize, updateTime)
};

struct FieldGroupOfAll {
  ProductId productId;
  UserId userId;
  AcctId acctId;
  StgId stgId;
  StgInstId stgInstId;
  AlgoId algoId;
  std::string marketCode;
  std::string symbolType;
  std::string symbolCode;
  std::string side;
  std::string posSide;
  std::uint32_t parValue;
  std::string feeCurrency;

  std::string fee;
  std::string pos;
  std::string prePos;
  std::string avgOpenPrice;
  std::string pnlUnReal;
  std::string pnlReal;
  std::string totalBidSize;
  std::string totalAskSize;
  std::string updateTime;

  JSER(FieldGroupOfAll, productId, userId, acctId, stgId, stgInstId, algoId,
       marketCode, symbolType, symbolCode, side, posSide, parValue, feeCurrency,
       fee, pos, prePos, avgOpenPrice, pnlUnReal, pnlReal, totalBidSize,
       totalAskSize, updateTime)
};

struct TableSchema {
  inline const static std::string TableName = "`BetterQuant`.`posInfo`";
  using KeyFields = FieldGroupOfKey;
  using ValFields = FieldGroupOfVal;
  using AllFields = FieldGroupOfAll;
};

using Record = FieldGroupOfAll;
using RecordSPtr = std::shared_ptr<Record>;
using RecordWPtr = std::weak_ptr<Record>;

}  // namespace bq::db::posInfo

using TBLPosInfo = bq::db::posInfo::TableSchema;
