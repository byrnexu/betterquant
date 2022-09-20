/*!
 * \file TDSvcUtil.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "TDSvcUtil.hpp"

#include "SHMIPC.hpp"
#include "db/TBLTrdSymbol.hpp"
#include "def/AssetInfo.hpp"
#include "def/DataStruOfOthers.hpp"
#include "util/TaskDispatcher.hpp"

namespace bq::td::svc {

ApiInfoSPtr GetApiInfo(const std::string& acctData) {
  ApiInfoSPtr ret = std::make_shared<ApiInfo>();
  std::vector<std::string> fieldGroup;
  boost::split(fieldGroup, acctData, boost::is_any_of(";"));
  ret->apiKey_ = fieldGroup[0];
  ret->secKey_ = fieldGroup[1];
  ret->password_ = fieldGroup[2];
  return ret;
}

SHMIPCAsyncTaskSPtr MakeTDSrvSignal(MsgId msgId, const std::any& arg) {
  TDSrvSignal tdSrvSignal(msgId);
  const auto task =
      std::make_shared<SHMIPCTask>(&tdSrvSignal, sizeof(TDSrvSignal));
  const auto ret = std::make_shared<SHMIPCAsyncTask>(task, arg);
  return ret;
}

void NotifyAssetInfo(const SHMCliSPtr& shmCli, AcctId acctId,
                     const UpdateInfoOfAssetGroupSPtr& updateInfoOfAssetGroup) {
  const auto extDataLen =
      updateInfoOfAssetGroup->assetInfoGroupAdd_->size() * sizeof(AssetInfo) +
      updateInfoOfAssetGroup->assetInfoGroupDel_->size() * sizeof(AssetInfo) +
      updateInfoOfAssetGroup->assetInfoGroupChg_->size() * sizeof(AssetInfo);
  const auto shmBufLen = sizeof(AssetInfoNotify) + extDataLen;

  shmCli->asyncSendMsgWithZeroCopy(
      [&](void* shmBuf) {
        auto assetInfoNotify = static_cast<AssetInfoNotify*>(shmBuf);
        assetInfoNotify->acctId_ = acctId;
        assetInfoNotify->addNum_ =
            updateInfoOfAssetGroup->assetInfoGroupAdd_->size();
        assetInfoNotify->delNum_ =
            updateInfoOfAssetGroup->assetInfoGroupDel_->size();
        assetInfoNotify->chgNum_ =
            updateInfoOfAssetGroup->assetInfoGroupChg_->size();
        assetInfoNotify->extDataLen_ = extDataLen;

        char* pos = static_cast<char*>(shmBuf) + sizeof(AssetInfoNotify);
        for (const auto& rec : *updateInfoOfAssetGroup->assetInfoGroupAdd_) {
          memcpy(pos, rec.get(), sizeof(AssetInfo));
          pos += sizeof(AssetInfo);
        }
        for (const auto& rec : *updateInfoOfAssetGroup->assetInfoGroupDel_) {
          memcpy(pos, rec.get(), sizeof(AssetInfo));
          pos += sizeof(AssetInfo);
        }
        for (const auto& rec : *updateInfoOfAssetGroup->assetInfoGroupChg_) {
          memcpy(pos, rec.get(), sizeof(AssetInfo));
          pos += sizeof(AssetInfo);
        }
      },
      MSG_ID_SYNC_ASSETS, shmBufLen);
}

}  // namespace bq::td::svc
