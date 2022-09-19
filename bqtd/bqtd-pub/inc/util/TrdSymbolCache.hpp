#pragma once

#include "db/DBEngDef.hpp"
#include "def/BQDef.hpp"
#include "def/Const.hpp"
#include "def/DataStruOfTD.hpp"
#include "def/Def.hpp"
#include "util/Pch.hpp"

namespace bq::db::trdSymbol {
struct FieldGroupOfAll;
using Record = FieldGroupOfAll;
using RecordSPtr = std::shared_ptr<Record>;
}  // namespace bq::db::trdSymbol

namespace bq {
struct OrderInfo;
using OrderInfoSPtr = std::shared_ptr<OrderInfo>;

}  // namespace bq

namespace bq::td {

using Identity2TrdSymbol =
    absl::node_hash_map<std::uint64_t, db::trdSymbol::RecordSPtr>;
using Identity2TrdSymbolSPtr = std::shared_ptr<Identity2TrdSymbol>;

class TrdSymbolCache;
using TrdSymbolCacheSPtr = std::shared_ptr<TrdSymbolCache>;

class TrdSymbolCache {
 public:
  TrdSymbolCache(const TrdSymbolCache&) = delete;
  TrdSymbolCache& operator=(const TrdSymbolCache&) = delete;
  TrdSymbolCache(const TrdSymbolCache&&) = delete;
  TrdSymbolCache& operator=(const TrdSymbolCache&&) = delete;

  explicit TrdSymbolCache(const db::DBEngSPtr& dbEng);

 public:
  int load(const std::string& marketCode, const std::string& symbolType,
           AcctId acctId);

  std::tuple<int, bool> add(const OrderInfoSPtr& orderInfo,
                            SyncToDB syncToDB = SyncToDB::True);

  Identity2TrdSymbolSPtr getTrdSymbolGroup() const;

 private:
  db::DBEngSPtr dbEng_;

  Identity2TrdSymbolSPtr identity2TrdSymbol_{nullptr};
  mutable std::ext::spin_mutex mtxIdentity2TrdSymbol_;
};

}  // namespace bq::td
