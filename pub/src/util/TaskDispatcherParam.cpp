/*!
 * \file TaskDispatcherParam.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "util/TaskDispatcherParam.hpp"

#include "def/Def.hpp"
#include "util/Logger.hpp"
#include "util/String.hpp"

namespace bq {

TaskDispatcherParam::TaskDispatcherParam(
    const std::string& moduleName, std::uint32_t taskRandAssignedThreadPoolSize,
    std::uint32_t taskSpecificThreadPoolSize,
    std::uint32_t numOfUnprocessedTaskAlert,
    std::uint32_t maxBulkRecvTaskNumEveryTime,
    std::uint32_t timeDurOfWaitForTask, bool preCreateTaskSpecificThreadPool)
    : moduleName_(moduleName),
      taskRandAssignedThreadPoolSize_(taskRandAssignedThreadPoolSize),
      taskSpecificThreadPoolSize_(taskSpecificThreadPoolSize),
      numOfUnprocessedTaskAlert_(numOfUnprocessedTaskAlert),
      maxBulkRecvTaskNumEveryTime_(maxBulkRecvTaskNumEveryTime),
      timeDurOfWaitForTask_(timeDurOfWaitForTask),
      preCreateTaskSpecificThreadPool_(preCreateTaskSpecificThreadPool) {}

std::tuple<int, TaskDispatcherParamSPtr> MakeTaskDispatcherParam(
    const std::string& taskDispatcherParamInStrFmt) {
  auto [retOfStr2Map, taskDispatcherParamTable] =
      Str2Map(taskDispatcherParamInStrFmt, SEP_OF_REC_IN_TASK_DISPATCHER_PARAM,
              SEP_OF_FIELD_IN_TASK_DISPATCHER_PARAM);
  if (retOfStr2Map != 0) {
    LOG_E("Make task dispatcher param failed. {}", taskDispatcherParamInStrFmt);
    return {retOfStr2Map, nullptr};
  }

  std::string fieldName;
  std::string fieldValue;
  auto ret = std::make_shared<TaskDispatcherParam>();
  try {
    fieldName = "modulename";
    fieldValue = taskDispatcherParamTable[fieldName];
    ret->moduleName_ = fieldValue;

    fieldName = "taskrandassignedthreadpoolsize";
    fieldValue = taskDispatcherParamTable[fieldName];
    ret->taskRandAssignedThreadPoolSize_ = CONV(std::uint32_t, fieldValue);

    fieldName = "taskspecificthreadpoolsize";
    fieldValue = taskDispatcherParamTable[fieldName];
    ret->taskSpecificThreadPoolSize_ = CONV(std::uint32_t, fieldValue);
    if (ret->taskSpecificThreadPoolSize_ == 0) {
      ret->taskSpecificThreadPoolSize_ = 1;
    }

    fieldName = "numofunprocessedtaskalert";
    fieldValue = taskDispatcherParamTable[fieldName];
    ret->numOfUnprocessedTaskAlert_ = CONV(std::uint32_t, fieldValue);

    fieldName = "maxbulkrecvtasknumeverytime";
    fieldValue = taskDispatcherParamTable[fieldName];
    ret->maxBulkRecvTaskNumEveryTime_ = CONV(std::uint32_t, fieldValue);

    fieldName = "timedurofwaitfortask";
    fieldValue = taskDispatcherParamTable[fieldName];
    ret->timeDurOfWaitForTask_ = CONV(std::uint32_t, fieldValue);

    fieldName = "precreatetaskspecificthreadpool";
    fieldValue = taskDispatcherParamTable[fieldName];
    auto preCreate = CONV(std::uint32_t, fieldValue);
    ret->preCreateTaskSpecificThreadPool_ = preCreate == 0 ? false : true;

  } catch (const std::exception& e) {
    LOG_E(
        "Make task dispatcher param failed "
        "because of invalid field info of {}. {}",
        fieldName, e.what());
    return {-1, TaskDispatcherParamSPtr()};
  }

  return {0, ret};
}

}  // namespace bq
