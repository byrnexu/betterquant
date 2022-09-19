#pragma once

#include "def/BQConst.hpp"
#include "util/Pch.hpp"

namespace bq::db::posInfo {
struct FieldGroupOfAll;
using Record = FieldGroupOfAll;
using RecordSPtr = std::shared_ptr<Record>;
using RecordWPtr = std::weak_ptr<Record>;
}  // namespace bq::db::posInfo

namespace bq {
struct OrderInfo;
using OrderInfoSPtr = std::shared_ptr<OrderInfo>;
}  // namespace bq

namespace bq {

struct PosInfo;
using PosInfoSPtr = std::shared_ptr<PosInfo>;

using PosChgInfo = std::vector<PosInfoSPtr>;
using PosChgInfoSPtr = std::shared_ptr<PosChgInfo>;

PosInfoSPtr MakePosInfo(const db::posInfo::RecordSPtr& recPosInfo);
PosInfoSPtr MakePosInfoOfContract(const OrderInfoSPtr& orderInfo);
PosInfoSPtr MakePosInfoOfContractWithKeyFields(const OrderInfoSPtr& orderInfo,
                                               Side side = Side::Others);

}  // namespace bq
