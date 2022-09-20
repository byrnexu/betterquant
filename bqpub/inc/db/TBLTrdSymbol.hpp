/*!
 * \file TBLTrdSymbol.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "def/Def.hpp"
#include "util/Pch.hpp"

namespace bq::db::trdSymbol {

struct FieldGroupOfKey {
  std::string marketCode;
  std::string symbolType;
  std::uint32_t acctId;
  std::string symbolCode;
  std::string exchSymbolCode;
  int isDel;

  JSER(FieldGroupOfKey, marketCode, symbolType, acctId, symbolCode,
       exchSymbolCode)
};

struct FieldGroupOfVal {
  int isDel;

  JSER(FieldGroupOfVal, isDel)
};

struct FieldGroupOfAll {
  std::string marketCode;
  std::string symbolType;
  std::uint32_t acctId;
  std::string symbolCode;
  std::string exchSymbolCode;
  int isDel;

  JSER(FieldGroupOfAll, marketCode, symbolType, acctId, symbolCode,
       exchSymbolCode, isDel)
};

struct TableSchema {
  inline const static std::string TableName = "`BetterQuant`.`trdSymbol`";
  using KeyFields = FieldGroupOfKey;
  using ValFields = FieldGroupOfVal;
  using AllFields = FieldGroupOfAll;
};

using Record = FieldGroupOfAll;
using RecordSPtr = std::shared_ptr<Record>;
using RecordWPtr = std::weak_ptr<Record>;

}  // namespace bq::db::trdSymbol

using TBLTrdSymbol = bq::db::trdSymbol::TableSchema;
