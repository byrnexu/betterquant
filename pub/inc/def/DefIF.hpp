#pragma once

#include "util/PchBase.hpp"

namespace bq {

#define SAFE_DELETE(obj) \
  if (obj != nullptr) {  \
    delete obj;          \
    obj = nullptr        \
  }

#define SAFE_FREE(buf)  \
  if (buf != nullptr) { \
    free(buf);          \
    buf = nullptr;      \
  }

using StringSPtr = std::shared_ptr<std::string>;
using StringUPtr = std::unique_ptr<std::string>;
using StringWPtr = std::weak_ptr<std::string>;

using TopicHash = std::uint64_t;

template <typename Task>
struct AsyncTask {
  AsyncTask() = default;
  explicit AsyncTask(const Task& task, const std::any& arg = std::any())
      : task_(task), arg_(arg) {}
  Task task_;
  std::any arg_;
};
template <typename Task>
using AsyncTaskSPtr = std::shared_ptr<AsyncTask<Task>>;

}  // namespace bq
