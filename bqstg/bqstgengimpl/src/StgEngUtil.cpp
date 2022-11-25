/*!
 * \file StgEngUtil.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "StgEngUtil.hpp"

#include "SHMIPCTask.hpp"
#include "def/BQDef.hpp"
#include "def/CommonIPCData.hpp"
#include "def/DataStruOfOthers.hpp"
#include "util/TaskDispatcher.hpp"

namespace bq::stg {

SHMIPCAsyncTaskSPtr MakeStgSignal(MsgId msgId, StgInstId stgInstId) {
  StgSignal stgSignal(msgId, stgInstId);
  const auto task = std::make_shared<SHMIPCTask>(&stgSignal, sizeof(StgSignal));
  const auto ret = std::make_shared<SHMIPCAsyncTask>(task, stgInstId);
  return ret;
}

std::tuple<int, StgInstId> GetStgInstId(const CommonIPCData* commonIPCData) {
  Doc doc;
  if (doc.Parse(commonIPCData->data_).HasParseError()) {
    LOG_W("Parse data failed. {0} [offset {1}] {2}",
          GetParseError_En(doc.GetParseError()), doc.GetErrorOffset(),
          commonIPCData->data_);
    return {-1, 1};
  }

  if (doc.HasMember("stgInstId") && doc["stgInstId"].IsUint()) {
    const auto stgInstId = doc["stgInstId"].GetUint();
    return {0, stgInstId};
  }

  return {-1, 1};
}

}  // namespace bq::stg
