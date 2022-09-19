#include "WSCliOfExch.hpp"

#include "Config.hpp"
#include "ConnMetadata.hpp"
#include "MDSvc.hpp"
#include "RspParser.hpp"
#include "SubAndUnSubSvc.hpp"
#include "TopicGroupMustSubMaint.hpp"
#include "WSCli.hpp"
#include "WSTask.hpp"
#include "WebConst.hpp"
#include "WebParam.hpp"
#include "def/MDWSCliAsyncTaskArg.hpp"
#include "util/BQMDUtil.hpp"
#include "util/Datetime.hpp"
#include "util/FlowCtrlSvc.hpp"
#include "util/Literal.hpp"
#include "util/String.hpp"

namespace bq::md::svc {

WSCliOfExch::WSCliOfExch(MDSvc* mdSvc)
    : mdSvc_(mdSvc),
      topicGroupNeedMaint_(std::make_shared<TopicGroupNeedMaint>()) {}

int WSCliOfExch::init() {
  if (const auto ret = initWSCli(); ret != 0) {
    return ret;
  }
  if (const auto ret = initTaskDispatcher(); ret != 0) {
    return ret;
  }
  taskDispatcher_->init();
  return 0;
}

int WSCliOfExch::initWSCli() {
  const auto wsParamInStrFmt =
      SetParam(web::DEFAULT_WS_PARAM, CONFIG["wsParam"].as<std::string>());
  const auto [ret, wsParam] = web::MakeWSParam(wsParamInStrFmt);
  if (ret != 0) {
    LOG_E("Init wscli failed. {}", wsParamInStrFmt);
    return ret;
  }

  wsCli_ = std::make_shared<web::WSCli>(
      wsParam,
      [this](auto* wsCli, const auto& connMetadata, const auto& msg) {
        OnWSCliMsg(wsCli, connMetadata, msg);
      },
      [this](auto* wsCli, const auto& connMetadata) {
        OnWSCliOpen(wsCli, connMetadata);
      },
      nullptr, nullptr, mdSvc_->getPingPongSvc());

  return 0;
}

int WSCliOfExch::initTaskDispatcher() {
  const auto wsTaskDispatcherParamInStrFmt =
      SetParam(DEFAULT_TASK_DISPATCHER_PARAM,
               CONFIG["wsTaskDispatcherParam"].as<std::string>());
  const auto [ret, wsTaskDispatcherParam] =
      MakeTaskDispatcherParam(wsTaskDispatcherParamInStrFmt);
  if (ret != 0) {
    LOG_E("Init taskdispatcher failed. {}", wsTaskDispatcherParamInStrFmt);
    return ret;
  }

  const auto getThreadForAsyncTask =
      [](const WSCliAsyncTaskSPtr& asyncTask,
         std::uint32_t taskSpecificThreadPoolSize) {
        const auto arg = std::any_cast  //
            <WSCliAsyncTaskArgSPtr>(asyncTask->arg_);

        switch (arg->wsMsgType_) {
          case WSMsgType::Books:
            return ThreadNo(1);

          case WSMsgType::Trades:
          case WSMsgType::Tickers:
          case WSMsgType::Candle:
            return ThreadNo(0);

          default:
            return ThreadNo(0);
        }
        return ThreadNo(0);
      };

  taskDispatcher_ = std::make_shared<TaskDispatcher<web::TaskFromSrvSPtr>>(
      wsTaskDispatcherParam,
      [this](const auto& task) { return makeAsyncTask(task); },
      getThreadForAsyncTask,
      [this](auto& asyncTask) { handleAsyncTask(asyncTask); });

  return ret;
}

int WSCliOfExch::start() {
  taskDispatcher_->start();
  const auto ret = wsCli_->start();
  if (ret != 0) {
    LOG_E("Start WSCliOfExch failed.");
    return ret;
  }
  return ret;
}

void WSCliOfExch::stop() {
  wsCli_->stop();
  taskDispatcher_->stop();
}

void WSCliOfExch::OnWSCliOpen(web::WSCli* wsCli,
                              const web::ConnMetadataSPtr& connMetadata) {
  onBeforeOpen(wsCli, connMetadata);
  mdSvc_->getTopicGroupMustSubMaint()->clearTopicGroupAlreadySub();
  mdSvc_->getFlowCtrlSvc()->reset();
}

void WSCliOfExch::OnWSCliMsg(web::WSCli* wsCli,
                             const web::ConnMetadataSPtr& connMetadata,
                             const web::MsgSPtr& msg) {
  auto task = std::make_shared<web::TaskFromSrv>(wsCli, connMetadata, msg);
  taskDispatcher_->dispatch(task);
}

std::tuple<int, WSCliAsyncTaskSPtr> WSCliOfExch::makeAsyncTask(
    const web::TaskFromSrvSPtr& task) {
  const auto asyncTaskArg = MakeWSCliAsyncTaskArg(task);
  if (asyncTaskArg == nullptr) {
    LOG_W("WSCli msg parse failed. {}", task->msg_->get_payload());
    return {-1, nullptr};
  }
  const auto asyncTask = std::make_shared<WSCliAsyncTask>(task, asyncTaskArg);
  return std::make_tuple(0, asyncTask);
}

void WSCliOfExch::handleAsyncTask(WSCliAsyncTaskSPtr& asyncTask) {
  const auto asyncTaskArg =
      std::any_cast<WSCliAsyncTaskArgSPtr>(asyncTask->arg_);

  std::string topic = "";

  switch (asyncTaskArg->wsMsgType_) {
    case WSMsgType::Trades:
      topic = handleMDTrades(asyncTask);
      break;

    case WSMsgType::Tickers:
      topic = handleMDTickers(asyncTask);
      break;

    case WSMsgType::Candle:
      topic = handleMDCandle(asyncTask);
      break;

    case WSMsgType::Books:
      topic = handleMDBooks(asyncTask);
      break;

    default:
      handleMDOthers(asyncTask);
      break;
  }

  if (!topic.empty()) {
    updateActiveTimeOfTopic(topic);
  }

#ifdef PERF_TEST
  EXEC_PERF_TEST("HandleAsyncTask", asyncTask->task_->localTs_, 10000, 100);
#endif
}

void WSCliOfExch::handleMDOthers(WSCliAsyncTaskSPtr& asyncTask) {
  if (isSubOrUnSubRet(asyncTask)) {
    const auto topicGroupNeedMaint =
        mdSvc_->getRspParser()->getTopicGroupForSubOrUnSubAgain(asyncTask);
    for (const auto& topic : topicGroupNeedMaint->topicGroupNeedSub_) {
      mdSvc_->getTopicGroupMustSubMaint()->removeTopicForSubAgain(topic);
    }
    for (const auto& topic : topicGroupNeedMaint->topicGroupNeedUnSub_) {
      mdSvc_->getTopicGroupMustSubMaint()->addTopicForUnSubAgain(topic);
    }
  }
}

void WSCliOfExch::updateActiveTimeOfTopic(const std::string& topic) {
  const auto now = GetTotalMSSince1970();
  TopicGroupNeedMaintSPtr topicGroupNeedMaintOfCopy = nullptr;
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxTopicGroupNeedMaint_);
    if (now - topicGroupNeedMaint_->checkTs_ <= 5000) {
      topicGroupNeedMaint_->topic2ActiveTimeGroup_[topic] = now;
    } else {
      topicGroupNeedMaintOfCopy = topicGroupNeedMaint_;
      topicGroupNeedMaint_ = std::make_shared<TopicGroupNeedMaint>(now);
    }
  }
  if (topicGroupNeedMaintOfCopy != nullptr) {
    mdSvc_->getSubAndUnSubSvc()->getTaskDispatcher()->dispatch(
        topicGroupNeedMaintOfCopy);
  }
}

}  // namespace bq::md::svc
