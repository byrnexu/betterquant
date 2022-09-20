/*!
 * \file AcctInfoCache.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "db/DBEngDef.hpp"
#include "def/BQConst.hpp"
#include "def/BQDef.hpp"
#include "def/Const.hpp"
#include "def/Def.hpp"
#include "util/Pch.hpp"

namespace bq::db::acctInfo {
struct FieldGroupOfAll;
using Record = FieldGroupOfAll;
using RecordSPtr = std::shared_ptr<Record>;
}  // namespace bq::db::acctInfo

namespace bq {

struct AcctInfo;
using AcctInfoSPtr = std::shared_ptr<AcctInfo>;

using AcctId2AcctInfo = absl::node_hash_map<AcctId, AcctInfoSPtr>;
using AcctId2AcctInfoSPtr = std::shared_ptr<AcctId2AcctInfo>;

class AcctInfoCache;
using AcctInfoCacheSPtr = std::shared_ptr<AcctInfoCache>;

class AcctInfoCache {
 public:
  AcctInfoCache(const AcctInfoCache&) = delete;
  AcctInfoCache& operator=(const AcctInfoCache&) = delete;
  AcctInfoCache(const AcctInfoCache&&) = delete;
  AcctInfoCache& operator=(const AcctInfoCache&&) = delete;

  explicit AcctInfoCache(const db::DBEngSPtr& dbEng);

 public:
  int load();
  int reload() { return load(); }

  std::tuple<int, MarketCode, SymbolType> getMarketCodeAndSymbolType(
      AcctId acctId);

 private:
  db::DBEngSPtr dbEng_;

  AcctId2AcctInfoSPtr acctId2AcctInfo_{nullptr};
  mutable std::ext::spin_mutex mtxAcctId2AcctInfo_;
};

}  // namespace bq
