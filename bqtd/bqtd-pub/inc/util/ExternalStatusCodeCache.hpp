/*!
 * \file ExternalStatusCodeCache.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "db/DBEngDef.hpp"
#include "db/TBLExternalStatusCode.hpp"
#include "def/Const.hpp"
#include "def/Def.hpp"
#include "util/Pch.hpp"

namespace bq::db::externalStatusCode {
struct FieldGroupOfAll;
using Record = FieldGroupOfAll;
using RecordSPtr = std::shared_ptr<Record>;
}  // namespace bq::db::externalStatusCode

namespace bq::td {

using Identity2ExternalStatusCode =
    absl::node_hash_map<std::uint64_t, db::externalStatusCode::RecordSPtr>;
using Identity2ExternalStatusCodeSPtr =
    std::shared_ptr<Identity2ExternalStatusCode>;

class ExternalStatusCodeCache;
using ExternalStatusCodeCacheSPtr = std::shared_ptr<ExternalStatusCodeCache>;

class ExternalStatusCodeCache {
 public:
  ExternalStatusCodeCache(const ExternalStatusCodeCache&) = delete;
  ExternalStatusCodeCache& operator=(const ExternalStatusCodeCache&) = delete;
  ExternalStatusCodeCache(const ExternalStatusCodeCache&&) = delete;
  ExternalStatusCodeCache& operator=(const ExternalStatusCodeCache&&) = delete;

  explicit ExternalStatusCodeCache(const db::DBEngSPtr& dbEng);

 public:
  int load(const std::string& marketCode, const std::string& symbolType);
  int reload(const std::string& marketCode, const std::string& symbolType) {
    return load(marketCode, symbolType);
  }

  std::tuple<int, bool> add(db::externalStatusCode::RecordSPtr& rec,
                            SyncToDB syncToDB = SyncToDB::True);

  int getAndSetStatusCodeIfNotExists(const std::string& marketCode,
                                     const std::string& symbolType,
                                     const std::string& externalStatusCode,
                                     const std::string& externalStatusMsg,
                                     int defaultValue = 0);

 private:
  db::DBEngSPtr dbEng_;

  Identity2ExternalStatusCodeSPtr identity2ExternalStatusCode_{nullptr};
  mutable std::ext::spin_mutex mtxIdentity2ExternalStatusCode_;
};

}  // namespace bq::td
