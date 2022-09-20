/*!
 * \file TaskDispatcher.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "TaskDispatcherParam.hpp"
#include "def/Def.hpp"
#include "util/Logger.hpp"
#include "util/Pch.hpp"
#include "util/StdExt.hpp"

namespace bq {

constexpr static std::uint32_t RAND_THREAD = UINT32_MAX;

template <typename Task>
using CBMsgParser =
    std::function<std::tuple<int, AsyncTaskSPtr<Task>>(const Task&)>;

template <typename Task>
using CBGetThreadNoForTask =
    std::function<std::uint32_t(const AsyncTaskSPtr<Task>&, std::uint32_t)>;

template <typename Task>
using CBHandleAsyncTask = std::function<void(AsyncTaskSPtr<Task>&)>;

using CBOnThreadStart = std::function<void(std::uint32_t threadNo)>;

template <typename Task>
using AsyncTaskBulk = std::deque<AsyncTaskSPtr<Task>>;

template <typename Task>
using AsyncTaskBulkSPtr = std::shared_ptr<AsyncTaskBulk<Task>>;

template <typename Task>
class TaskDispatcher;

template <typename Task>
using TaskDispatcherSPtr = std::shared_ptr<TaskDispatcher<Task>>;

template <typename Task>
class TaskDispatcher {
 public:
  TaskDispatcher(const TaskDispatcher&) = delete;
  TaskDispatcher& operator=(const TaskDispatcher&) = delete;
  TaskDispatcher(const TaskDispatcher&&) = delete;
  TaskDispatcher& operator=(const TaskDispatcher&&) = delete;

  TaskDispatcher(const TaskDispatcherParamSPtr& taskDispatcherParam,
                 const CBMsgParser<Task>& cbMsgParser,
                 const CBGetThreadNoForTask<Task>& cbGetThreadNoForTask,
                 const CBHandleAsyncTask<Task>& cbHandleAsyncTask,
                 const CBOnThreadStart cbOnThreadStart = nullptr)
      : taskDispatcherParam_(taskDispatcherParam),
        cbMsgParser_(cbMsgParser),
        cbGetThreadNoForTask_(cbGetThreadNoForTask),
        cbHandleAsyncTask_(cbHandleAsyncTask),
        cbOnThreadStart_(cbOnThreadStart) {}

  void init() {
    for (std::uint32_t i = 0;
         i < taskDispatcherParam_->taskSpecificThreadPoolSize_; ++i) {
      taskSpecificQueueGroup_.emplace_back(
          moodycamel::BlockingConcurrentQueue<AsyncTaskSPtr<Task>>());
      {
        std::lock_guard<std::ext::spin_mutex> guard(mtxTaskSpecificThreadPool_);
        taskSpecificThreadPool_.emplace_back(std::shared_ptr<std::thread>());
      }
    }
  }

  void start() {
    for (std::uint32_t i = 0;
         i < taskDispatcherParam_->taskRandAssignedThreadPoolSize_; ++i) {
      taskRandAssignedThreadPool_.emplace_back(
          std::thread([this]() { doStart(); }));
    }
    if (taskDispatcherParam_->preCreateTaskSpecificThreadPool_) {
      for (std::uint32_t threadNo = 0;
           threadNo < taskDispatcherParam_->taskSpecificThreadPoolSize_;
           ++threadNo) {
        taskSpecificThreadPool_[threadNo] = std::make_shared<std::thread>(
            [this, threadNo]() { doStart(threadNo); });
      }
    }
  }

 private:
  void doStart() {
    while (stopped_ == false || taskRandAssignedQueue_.size_approx() != 0) {
      checkUnprocessedAsyncTaskAndAlert(taskRandAssignedQueue_, -1);
      auto asyncTaskBulk = std::make_shared<AsyncTaskBulk<Task>>(
          taskDispatcherParam_->maxBulkRecvTaskNumEveryTime_);
      if (const auto asyncTaskNumInQue =
              taskRandAssignedQueue_.wait_dequeue_bulk_timed(
                  std::begin(*asyncTaskBulk),
                  taskDispatcherParam_->maxBulkRecvTaskNumEveryTime_,
                  std::chrono::milliseconds(
                      taskDispatcherParam_->timeDurOfWaitForTask_));
          asyncTaskNumInQue != 0) {
        asyncTaskBulk->resize(asyncTaskNumInQue);
      } else {
        continue;
      }
      for (auto& asyncTask : *asyncTaskBulk) {
        if (cbHandleAsyncTask_) cbHandleAsyncTask_(asyncTask);
      }
    }
  }

  void doStart(int i) {
    if (cbOnThreadStart_) cbOnThreadStart_(i);
    while (stopped_ == false || taskSpecificQueueGroup_[i].size_approx() != 0) {
      checkUnprocessedAsyncTaskAndAlert(taskSpecificQueueGroup_[i], i);
      AsyncTaskSPtr<Task> asyncTask;
      if (taskSpecificQueueGroup_[i].wait_dequeue_timed(
              asyncTask, std::chrono::milliseconds(
                             taskDispatcherParam_->timeDurOfWaitForTask_))) {
        if (cbHandleAsyncTask_) cbHandleAsyncTask_(asyncTask);
      } else {
        continue;
      }
    }
  }

  template <typename AsyncTaskQueue>
  void checkUnprocessedAsyncTaskAndAlert(const AsyncTaskQueue& asyncTaskQueue,
                                         int i) {
    const auto size_approx = asyncTaskQueue.size_approx();
    if (size_approx > 0 &&
        size_approx % taskDispatcherParam_->numOfUnprocessedTaskAlert_ == 0) {
      std::string threadNo = i >= 0 ? std::to_string(i) : "N/A";
      LOG_W("[{}] Too many unprocessed task info. [num = {}] [threadNo = {}].",
            taskDispatcherParam_->moduleName_, size_approx, threadNo);
    }
  }

 public:
  void stop() {
    stopped_ = true;

    for (std::uint32_t i = 0;
         i < taskDispatcherParam_->taskSpecificThreadPoolSize_; ++i) {
      std::lock_guard<std::ext::spin_mutex> guard(mtxTaskSpecificThreadPool_);
      if (taskSpecificThreadPool_[i] &&
          taskSpecificThreadPool_[i]->joinable()) {
        taskSpecificThreadPool_[i]->join();
      }
    }

    for (auto& trd : taskRandAssignedThreadPool_) {
      if (trd.joinable()) {
        trd.join();
      }
    }
  }

 public:
  int dispatch(Task& task) {
    assert(cbMsgParser_ != nullptr && "cbMsgParser_ != nullptr");
    auto [ret, asyncTask] = cbMsgParser_(task);
    if (ret != 0) {
      LOG_W("Dispatch task failed.");
      return -1;
    }
    if (asyncTask == nullptr) return 0;
    return dispatch(asyncTask);
  }

  int dispatch(AsyncTaskSPtr<Task>& asyncTask) {
    assert(cbGetThreadNoForTask_ != nullptr &&
           "cbGetThreadNoForTask_ != nullptr");
    const auto threadNo = cbGetThreadNoForTask_(
        asyncTask, taskDispatcherParam_->taskSpecificThreadPoolSize_);
    if (threadNo == RAND_THREAD) {
      return dispatchToRandAssignedThread(asyncTask);
    } else {
      return dispatchToSpecifiedThread(asyncTask, threadNo);
    }
    return 0;
  }

 private:
  inline int dispatchToRandAssignedThread(AsyncTaskSPtr<Task>& asyncTask) {
    taskRandAssignedQueue_.enqueue(asyncTask);
    return 0;
  }

  inline int dispatchToSpecifiedThread(AsyncTaskSPtr<Task>& asyncTask,
                                       std::uint32_t threadNo) {
    if (threadNo >= taskDispatcherParam_->taskSpecificThreadPoolSize_) {
      LOG_W(
          "[{}] Thread no {} greater than or equal to "
          "dispatcher thread pool size {}.",
          taskDispatcherParam_->moduleName_, threadNo,
          taskDispatcherParam_->taskSpecificThreadPoolSize_);
      return -1;
    }

    {
      std::lock_guard<std::ext::spin_mutex> guard(mtxTaskSpecificThreadPool_);
      if (taskSpecificThreadPool_[threadNo] == nullptr) {
        taskSpecificThreadPool_[threadNo] = std::make_shared<std::thread>(
            [this, threadNo]() { doStart(threadNo); });
      }
    }

    taskSpecificQueueGroup_[threadNo].enqueue(asyncTask);
    return 0;
  }

 public:
  TaskDispatcherParamSPtr getTaskDispatcherParam() const {
    return taskDispatcherParam_;
  }

 private:
  bool stopped_{false};

  CBMsgParser<Task> cbMsgParser_{nullptr};
  CBGetThreadNoForTask<Task> cbGetThreadNoForTask_{nullptr};
  CBHandleAsyncTask<Task> cbHandleAsyncTask_{nullptr};
  CBOnThreadStart cbOnThreadStart_{nullptr};

  TaskDispatcherParamSPtr taskDispatcherParam_{nullptr};

  moodycamel::BlockingConcurrentQueue<AsyncTaskSPtr<Task>>
      taskRandAssignedQueue_;
  std::vector<std::thread> taskRandAssignedThreadPool_;

  std::vector<moodycamel::BlockingConcurrentQueue<AsyncTaskSPtr<Task>>>
      taskSpecificQueueGroup_;
  std::vector<std::shared_ptr<std::thread>> taskSpecificThreadPool_;
  std::ext::spin_mutex mtxTaskSpecificThreadPool_;
};

}  // namespace bq
