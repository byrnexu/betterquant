/*!
 * \file SubAndUnSubSvc.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "SubAndUnSubSvc.hpp"

#include "Config.hpp"
#include "ConnMetadata.hpp"
#include "MDSvc.hpp"
#include "ReqParser.hpp"
#include "TopicGroupMustSubMaint.hpp"
#include "WSCli.hpp"
#include "WSCliOfExch.hpp"
#include "util/BQMDUtil.hpp"
#include "util/Datetime.hpp"
#include "util/FlowCtrlSvc.hpp"
#include "util/Literal.hpp"
#include "util/String.hpp"
#include "util/TaskDispatcher.hpp"

namespace bq::md::svc {

SubAndUnSubSvc::SubAndUnSubSvc(MDSvc const* mdSvc) : mdSvc_(mdSvc) {}

int SubAndUnSubSvc::init() {
  const auto subAndUnSubSvcParamInStrFmt =
      SetParam(DEFAULT_TASK_DISPATCHER_PARAM,
               CONFIG["subAndUnSubSvcParam"].as<std::string>());
  const auto [ret, subAndUnSubSvcParam] =
      MakeTaskDispatcherParam(subAndUnSubSvcParamInStrFmt);
  if (ret != 0) {
    LOG_E("Init failed. {}", subAndUnSubSvcParamInStrFmt);
    return ret;
  }

  taskDispatcher_ = std::make_shared<TaskDispatcher<TopicGroupNeedMaintSPtr>>(
      subAndUnSubSvcParam,
      [](auto& task) {
        const auto asyncTask =
            std::make_shared<SubAndUnSubAsyncTask>(task, std::any());
        return std::make_tuple(0, asyncTask);
      },
      [](auto& asyncTask, auto taskSpecificThreadPoolSize) {
        return ThreadNo(0);
      },
      [this](auto& asyncTask) { handleAsyncTask(asyncTask); });

  getTaskDispatcher()->init();
  return ret;
}

void SubAndUnSubSvc::start() { getTaskDispatcher()->start(); }
void SubAndUnSubSvc::stop() { getTaskDispatcher()->stop(); }

void SubAndUnSubSvc::handleAsyncTask(SubAndUnSubAsyncTaskSPtr& asyncTask) {
  if (asyncTask->task_->needSubOrUnSub()) {
    handleTaskOfSubAndUnSub(asyncTask);
  }

  if (!asyncTask->task_->topic2ActiveTimeGroup_.empty()) {
    handleTaskOfUpdateActiveTime(asyncTask);
  }
}

void SubAndUnSubSvc::handleTaskOfSubAndUnSub(
    SubAndUnSubAsyncTaskSPtr& asyncTask) {
  const auto [subReqGroup, unSubReqGroup] =
      convertTopicToWSReq(asyncTask->task_);
  doSubOrUnSub(subReqGroup, TopicOP::Sub);
  doSubOrUnSub(unSubReqGroup, TopicOP::UnSub);
}

void SubAndUnSubSvc::doSubOrUnSub(const WSReqGroup& reqGroup, TopicOP topicOP) {
  const auto taskType = std::string(magic_enum::enum_name(topicOP));
  for (const auto& req : reqGroup) {
    const auto exceedFlowCtrl =
        mdSvc_->getFlowCtrlSvc()->exceedFlowCtrl(taskType, WriteLog::False);
    if (exceedFlowCtrl) {
      updateTopicGroupForSubOrUnSubAgain(req, topicOP);
      continue;
    }
    const auto connStatus =
        mdSvc_->getWSCliOfExch()->getWSCli()->getMetadata()->getStatus();
    if (connStatus != "Open") {
      updateTopicGroupForSubOrUnSubAgain(req, topicOP);
      continue;
    }
    LOG_D("Send req of {}. {}", taskType, req);
    const auto ret = mdSvc_->getWSCliOfExch()->getWSCli()->send(req);
    if (ret != 0) {
      updateTopicGroupForSubOrUnSubAgain(req, topicOP);
      continue;
    }
  }
}

void SubAndUnSubSvc::updateTopicGroupForSubOrUnSubAgain(const std::string& req,
                                                        TopicOP topicOP) {
  const auto topicGroup =
      mdSvc_->getReqParser()->getTopicGroupForSubOrUnSubAgain(req);
  if (topicOP == TopicOP::Sub) {
    for (const auto& topic : topicGroup) {
      mdSvc_->getTopicGroupMustSubMaint()->removeTopicForSubAgain(topic);
    }
  } else {
    for (const auto& topic : topicGroup) {
      mdSvc_->getTopicGroupMustSubMaint()->addTopicForUnSubAgain(topic);
    }
  }
}

void SubAndUnSubSvc::handleTaskOfUpdateActiveTime(
    SubAndUnSubAsyncTaskSPtr& asyncTask) {
  const auto& topic2ActiveTimeGroup = asyncTask->task_->topic2ActiveTimeGroup_;

  std::map<std::string, std::uint64_t> topic2ActiveTimeGroupAfterMerge;
  for (const auto& rec : topic2ActiveTimeGroup) {
    const auto& topic = rec.first;
    const auto topicAfterRemoveDepth = RemoveDepthInTopicOfBooks(topic);
    const auto activeTime = rec.second;
    const auto iter =
        topic2ActiveTimeGroupAfterMerge.find(topicAfterRemoveDepth);
    if (iter == std::end(topic2ActiveTimeGroupAfterMerge)) {
      topic2ActiveTimeGroupAfterMerge[topicAfterRemoveDepth] = activeTime;
    } else {
      if (activeTime > iter->second) {
        topic2ActiveTimeGroupAfterMerge[topicAfterRemoveDepth] = activeTime;
      }
    }
  }

  for (const auto& rec : topic2ActiveTimeGroupAfterMerge) {
    mdSvc_->getTopicGroupMustSubMaint()->updateTopicActiveTime(rec.first,
                                                               rec.second);
#ifndef NDEBUG
    LOG_I("Update active time of topic {} to {}, delay {}s.", rec.first,
          ConvertTsToPtime(rec.second * 1000),
          static_cast<double>(GetTotalMSSince1970() - rec.second) / 1000.0);
#endif
  }
}

}  // namespace bq::md::svc
