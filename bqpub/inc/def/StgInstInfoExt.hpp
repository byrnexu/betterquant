#pragma once

#include "util/PchBase.hpp"

namespace bq::db::stgInstInfo {
struct FieldGroupOfAll;
using Record = FieldGroupOfAll;
using RecordSPtr = std::shared_ptr<Record>;
using RecordWPtr = std::weak_ptr<Record>;
}  // namespace bq::db::stgInstInfo

namespace bq {

struct StgInstInfo;
using StgInstInfoSPtr = std::shared_ptr<StgInstInfo>;

StgInstInfoSPtr MakeStgInstInfo(
    const db::stgInstInfo::RecordSPtr& recStgInstInfo);

}  // namespace bq
