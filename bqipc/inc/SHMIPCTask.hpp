/*!
 * \file SHMIPCTask.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "def/DefIF.hpp"
#include "util/PchBase.hpp"

namespace bq {

struct SHMIPCTask {
  SHMIPCTask() = default;
  SHMIPCTask(const void* origData, std::size_t origDataLen) {
    data_ = malloc(origDataLen);
    len_ = origDataLen;
    memcpy(data_, origData, origDataLen);
  }
  void* data_{nullptr};
  std::size_t len_{0};

  ~SHMIPCTask() { SAFE_FREE(data_); }
};
using SHMIPCTaskSPtr = std::shared_ptr<SHMIPCTask>;

template <typename Msg>
std::shared_ptr<Msg> MakeMsgSPtrByTask(const SHMIPCTaskSPtr& task) {
  Msg* msg = static_cast<Msg*>(task->data_);
  std::shared_ptr<Msg> ret(msg);
  task->data_ = nullptr;
  task->len_ = 0;
  return ret;
}

}  // namespace bq
