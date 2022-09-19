#pragma once

#include "TDSvcDef.hpp"
#include "def/BQDef.hpp"
#include "util/Pch.hpp"

namespace bq {
struct AssetInfo;
using AssetInfoSPtr = std::shared_ptr<AssetInfo>;

struct UpdateInfoOfAssetGroup;
using UpdateInfoOfAssetGroupSPtr = std::shared_ptr<UpdateInfoOfAssetGroup>;

struct OrderInfo;
using OrderInfoSPtr = std::shared_ptr<OrderInfo>;

class SHMCli;
using SHMCliSPtr = std::shared_ptr<SHMCli>;
}  // namespace bq

namespace bq::db::trdSymbol {
struct FieldGroupOfAll;
using Record = FieldGroupOfAll;
using RecordSPtr = std::shared_ptr<Record>;
using RecordWPtr = std::weak_ptr<Record>;
}  // namespace bq::db::trdSymbol

namespace bq::td::svc {

ApiInfoSPtr GetApiInfo(const std::string& acctData);

SHMIPCAsyncTaskSPtr MakeTDSrvSignal(MsgId msgId,
                                    const std::any& arg = std::any());

void NotifyAssetInfo(const SHMCliSPtr& shmCli, AcctId acctId,
                     const UpdateInfoOfAssetGroupSPtr& updateInfoOfAssetGroup);

}  // namespace bq::td::svc
