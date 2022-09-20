/*!
 * \file TBLMonitorOfStgInstInfo.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "db/TBLMonitor.hpp"
#include "db/TBLStgInstInfo.hpp"
#include "def/BQDef.hpp"
#include "def/DataStruOfStg.hpp"
#include "def/StatusCode.hpp"
#include "util/Pch.hpp"

namespace bq::db {

using StgInstInfoGroup = absl::node_hash_map<StgInstId, StgInstInfoSPtr>;
using StgInstInfoGroupSPtr = std::shared_ptr<StgInstInfoGroup>;

class TBLMonitorOfStgInstInfo : public TBLMonitor<TBLStgInstInfo> {
 public:
  using TBLMonitor<TBLStgInstInfo>::TBLMonitor;

 public:
  std::tuple<int, StgInstInfoSPtr> getStgInstInfo(StgInstId stgInstId) const {
    {
      std::lock_guard<std::ext::spin_mutex> guard(mtxStgInstInfoGroup_);
      const auto iter = stgInstInfoGroup_.find(stgInstId);
      if (iter != std::end(stgInstInfoGroup_)) {
        return {0, iter->second};
      }
    }
    return {SCODE_DB_CAN_NOT_FIND_STG_INST, nullptr};
  }

  std::vector<StgInstId> getStgInstIdGroup() const {
    std::vector<StgInstId> ret;
    {
      std::lock_guard<std::ext::spin_mutex> guard(mtxStgInstInfoGroup_);
      for (const auto& rec : stgInstInfoGroup_) {
        ret.emplace_back(rec.first);
      }
    }
    return ret;
  }

 private:
  void initNecessaryDataStructures(
      const TBLRecSetSPtr<TBLStgInstInfo>& tblRecOfAll) final {
    {
      std::lock_guard<std::ext::spin_mutex> guard(mtxStgInstInfoGroup_);
      for (const auto& tblRec : *tblRecOfAll) {
        const auto recStgInstInfo = tblRec.second->getRecWithAllFields();
        const auto stgInstInfo = MakeStgInstInfo(recStgInstInfo);
        stgInstInfoGroup_.emplace(stgInstInfo->stgInstId_, stgInstInfo);
      }
    }
  }

  void handleTBLRecSetOfCompRet(
      const db::TBLRecSetSPtr<TBLStgInstInfo>& tblRecSetAdd,
      const db::TBLRecSetSPtr<TBLStgInstInfo>& tblRecSetDel,
      const db::TBLRecSetSPtr<TBLStgInstInfo>& tblRecSetChg) final {
    {
      std::lock_guard<std::ext::spin_mutex> guard(mtxStgInstInfoGroup_);
      for (const auto& tblRec : *tblRecSetAdd) {
        const auto recStgInstInfo = tblRec.second->getRecWithAllFields();
        const auto stgInstInfo = MakeStgInstInfo(recStgInstInfo);
        stgInstInfoGroup_.emplace(stgInstInfo->stgInstId_, stgInstInfo);
      }
      for (const auto& tblRec : *tblRecSetDel) {
        const auto recStgInstInfo = tblRec.second->getRecWithAllFields();
        const auto stgInstInfo = MakeStgInstInfo(recStgInstInfo);
        stgInstInfoGroup_.erase(stgInstInfo->stgInstId_);
      }
      for (const auto& tblRec : *tblRecSetChg) {
        const auto recStgInstInfo = tblRec.second->getRecWithAllFields();
        const auto stgInstInfo = MakeStgInstInfo(recStgInstInfo);
        stgInstInfoGroup_[stgInstInfo->stgInstId_] = stgInstInfo;
      }
    }
  }

 private:
  StgInstInfoGroup stgInstInfoGroup_;
  mutable std::ext::spin_mutex mtxStgInstInfoGroup_;
};

}  // namespace bq::db
