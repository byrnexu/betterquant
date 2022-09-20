/*!
 * \file TBLExternalStatusCode.hpp
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

namespace bq::db::externalStatusCode {

struct FieldGroupOfKey {
  std::string marketCode;
  std::string symbolType;
  std::string externalStatusCode;

  JSER(FieldGroupOfKey, marketCode, symbolType, externalStatusCode)
};

struct FieldGroupOfVal {
  std::string externalStatusMsg;
  int statusCode;

  JSER(FieldGroupOfVal, externalStatusMsg, statusCode)
};

struct FieldGroupOfAll {
  std::string marketCode;
  std::string symbolType;
  std::string externalStatusCode;
  std::string externalStatusMsg;
  int statusCode;

  JSER(FieldGroupOfAll, marketCode, symbolType, externalStatusCode,
       externalStatusMsg, statusCode)
};

struct TableSchema {
  inline const static std::string TableName =
      "`BetterQuant`.`externalStatusCode`";
  using KeyFields = FieldGroupOfKey;
  using ValFields = FieldGroupOfVal;
  using AllFields = FieldGroupOfAll;
};

using Record = FieldGroupOfAll;
using RecordSPtr = std::shared_ptr<Record>;
using RecordWPtr = std::weak_ptr<Record>;

}  // namespace bq::db::externalStatusCode

using TBLExternalStatusCode = bq::db::externalStatusCode::TableSchema;
