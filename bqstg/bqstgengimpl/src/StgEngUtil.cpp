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
#include "def/DataStruOfOthers.hpp"
#include "util/TaskDispatcher.hpp"

namespace bq::stg {

SHMIPCAsyncTaskSPtr MakeStgSignal(MsgId msgId, StgInstId stgInstId) {
  StgSignal stgSignal(msgId, stgInstId);
  const auto task = std::make_shared<SHMIPCTask>(&stgSignal, sizeof(StgSignal));
  const auto ret = std::make_shared<SHMIPCAsyncTask>(task, stgInstId);
  return ret;
}

}  // namespace bq::stg
