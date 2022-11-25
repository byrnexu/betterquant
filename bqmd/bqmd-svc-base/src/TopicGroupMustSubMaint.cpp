/*!
 * \file TopicGroupMustSubMaint.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "TopicGroupMustSubMaint.hpp"

#include "Config.hpp"
#include "MDSvc.hpp"
#include "SubAndUnSubSvc.hpp"
#include "def/BQConst.hpp"
#include "def/Const.hpp"
#include "util/BQMDUtil.hpp"
#include "util/Logger.hpp"
#include "util/Scheduler.hpp"
#include "util/StdExt.hpp"
#include "util/TaskDispatcher.hpp"
#include "util/Util.hpp"

namespace bq::md::svc {

int TopicGroupMustSubMaint::start() {
  const auto milliSecIntervalOfTopicGroupMustSubMaint =
      CONFIG["milliSecIntervalOfTopicGroupMustSubMaint"].as<std::uint32_t>();
  schedulerTopicGroupMustSubMaint_ = std::make_shared<Scheduler>(
      "TOPIC_GROUP_MUST_MAINT", [this]() { execTopicGroupMustSubMaint(); },
      milliSecIntervalOfTopicGroupMustSubMaint);

  execTopicGroupMustSubMaint();

  auto ret = schedulerTopicGroupMustSubMaint_->start();
  if (ret != 0) {
    LOG_E("Start topic group must sub maintainment failed.");
    return ret;
  }

  LOG_D("Start topic group must sub maintainment success.");
  return 0;
}

int TopicGroupMustSubMaint::execTopicGroupMustSubMaint() {
  LOG_T("Begin to exec topic group must sub maintainment.");

  auto [retOfMustSubInAdvance, topicGroupMustSubInAdvance, topicGroupMustSave] =
      Config::get_mutable_instance().refreshTopicGroupMustSubAndSave();
  if (retOfMustSubInAdvance != 0) {
    LOG_W("Exec topic group must sub maintainment failed.");
    return retOfMustSubInAdvance;
  }

  auto [retOfInBlackList, topicGroupInBlackList] =
      Config::get_mutable_instance().refreshTopicGroupInBlackList();
  if (retOfInBlackList != 0) {
    LOG_W("Exec topic group must sub maintainment failed.");
    return retOfInBlackList;
  }

  TopicGroup topicGroupMustSubOfAll = getTopicOfSubscriber();

  topicGroupMustSubOfAll =
      addTopicGroupMustSub(topicGroupMustSubOfAll, *topicGroupMustSubInAdvance);

  topicGroupMustSubOfAll =
      removeTopicInBlackList(topicGroupMustSubOfAll, *topicGroupInBlackList);

  topicGroupMustSubOfAll =
      mergeSameTypeTopicToAvoidDupSub(topicGroupMustSubOfAll);

  removeTopicThatHaveNotRecvMDForALongTimeForReSub();

  auto topicGroupNeedMaint = getTopicGroupNeedMaint(topicGroupMustSubOfAll);

  if (topicGroupNeedMaint->needSubOrUnSub()) {
    LOG_I("Find topic need to be sub or unsub. {}",
          topicGroupNeedMaint->toStr());
    updateTopicGroupAlreadySub(topicGroupMustSubOfAll);
    mdSvc_->getSubAndUnSubSvc()->getTaskDispatcher()->dispatch(
        topicGroupNeedMaint);
  }

  return 0;
}

void TopicGroupMustSubMaint::removeTopicForSubAgain(const std::string& topic) {
  {
    std::lock_guard<std::mutex> guard(mtxTopicGroupAlreadySub_);
    topicGroupAlreadySub_.erase(topic);
  }
}

void TopicGroupMustSubMaint::addTopicForUnSubAgain(const std::string& topic) {
  const auto now = GetTotalMSSince1970();
  {
    std::lock_guard<std::mutex> guard(mtxTopicGroupAlreadySub_);
    topicGroupAlreadySub_.emplace(topic, now);
  }
}

void TopicGroupMustSubMaint::addTopicOfSubscriber(const std::string& topic) {
  LOG_D("Add topic of subscriber. {}", topic);
  {
    std::lock_guard<std::mutex> guard(mtxTopicGroupOfSubscriber_);
    topicGroupOfSubscriber_.emplace(topic);
  }
}

void TopicGroupMustSubMaint::removeTopicOfSubscriber(const std::string& topic) {
  LOG_D("Remove topic of subscriber. {}", topic);
  {
    std::lock_guard<std::mutex> guard(mtxTopicGroupOfSubscriber_);
    topicGroupOfSubscriber_.erase(topic);
  }
}

TopicGroup TopicGroupMustSubMaint::getTopicOfSubscriber() const {
  {
    std::lock_guard<std::mutex> guard(mtxTopicGroupOfSubscriber_);
    return topicGroupOfSubscriber_;
  }
}

TopicGroup TopicGroupMustSubMaint::addTopicGroupMustSub(
    const TopicGroup& origTopicGroup, const TopicGroup& topicGroupMustSub) {
  TopicGroup ret;
  std::set_union(std::begin(origTopicGroup), std::end(origTopicGroup),
                 std::begin(topicGroupMustSub), std::end(topicGroupMustSub),
                 std::inserter(ret, std::begin(ret)));
  return ret;
}

TopicGroup TopicGroupMustSubMaint::removeTopicInBlackList(
    const TopicGroup& origTopicGroup, const TopicGroup& topicGroupInBlackList) {
  auto topicExistsInBlackList = [](const auto& topic,
                                   const auto& topicGroupInBlackList) {
    for (auto topicInBlackList : topicGroupInBlackList) {
      boost::erase_all(topicInBlackList, "*");
      if (boost::contains(topic, topicInBlackList)) {
        return true;
      }
    }
    return false;
  };

  TopicGroup ret;
  for (const auto& topic : origTopicGroup) {
    if (topicExistsInBlackList(topic, topicGroupInBlackList)) {
      LOG_T("Find topic {} in blacklist.", topic);
    } else {
      ret.emplace(topic);
    }
  }

  return ret;
}

TopicGroup TopicGroupMustSubMaint::mergeSameTypeTopicToAvoidDupSub(
    const TopicGroup& origTopicGroup) {
  TopicGroup ret;
  for (const auto& topic : origTopicGroup) {
    const auto t = RemoveDepthInTopicOfBooks(topic);
    ret.emplace(t);
  }
  return ret;
}

void TopicGroupMustSubMaint::
    removeTopicThatHaveNotRecvMDForALongTimeForReSub() {
  const auto [ret, topic2ThresholdOfReSubGroup] =
      loadTopic2ThresholdOfReSubGroup();
  if (ret != 0) {
    LOG_W(
        "Remove topic that have not recv market data for a long time failed.");
    return;
  }

  std::uint32_t defaultThreshold = 60000;
  const auto iter = topic2ThresholdOfReSubGroup.find("default");
  if (iter != std::end(topic2ThresholdOfReSubGroup)) {
    defaultThreshold = iter->second;
  }

  {
    std::lock_guard<std::mutex> guard(mtxTopicGroupAlreadySub_);
    for (auto iter = std::begin(topicGroupAlreadySub_);
         iter != std::end(topicGroupAlreadySub_);) {
      const auto topic = iter->first;
      const auto now = GetTotalMSSince1970();
      const auto activeTime = iter->second;
      const auto timeDurOfNotRecvMD = now - activeTime;

      auto thresholdOfReSub = defaultThreshold;
      if (const auto iter = topic2ThresholdOfReSubGroup.find(topic);
          iter != std::end(topic2ThresholdOfReSubGroup)) {
        thresholdOfReSub = iter->second;
      }

      if (timeDurOfNotRecvMD > thresholdOfReSub) {
        iter = topicGroupAlreadySub_.erase(iter);
        LOG_W(
            "Resub {} required because of time dur of not recv MD {}ms greater "
            "than threshold of resub {}ms.",
            topic, timeDurOfNotRecvMD, thresholdOfReSub);
      } else {
        ++iter;
        LOG_T("===== {} {} {}", topic, thresholdOfReSub, timeDurOfNotRecvMD);
      }
    }
  }
}

std::tuple<int, std::map<std::string, std::uint32_t>>
TopicGroupMustSubMaint::loadTopic2ThresholdOfReSubGroup() {
  std::map<std::string, std::uint32_t> topic2ThresholdOfReSubGroup;
  try {
    const auto prefix = fmt::format("{}{}{}{}{}", TOPIC_PREFIX_OF_MARKET_DATA,
                                    SEP_OF_TOPIC, mdSvc_->getMarketCode(),
                                    SEP_OF_TOPIC, mdSvc_->getSymbolType());

    const auto configFilename =
        CONFIG["thresholdOfReSubWithoutRecvMD"].as<std::string>();
    const auto [ret, config] = InitConfig(configFilename);
    if (ret != 0) {
      LOG_W("Load threshold of resub group failed.");
      return {ret, std::map<std::string, std::uint32_t>()};
    }

    const auto& thresholdOfReSub = config["thresholdOfReSub"];
    for (const auto& rec : thresholdOfReSub) {
      const auto name = rec["name"].as<std::string>();
      std::string topic = name;
      if (topic != "default") {
        topic = fmt::format("{}{}{}", prefix, SEP_OF_TOPIC, name);
      }
      const auto timeDur = rec["timeDur"].as<std::uint32_t>();
      topic2ThresholdOfReSubGroup.emplace(topic, timeDur);
    }

  } catch (const std::exception& e) {
    LOG_W("Load threshold of resub group failed. [{}]", e.what());
    return {0, std::map<std::string, std::uint32_t>()};
  }

  return {0, topic2ThresholdOfReSubGroup};
}

TopicGroupNeedMaintSPtr TopicGroupMustSubMaint::getTopicGroupNeedMaint(
    const TopicGroup& topicGroupMustSubOfAll) const {
  auto ret = std::make_shared<TopicGroupNeedMaint>();
  {
    std::lock_guard<std::mutex> guard(mtxTopicGroupAlreadySub_);
    for (const auto& topic : topicGroupMustSubOfAll) {
      if (topicGroupAlreadySub_.find(topic) ==
          std::end(topicGroupAlreadySub_)) {
        ret->topicGroupNeedSub_.emplace(topic);
      }
    }
    for (const auto& rec : topicGroupAlreadySub_) {
      const auto topic = rec.first;
      if (topicGroupMustSubOfAll.find(topic) ==
          std::end(topicGroupMustSubOfAll)) {
        ret->topicGroupNeedUnSub_.emplace(topic);
      }
    }
  }
  return ret;
}

void TopicGroupMustSubMaint::updateTopicGroupAlreadySub(
    const TopicGroup& topicGroupMustSubOfAll) {
  {
    std::lock_guard<std::mutex> guard(mtxTopicGroupAlreadySub_);
    for (const auto& topic : topicGroupMustSubOfAll) {
      if (topicGroupAlreadySub_.find(topic) ==
          std::end(topicGroupAlreadySub_)) {
        const auto now = GetTotalMSSince1970();
        topicGroupAlreadySub_.emplace(topic, now);
      }
    }
    for (auto iter = std::begin(topicGroupAlreadySub_);
         iter != std::end(topicGroupAlreadySub_);) {
      const auto topic = iter->first;
      if (topicGroupMustSubOfAll.find(topic) ==
          std::end(topicGroupMustSubOfAll)) {
        iter = topicGroupAlreadySub_.erase(iter);
      } else {
        ++iter;
      }
    }
  }
}

void TopicGroupMustSubMaint::clearTopicGroupAlreadySub() {
  {
    std::lock_guard<std::mutex> guard(mtxTopicGroupAlreadySub_);
    topicGroupAlreadySub_.clear();
  }
}

void TopicGroupMustSubMaint::updateTopicActiveTime(const std::string& topic,
                                                   std::uint64_t value) {
  {
    std::lock_guard<std::mutex> guard(mtxTopicGroupAlreadySub_);
    const auto iter = topicGroupAlreadySub_.find(topic);
    if (iter != std::end(topicGroupAlreadySub_)) {
      iter->second = value;
    } else {
      LOG_I(
          "Can not find topic {} in topic group already sub "
          "when update topic active time.",
          topic);
    }
  }
}

void TopicGroupMustSubMaint::stop() {
  schedulerTopicGroupMustSubMaint_->stop();
  LOG_D("Stop topic group must sub maintainment finished. ");
}

}  // namespace bq::md::svc
