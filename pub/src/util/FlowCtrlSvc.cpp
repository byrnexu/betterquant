#include "util/FlowCtrlSvc.hpp"

#include "util/Float.hpp"
#include "util/Logger.hpp"

namespace bq {

FlowLimitInfo::FlowLimitInfo(std::uint32_t timeDur, std::int32_t limitNum)
    : timeDur_(timeDur),
      limitNum_(limitNum),
      timePointGroupOfTaskHasBeenExec_(
          boost::circular_buffer<boost::posix_time::ptime>(limitNum_)) {}

std::string FlowLimitInfo::toStr() const {
  return fmt::format("timeDur={}; limitNum={}", timeDur_, limitNum_);
}

FlowCtrlSvc::FlowCtrlSvc(const YAML::Node& node) {
  const auto& flowCtrlRule = node["flowCtrlRule"];
  std::size_t groupNo = 0;
  for (auto iter = flowCtrlRule.begin(); iter != flowCtrlRule.end();
       ++iter, ++groupNo) {
    const auto taskGroup = (*iter)["taskGroup"];
    for (auto iter = taskGroup.begin(); iter != taskGroup.end(); ++iter) {
      const auto taskName = (*iter)["name"].as<std::string>();
      const auto taskWeight = (*iter)["weight"].as<std::uint32_t>();
      taskName2GroupNo_.emplace(taskName, groupNo);
      taskName2Weight_.emplace(taskName, taskWeight);
      LOG_D("Add task info. groupNo={}; taskName={}; taskWeight={}", groupNo,
            taskName, taskWeight);
    }
    const auto timeDur = (*iter)["timeDur"].as<std::uint32_t>();
    const auto limitNum = (*iter)["limitNum"].as<std::uint32_t>();
    const auto flowLimitInfo =
        std::make_shared<FlowLimitInfo>(timeDur, limitNum);
    groupNo2FlowLimitInfo_.emplace(groupNo, flowLimitInfo);
    LOG_D("Add flow limit info. groupNo={}; {}", groupNo,
          flowLimitInfo->toStr());
  }
}

bool FlowCtrlSvc::exceedFlowCtrl(const std::string& taskName,
                                 WriteLog writeLog) {
  auto getGroupNo =
      [this](const std::string& taskName) -> std::optional<std::uint32_t> {
    const auto iter = taskName2GroupNo_.find(taskName);
    if (iter != taskName2GroupNo_.end()) {
      return std::make_optional<std::uint32_t>(iter->second);
    }
    return std::nullopt;
  };

  auto getTaskWeight = [this](const std::string& taskName) -> std::uint32_t {
    const auto iter = taskName2Weight_.find(taskName);
    if (iter != taskName2Weight_.end()) {
      return iter->second;
    }
    return 0;
  };

  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxFlowCtrl_);
    const auto groupNoOpt = getGroupNo(taskName);
    if (groupNoOpt == std::nullopt) {
      return false;
    }
    const auto groupNo = groupNoOpt.value();

    const auto taskWeight = getTaskWeight(taskName);
    if (taskWeight == 0) {
      return false;
    }

    const auto iter = groupNo2FlowLimitInfo_.find(groupNo);
    auto& flowLimitInfo = iter->second;

    const auto now = boost::posix_time::microsec_clock::universal_time();
    const auto numOfTask =
        flowLimitInfo->timePointGroupOfTaskHasBeenExec_.size();
    if (numOfTask + taskWeight < flowLimitInfo->limitNum_) {
      for (std::uint32_t i = 0; i < taskWeight; ++i) {
        flowLimitInfo->timePointGroupOfTaskHasBeenExec_.push(now);
      }
      return false;
    }

    const auto td =
        now - flowLimitInfo->timePointGroupOfTaskHasBeenExec_.front();
    if (td.total_milliseconds() == 0) {
      return true;
    }

    const auto timeDurBetweenFirstTaskAndNow =
        static_cast<double>(td.total_milliseconds());
    const auto taskNumPerMSIfExec =
        (numOfTask + taskWeight) / timeDurBetweenFirstTaskAndNow;

    const auto timeDurOfLimit = static_cast<double>(flowLimitInfo->timeDur_);
    const auto taskNumLimitPerMS = flowLimitInfo->limitNum_ / timeDurOfLimit;

    if (isDefinitelyGreaterOrEqual(taskNumPerMSIfExec, taskNumLimitPerMS)) {
      if (writeLog == WriteLog::True) {
        LOG_W(
            "Exceed flow ctrl. "
            "timeDur={}; limitNum={}; "
            "timeDurBetweenFirstTaskAndNow={}; taskNumIfExec={}; "
            "taskNumPerMSIfExec={} >= taskNumLimitPerMS={}",
            flowLimitInfo->timeDur_, flowLimitInfo->limitNum_,
            timeDurBetweenFirstTaskAndNow, (numOfTask + taskWeight),
            taskNumPerMSIfExec, taskNumLimitPerMS);
      }
      return true;
    } else {
      for (std::uint32_t i = 0; i < taskWeight; ++i) {
        flowLimitInfo->timePointGroupOfTaskHasBeenExec_.push(now);
      }
      return false;
    }
  }

  return false;
}

void FlowCtrlSvc::reset() {
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxFlowCtrl_);
    for (const auto& rec : groupNo2FlowLimitInfo_) {
      auto& flowLimitInfo = rec.second;
      while (!flowLimitInfo->timePointGroupOfTaskHasBeenExec_.empty()) {
        flowLimitInfo->timePointGroupOfTaskHasBeenExec_.pop();
      }
    }
  }
}

}  // namespace bq
