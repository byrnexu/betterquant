/*!
 * \file TBLAssetInfo.hpp
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

namespace bq::db::assetInfo {

struct FieldGroupOfKey {
  AcctId acctId;
  std::string marketCode;
  std::string symbolType;
  std::string assetName;
  JSER(FieldGroupOfKey, acctId, marketCode, symbolType, assetName)
};

struct FieldGroupOfVal {
  std::string vol;
  std::string crossVol;
  std::string frozen;
  std::string available;
  std::string pnlUnreal;
  std::string maxWithdraw;
  std::string updateTime;

  JSER(FieldGroupOfVal, vol, crossVol, frozen, available, pnlUnreal,
       maxWithdraw, updateTime)
};

struct FieldGroupOfAll {
  AcctId acctId;
  std::string marketCode;
  std::string symbolType;
  std::string assetName;

  std::string vol;
  std::string crossVol;
  std::string frozen;
  std::string available;
  std::string pnlUnreal;
  std::string maxWithdraw;
  std::string updateTime;

  JSER(FieldGroupOfAll, acctId, marketCode, symbolType, assetName, vol,
       crossVol, frozen, available, pnlUnreal, maxWithdraw, updateTime)
};

struct TableSchema {
  inline const static std::string TableName = "`BetterQuant`.`assetInfo`";
  using KeyFields = FieldGroupOfKey;
  using ValFields = FieldGroupOfVal;
  using AllFields = FieldGroupOfAll;
};

using Record = FieldGroupOfAll;
using RecordSPtr = std::shared_ptr<Record>;
using RecordWPtr = std::weak_ptr<Record>;

}  // namespace bq::db::assetInfo

using TBLAssetInfo = bq::db::assetInfo::TableSchema;
