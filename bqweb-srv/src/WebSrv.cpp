/*!
 * \file WebSrv.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/20
 *
 * \brief
 */

#include "WebSrv.hpp"

#include <drogon/drogon.h>

#include "Config.hpp"
#include "SHMHeader.hpp"
#include "SHMIPCTask.hpp"
#include "SHMSrv.hpp"
#include "WebSrvConst.hpp"
#include "db/DBE.hpp"
#include "def/BQDef.hpp"
#include "util/Literal.hpp"
#include "util/Logger.hpp"
#include "util/StdExt.hpp"
#include "util/String.hpp"
#include "util/TaskDispatcher.hpp"

namespace bq {

int WebSrv::prepareInit() {
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

int WebSrv::doInit() {
  if (const auto ret = initDBEng(); ret != 0) {
    LOG_E("Do init failed. ");
    return ret;
  }

  if (const auto ret = initWebSrvTaskDispatcher(); ret != 0) {
    LOG_E("Do init failed.");
    return ret;
  }

  initSHMSrv();

  return 0;
}

int WebSrv::initWebSrvTaskDispatcher() {
  const auto webSrvTaskDispatcherParamInStrFmt =
      SetParam(DEFAULT_TASK_DISPATCHER_PARAM,
               CONFIG["webSrvTaskDispatcherParam"].as<std::string>());
  const auto [ret, webSrvTaskDispatcherParam] =
      MakeTaskDispatcherParam(webSrvTaskDispatcherParamInStrFmt);
  if (ret != 0) {
    LOG_E("Init taskdispatcher failed. {}", webSrvTaskDispatcherParamInStrFmt);
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

  webSrvTaskDispatcher_ = std::make_shared<TaskDispatcher<SHMIPCTaskSPtr>>(
      webSrvTaskDispatcherParam, makeAsyncTask, getThreadForAsyncTask,
      handleAsyncTask);
  webSrvTaskDispatcher_->init();

  return ret;
}

void WebSrv::initSHMSrv() {
  const auto stgEngChannel =
      fmt::format("{}@{}", AppName, CONFIG["stgEngChannel"].as<std::string>());
  shmSrvOfStgEng_ = std::make_shared<SHMSrv>(
      stgEngChannel, [this](const auto* shmBuf, std::size_t shmBufLen) {
        auto task = std::make_shared<SHMIPCTask>(shmBuf, shmBufLen);
        webSrvTaskDispatcher_->dispatch(task);
      });
}

int WebSrv::initDBEng() {
  const auto dbEngParam = SetParam(db::DEFAULT_DB_ENG_PARAM,
                                   CONFIG["dbEngParam"].as<std::string>());
  int retOfMakeDBEng = 0;
  std::tie(retOfMakeDBEng, dbEng_) = db::MakeDBEng(
      dbEngParam, [](db::DBTaskSPtr& dbTask, const StringSPtr& dbExecRet) {
        LOG_D("Exec sql finished. [{}] [exec result = {}]", dbTask->toStr(),
              *dbExecRet);
      });
  if (retOfMakeDBEng != 0) {
    LOG_E("Init dbeng failed. {}", dbEngParam);
    return retOfMakeDBEng;
  }

  if (auto retOfInit = getDBEng()->init(); retOfInit != 0) {
    LOG_E("Init dbeng failed. {}", dbEngParam);
    return retOfInit;
  }

  return 0;
}

int WebSrv::doRun() {
  LOG_I("Start web srv.");
  getDBEng()->start();
  webSrvTaskDispatcher_->start();
  shmSrvOfStgEng_->start();
  startDrogon();
  return 0;
}

void WebSrv::startDrogon() {
  LOG_I("Begin to start drogon.");
  threadDrogon_ = std::make_shared<std::thread>([]() {
    drogon::app().loadConfigFile("config/bqweb-srv/config.json");
    drogon::app().disableSigtermHandling();
    drogon::app().run();
  });
}

void WebSrv::doExit(const boost::system::error_code* ec, int signalNum) {
  stopDrogon();
  LOG_I("End of stop drogon.");
  webSrvTaskDispatcher_->stop();
  shmSrvOfStgEng_->stop();
  getDBEng()->stop();
  LOG_I("Stop web srv.");
}

void WebSrv::stopDrogon() {
  drogon::app().quit();
  if (threadDrogon_->joinable()) {
    threadDrogon_->join();
  }
}

}  // namespace bq
