/*!
 * \file MDSim.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/26
 *
 * \brief
 */

#include "MDSim.hpp"

#include "Config.hpp"
#include "MDCache.hpp"
#include "MDPlayback.hpp"
#include "SHMHeader.hpp"
#include "SHMIPCTask.hpp"
#include "SHMSrv.hpp"
#include "def/BQDef.hpp"
#include "def/Const.hpp"
#include "def/DataStruOfMD.hpp"
#include "def/Def.hpp"
#include "util/BQUtil.hpp"
#include "util/Literal.hpp"
#include "util/Logger.hpp"
#include "util/ScheduleTaskBundle.hpp"
#include "util/Scheduler.hpp"
#include "util/StdExt.hpp"
#include "util/String.hpp"
#include "util/TaskDispatcher.hpp"

namespace bq::md {

int MDSim::prepareInit() {
  if (const auto ret = Config::get_mutable_instance().init(configFilename_);
      ret != 0) {
    const auto statusMsg = fmt::format("Prepare init failed.");
    std::cerr << statusMsg << std::endl;
    return ret;
  }

  if (const auto ret = InitLogger(configFilename_); ret != 0) {
    const auto statusMsg =
        fmt::format("Init td srv failed because of init logger failed. {}",
                    configFilename_);
    std::cerr << statusMsg << std::endl;
    return ret;
  }

  return 0;
}

int MDSim::doInit() {
  if (const auto ret = initMDSimTaskDispatcher(); ret != 0) {
    LOG_E("Do init failed.");
    return ret;
  }

  initSHMSrvGroup();
  mdCache_ = std::make_shared<MDCache>(this);
  mdPlayback_ = std::make_shared<MDPlayback>(this);

  return 0;
}

int MDSim::initMDSimTaskDispatcher() {
  const auto mdSimTaskDispatcherParamInStrFmt =
      SetParam(DEFAULT_TASK_DISPATCHER_PARAM,
               CONFIG["mdSimTaskDispatcherParam"].as<std::string>());
  const auto [ret, mdSimTaskDispatcherParam] =
      MakeTaskDispatcherParam(mdSimTaskDispatcherParamInStrFmt);
  if (ret != 0) {
    LOG_E("Init taskdispatcher failed. {}", mdSimTaskDispatcherParamInStrFmt);
    return ret;
  }

  const auto makeAsyncTask = [](const auto& task) {
    return std::make_tuple(0,
                           std::make_shared<SHMIPCAsyncTask>(task, std::any()));
  };

  const auto getThreadForAsyncTask = [](const auto& asyncTask,
                                        auto taskSpecificThreadPoolSize) {
    return ThreadNo(0);
  };

  const auto handleAsyncTask = [this](auto& asyncTask) {
    const auto shmHeader =
        static_cast<const SHMHeader*>(asyncTask->task_->data_);
    switch (shmHeader->msgId_) {
      default:
        LOG_W("Unhandled msg {} - {}.", shmHeader->msgId_,
              GetMsgName(shmHeader->msgId_));
        break;
    }
  };

  mdSimTaskDispatcher_ = std::make_shared<TaskDispatcher<SHMIPCTaskSPtr>>(
      mdSimTaskDispatcherParam, makeAsyncTask, getThreadForAsyncTask,
      handleAsyncTask);
  mdSimTaskDispatcher_->init();

  return ret;
}

void MDSim::initSHMSrvGroup() {
  const auto addrOfSHMSrvGroup =
      Config::get_const_instance().getAddrOfSHMSrvGroup();
  for (const auto& addr : addrOfSHMSrvGroup) {
    auto shmSrv = std::make_shared<SHMSrv>(
        addr, [this](const auto* shmBuf, std::size_t shmBufLen) {
          auto task = std::make_shared<SHMIPCTask>(shmBuf, shmBufLen);
          mdSimTaskDispatcher_->dispatch(task);
        });
    shmSrvGroup_.emplace(addr, shmSrv);
  }
}

int MDSim::doRun() {
  mdSimTaskDispatcher_->start();
  for (const auto& rec : shmSrvGroup_) {
    const auto& shmSrv = rec.second;
    shmSrv->start();
  }

  const auto statusCode = mdCache_->start();
  if (statusCode != 0) {
    return statusCode;
  }

  mdPlayback_->start();

  return 0;
}

void MDSim::doExit(const boost::system::error_code* ec, int signalNum) {
  mdPlayback_->stop();
  mdCache_->stop();

  for (const auto& rec : shmSrvGroup_) {
    const auto& shmSrv = rec.second;
    shmSrv->stop();
  }
  mdSimTaskDispatcher_->stop();
}

}  // namespace bq::md
