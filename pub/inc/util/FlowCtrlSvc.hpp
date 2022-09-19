#pragma once

#include "def/Const.hpp"
#include "util/Pch.hpp"
#include "util/StdExt.hpp"

namespace bq {

class FlowCtrlSvc;
using FlowCtrlSvcSPtr = std::shared_ptr<FlowCtrlSvc>;

template <typename T>
using FixedSizeQueue = std::queue<T, boost::circular_buffer<T>>;

struct FlowLimitInfo {
  FlowLimitInfo(std::uint32_t timeDur, std::int32_t limitNum);

  std::string toStr() const;

  std::uint32_t timeDur_;
  std::uint32_t limitNum_;
  FixedSizeQueue<boost::posix_time::ptime> timePointGroupOfTaskHasBeenExec_;
};
using FlowLimitInfoSPtr = std::shared_ptr<FlowLimitInfo>;

class FlowCtrlSvc {
 public:
  explicit FlowCtrlSvc(const YAML::Node& node);

 public:
  bool exceedFlowCtrl(const std::string& taskName,
                      WriteLog writeLog = WriteLog::True);
  void reset();

 private:
  std::map<std::string, std::uint32_t> taskName2GroupNo_;
  std::map<std::string, std::uint32_t> taskName2Weight_;
  std::map<std::uint32_t, FlowLimitInfoSPtr> groupNo2FlowLimitInfo_;
  std::ext::spin_mutex mtxFlowCtrl_;
};

}  // namespace bq
