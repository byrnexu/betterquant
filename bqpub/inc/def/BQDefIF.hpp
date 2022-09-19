#pragma once

#include "util/PchBase.hpp"

namespace bq {

using TopicGroup = std::set<std::string>;
using TopicGroupSPtr = std::shared_ptr<TopicGroup>;

template <typename Task>
struct AsyncTask;

struct SHMIPCTask;
using SHMIPCTaskSPtr = std::shared_ptr<SHMIPCTask>;

using SHMIPCAsyncTask = AsyncTask<SHMIPCTaskSPtr>;
using SHMIPCAsyncTaskSPtr = std::shared_ptr<SHMIPCAsyncTask>;

using Decimal = double;

using StgId = std::uint16_t;
using StgInstId = std::uint16_t;

using UserId = std::uint16_t;
using AcctId = std::uint16_t;

using OrderId = std::uint64_t;
using ExchOrderId = std::uint64_t;

}  // namespace bq
