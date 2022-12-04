/*!
 * \file MDSim.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/26
 *
 * \brief
 */

#pragma once

#include "SHMIPCDef.hpp"
#include "SHMIPCMsgId.hpp"
#include "db/DBEngDef.hpp"
#include "util/Pch.hpp"
#include "util/StdExt.hpp"
#include "util/SvcBase.hpp"

namespace bq {
template <typename Task>
class TaskDispatcher;
template <typename Task>
using TaskDispatcherSPtr = std::shared_ptr<TaskDispatcher<Task>>;
}  // namespace bq

namespace bq::md {

class MDCache;
using MDCacheSPtr = std::shared_ptr<MDCache>;

class MDPlayback;
using MDPlaybackSPtr = std::shared_ptr<MDPlayback>;

class MDSim : public SvcBase {
 public:
  using SvcBase::SvcBase;

 private:
  int prepareInit() final;
  int doInit() final;

 private:
  int initMDSimTaskDispatcher();
  void initSHMSrvGroup();

 public:
  int doRun() final;

 private:
  void doExit(const boost::system::error_code* ec, int signalNum) final;

 public:
  MDCacheSPtr getMDCache() const { return mdCache_; }

  TaskDispatcherSPtr<SHMIPCTaskSPtr> getMDSimTaskDispatcher() const {
    return mdSimTaskDispatcher_;
  }

  std::map<std::string, SHMSrvSPtr> getSHMSrvGroup() const {
    return shmSrvGroup_;
  }

  SHMSrvSPtr getSHMSrv(const std::string& addr) const {
    SHMSrvSPtr ret{nullptr};
    const auto iter = shmSrvGroup_.find(addr);
    if (iter != std::end(shmSrvGroup_)) {
      ret = iter->second;
    }
    return ret;
  }

 private:
  MDCacheSPtr mdCache_{nullptr};
  MDPlaybackSPtr mdPlayback_{nullptr};

  TaskDispatcherSPtr<SHMIPCTaskSPtr> mdSimTaskDispatcher_{nullptr};
  std::map<std::string, SHMSrvSPtr> shmSrvGroup_;
};

}  // namespace bq::md
