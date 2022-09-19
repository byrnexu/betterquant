#include "util/ScheduleTaskBundle.hpp"

#include "util/Logger.hpp"

namespace bq {

ScheduleTask::ScheduleTask(const std::string& scheduleTaskName,
                           const ScheduleTaskHandler& scheduleTaskHandler,
                           ExecAtStartup execAtStartup, std::uint32_t interval,
                           std::uint64_t maxExecTimes, WriteLog writeLog)
    : scheduleTaskName_(scheduleTaskName),
      scheduleTaskHandler_(scheduleTaskHandler),
      execAtStartup_(execAtStartup),
      interval_(interval),
      maxExecTimes_(maxExecTimes),
      writeLog_(writeLog) {
  if (execAtStartup_ != ExecAtStartup::True) {
    lastExecTime_ = GetTotalMSSince1970();
  } else {
    lastExecTime_ = 0;
  }
}

void ExecScheduleTaskBundle(ScheduleTaskBundleSPtr& scheduleTaskBundle) {
  if (!scheduleTaskBundle) return;
  for (auto& scheduleTask : *scheduleTaskBundle) {
    if (scheduleTask->execTimes_ >= scheduleTask->maxExecTimes_) {
      continue;
    }
    const auto now = GetTotalMSSince1970();
    const auto timeDurAfterLastExec = now - scheduleTask->lastExecTime_;
    if (timeDurAfterLastExec >= scheduleTask->interval_) {
      if (scheduleTask->writeLog_ == WriteLog::True) {
        LOG_D("Begin to exec scheduler task {}. [exec times = {}]",
              scheduleTask->scheduleTaskName_, scheduleTask->execTimes_ + 1);
      }
      scheduleTask->lastExecTime_ = now;
      if (scheduleTask->scheduleTaskHandler_() == true) {
        ++scheduleTask->execTimes_;
      }
    }
  }
}

}  // namespace bq
