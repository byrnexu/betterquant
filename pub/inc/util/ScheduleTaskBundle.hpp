#pragma once

#include "def/Const.hpp"
#include "util/Datetime.hpp"
#include "util/Pch.hpp"

namespace bq {

using ScheduleTaskHandler = std::function<bool()>;

struct ScheduleTask {
  ScheduleTask(const std::string& scheduleTaskName,
               const ScheduleTaskHandler& scheduleTaskHandler,
               ExecAtStartup execAtStartup, std::uint32_t interval,
               std::uint64_t maxExecTimes = UINT64_MAX,
               WriteLog writeLog = WriteLog::True);

  const std::string scheduleTaskName_;
  const ScheduleTaskHandler scheduleTaskHandler_;
  const ExecAtStartup execAtStartup_{ExecAtStartup::False};
  const std::uint32_t interval_{1};
  std::uint64_t lastExecTime_{GetTotalSecSince1970()};
  std::uint64_t execTimes_{0};
  const std::uint64_t maxExecTimes_{UINT64_MAX};
  const WriteLog writeLog_{WriteLog::True};
};

using ScheduleTaskSPtr = std::shared_ptr<ScheduleTask>;
using ScheduleTaskBundle = std::vector<ScheduleTaskSPtr>;
using ScheduleTaskBundleSPtr = std::shared_ptr<ScheduleTaskBundle>;
void ExecScheduleTaskBundle(ScheduleTaskBundleSPtr& scheduleTaskBundle);

}  // namespace bq
