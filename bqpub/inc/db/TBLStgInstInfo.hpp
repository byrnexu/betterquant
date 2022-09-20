/*!
 * \file TBLStgInstInfo.hpp
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

namespace bq::db::stgInstInfo {

struct FieldGroupOfKey {
  std::uint32_t stgId;
  std::uint32_t stgInstId;
  JSER(FieldGroupOfKey, stgId, stgInstId)
};

struct FieldGroupOfVal {
  std::string stgInstParams;
  JSER(FieldGroupOfVal, stgInstParams)
};

struct FieldGroupOfAll {
  std::uint32_t stgId;
  std::string stgName;
  std::uint32_t userIdOfAuthor;
  std::uint32_t stgInstId;
  std::string stgInstParams;
  std::string stgInstName;
  std::uint32_t userId;
  int isDel;
  JSER(FieldGroupOfAll, stgId, stgName, userIdOfAuthor, stgInstId,
       stgInstParams, stgInstName, userId, isDel)
};

struct TableSchema {
  inline const static std::string TableName = "stgInstInfo";
  using KeyFields = FieldGroupOfKey;
  using ValFields = FieldGroupOfVal;
  using AllFields = FieldGroupOfAll;
};

using Record = FieldGroupOfAll;
using RecordSPtr = std::shared_ptr<Record>;
using RecordWPtr = std::weak_ptr<Record>;

}  // namespace bq::db::stgInstInfo

using TBLStgInstInfo = bq::db::stgInstInfo::TableSchema;
