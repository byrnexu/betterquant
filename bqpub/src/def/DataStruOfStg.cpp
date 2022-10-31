/*!
 * \file DataStruOfStg.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "def/DataStruOfStg.hpp"

#include "db/TBLStgInstInfo.hpp"

namespace bq {

std::string StgInstInfo::toStr() const {
  std::string ret;
  ret = fmt::format(
      "productId = {}; stgId={}; stgName={}; userIdOfAuthor={}; stgInstId={}; "
      "stgInstParams={}; stgInstName={}; userId={}; isDel={}",
      productId_, stgId_, stgName_, userIdOfAuthor_, stgInstId_, stgInstParams_,
      stgInstName_, userId_, isDel_);
  return ret;
}

StgInstInfoSPtr MakeStgInstInfo(
    const db::stgInstInfo::RecordSPtr& recStgInstInfo) {
  StgInstInfoSPtr ret = std::make_shared<StgInstInfo>();
  ret->productId_ = recStgInstInfo->productId;
  ret->stgId_ = recStgInstInfo->stgId;
  ret->stgName_ = recStgInstInfo->stgName;
  ret->userIdOfAuthor_ = recStgInstInfo->userIdOfAuthor;
  ret->stgInstId_ = recStgInstInfo->stgInstId;
  ret->stgInstParams_ = recStgInstInfo->stgInstParams;
  ret->stgInstName_ = recStgInstInfo->stgInstName;
  ret->userId_ = recStgInstInfo->userId;
  ret->isDel_ = recStgInstInfo->isDel;
  return ret;
}

StgInstId StgInstIdOfTriggerSignal(const StgInstInfoSPtr& stgInstInfo) {
  return stgInstInfo->stgInstId_;
}

}  // namespace bq
