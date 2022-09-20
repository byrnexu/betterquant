/*!
 * \file StgEngUtil.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "SHMIPCMsgId.hpp"
#include "StgEngDef.hpp"

namespace bq {
struct SHMIPCTask;
using SHMIPCTaskSPtr = std::shared_ptr<SHMIPCTask>;
template <typename Task>
struct AsyncTask;
using SHMIPCAsyncTask = AsyncTask<SHMIPCTaskSPtr>;
using SHMIPCAsyncTaskSPtr = std::shared_ptr<SHMIPCAsyncTask>;
}  // namespace bq

namespace bq::stg {

SHMIPCAsyncTaskSPtr MakeStgSignal(MsgId msgId, StgInstId stgInstId);

}  // namespace bq::stg
