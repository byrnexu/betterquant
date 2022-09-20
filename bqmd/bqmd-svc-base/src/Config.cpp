/*!
 * \file Config.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "Config.hpp"

#include "def/BQConst.hpp"
#include "util/Logger.hpp"

namespace bq::md::svc {

std::tuple<int, TopicGroupSPtr> Config::topicGroupMustSubInAdvance() {
  auto [ret, topicGroupMustSubInAdvance] =
      getTopicGroupInConf("topicGroupMustSubInAdvance");
  if (ret != 0) {
    LOG_W("Get topic group must save to disk failed.");
    return {-1, topicGroupMustSubInAdvance};
  }

  if (*topicGroupMustSubInAdvance == *topicGroupMustSubInAdvance_) {
    return {0, topicGroupMustSubInAdvance};
  }

  topicGroupMustSubInAdvance_ = topicGroupMustSubInAdvance;
  {
    std::lock_guard<std::mutex> guard(mtxCacheOfTopicGroupMustSubInAdvance_);
    cacheOfTopicGroupMustSubInAdvance_ =
        std::make_shared<TopicGroup>(*topicGroupMustSubInAdvance_);
  }
  return {0, topicGroupMustSubInAdvance};
}

std::tuple<int, TopicGroupSPtr> Config::topicGroupInBlackList() {
  auto [ret, topicGroupInBlackList] =
      getTopicGroupInConf("topicGroupInBlackList");
  if (ret != 0) {
    LOG_W("Get topic group in black list failed.");
    return {-1, topicGroupInBlackList};
  }

  if (*topicGroupInBlackList == *topicGroupInBlackList_) {
    return {0, topicGroupInBlackList};
  }

  topicGroupInBlackList_ = topicGroupInBlackList;
  {
    std::lock_guard<std::mutex> guard(mtxCacheOfTopicGroupInBlackList_);
    cacheOfTopicGroupInBlackList_ =
        std::make_shared<TopicGroup>(*topicGroupInBlackList_);
  }
  return {0, topicGroupInBlackList};
}

bool Config::topicMustSubInAdvance(const std::string& topic) const {
  {
    std::lock_guard<std::mutex> guard(mtxCacheOfTopicGroupMustSubInAdvance_);
    const auto iter = cacheOfTopicGroupMustSubInAdvance_->find(topic);
    return iter != std::end(*cacheOfTopicGroupMustSubInAdvance_);
  }
}

bool Config::topicInBlackList(const std::string& topic) const {
  {
    std::lock_guard<std::mutex> guard(mtxCacheOfTopicGroupInBlackList_);
    const auto iter = cacheOfTopicGroupInBlackList_->find(topic);
    return iter != std::end(*cacheOfTopicGroupInBlackList_);
  }
}

std::tuple<int, TopicGroupSPtr> Config::getTopicGroupInConf(
    const std::string& nodeName) const {
  try {
    const auto configFilename = node_[nodeName].as<std::string>();
    const auto [ret, config] = InitConfig(configFilename);
    if (ret != 0) {
      LOG_W("Get topic of {} failed.", nodeName);
      return {-1, std::make_shared<TopicGroup>()};
    }

    const auto marketCode = node_["marketCode"].as<std::string>();
    const auto symbolType = node_["symbolType"].as<std::string>();
    const auto prefix =
        fmt::format("{}{}{}{}{}", TOPIC_PREFIX_OF_MARKET_DATA, SEP_OF_TOPIC,
                    marketCode, SEP_OF_TOPIC, symbolType);

    auto topicGroupInConf = std::make_shared<TopicGroup>();
    const auto& topicGroup = config["topicGroup"];
    for (auto iter = topicGroup.begin(); iter != topicGroup.end(); ++iter) {
      const auto name = iter->as<std::string>();
      if (boost::contains(name, "*")) {
        topicGroupInConf->emplace(name);
      } else {
        const auto topic = fmt::format("{}{}{}", prefix, SEP_OF_TOPIC, name);
        topicGroupInConf->emplace(topic);
      }
    }
    return {0, topicGroupInConf};

  } catch (const std::exception& e) {
    LOG_W("Get topic of {} failed. [{}]", nodeName, e.what());
    return {-1, std::make_shared<TopicGroup>()};
  }
}

}  // namespace bq::md::svc
