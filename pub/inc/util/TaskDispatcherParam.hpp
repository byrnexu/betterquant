#pragma once

#include "util/Pch.hpp"

namespace bq {

const static std::string SEP_OF_REC_IN_TASK_DISPATCHER_PARAM = ";";
const static std::string SEP_OF_FIELD_IN_TASK_DISPATCHER_PARAM = "=";

const static std::string DEFAULT_TASK_DISPATCHER_PARAM =
    "moduleName=TaskDispatcher; numOfUnprocessedTaskAlert=100; "
    "maxBulkRecvTaskNumEveryTime=1; timeDurOfWaitForTask=500; "
    "taskRandAssignedThreadPoolSize=1; taskSpecificThreadPoolSize=4; "
    "preCreateTaskSpecificThreadPool=0";

struct TaskDispatcherParam;
using TaskDispatcherParamSPtr = std::shared_ptr<TaskDispatcherParam>;

struct TaskDispatcherParam {
  TaskDispatcherParam() = default;
  TaskDispatcherParam(const std::string& moduleName,
                      std::uint32_t taskRandAssignedThreadPoolSize = 1,
                      std::uint32_t taskSpecificThreadPoolSize = 4,
                      std::uint32_t numOfUnprocessedTaskAlert = 100,
                      std::uint32_t maxBulkRecvTaskNumEveryTime = 1,
                      std::uint32_t timeDurOfWaitForTask = 1000,
                      bool preCreateTaskSpecificThreadPool = false);

  std::string moduleName_;
  std::uint32_t taskRandAssignedThreadPoolSize_{1};
  std::uint32_t taskSpecificThreadPoolSize_{4};
  std::uint32_t numOfUnprocessedTaskAlert_{100};
  std::uint32_t maxBulkRecvTaskNumEveryTime_{1};
  std::uint32_t timeDurOfWaitForTask_{1000};
  bool preCreateTaskSpecificThreadPool_{false};
};

std::tuple<int, TaskDispatcherParamSPtr> MakeTaskDispatcherParam(
    const std::string& taskDispatcherParamInStrFmt);

}  // namespace bq
