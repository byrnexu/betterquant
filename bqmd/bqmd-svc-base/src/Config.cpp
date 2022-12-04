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

std::tuple<int, TopicGroupSPtr, TopicGroupSPtr>
Config::refreshTopicGroupMustSubAndSave() {
  auto [ret, topicGroupMustSubInAdvance, topicGroupMustSave] =
      getTopicGroupMustSubAndSaveInConf("topicGroupMustSubInAdvance");
  if (ret != 0) {
    LOG_W("Get topic group must save to disk failed.");
    return {-1, topicGroupMustSubInAdvance, topicGroupMustSave};
  }

  if (*topicGroupMustSubInAdvance != *topicGroupMustSubInAdvance_) {
    topicGroupMustSubInAdvance_ = topicGroupMustSubInAdvance;
    {
      std::lock_guard<std::mutex> guard(mtxCacheOfTopicGroupMustSubInAdvance_);
      cacheOfTopicGroupMustSubInAdvance_ =
          std::make_shared<TopicGroup>(*topicGroupMustSubInAdvance_);
    }
  }

  if (*topicGroupMustSave != *topicGroupMustSave_) {
    topicGroupMustSave_ = topicGroupMustSave;
    {
      std::lock_guard<std::mutex> guard(mtxCacheOfTopicGroupMustSave_);
      cacheOfTopicGroupMustSave_ =
          std::make_shared<TopicGroup>(*topicGroupMustSave_);
    }
  }

  return {0, topicGroupMustSubInAdvance, topicGroupMustSave};
}

std::tuple<int, TopicGroupSPtr> Config::refreshTopicGroupInBlackList() {
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

bool Config::topicMustSaveToDisk(const std::string& topic) const {
  {
    std::lock_guard<std::mutex> guard(mtxCacheOfTopicGroupMustSave_);
    const auto iter = cacheOfTopicGroupMustSave_->find(topic);
    return iter != std::end(*cacheOfTopicGroupMustSave_);
  }
}

bool Config::topicInBlackList(const std::string& topic) const {
  {
    std::lock_guard<std::mutex> guard(mtxCacheOfTopicGroupInBlackList_);
    const auto iter = cacheOfTopicGroupInBlackList_->find(topic);
    return iter != std::end(*cacheOfTopicGroupInBlackList_);
  }
}

std::tuple<int, TopicGroupSPtr, TopicGroupSPtr>
Config::getTopicGroupMustSubAndSaveInConf(const std::string& nodeName) const {
  auto emptyTopicGroup = std::make_shared<TopicGroup>();
  try {
    const auto configFilename = node_[nodeName].as<std::string>();
    const auto [ret, config] = InitConfig(configFilename);
    if (ret != 0) {
      LOG_W("Get topic of {} failed.", nodeName);
      return {-1, emptyTopicGroup, emptyTopicGroup};
    }

    const auto marketCode = node_["marketCode"].as<std::string>();
    const auto symbolType = node_["symbolType"].as<std::string>();
    const auto prefix =
        fmt::format("{}{}{}{}{}", TOPIC_PREFIX_OF_MARKET_DATA, SEP_OF_TOPIC,
                    marketCode, SEP_OF_TOPIC, symbolType);

    auto topicGroupMustSubInAdvance = std::make_shared<TopicGroup>();
    auto topicGroupMustSave = std::make_shared<TopicGroup>();

    const auto& topicGroup = config["topicGroup"];
    for (auto iter = topicGroup.begin(); iter != topicGroup.end(); ++iter) {
      const auto name = (*iter)["name"].as<std::string>();
      const auto topic = fmt::format("{}{}{}", prefix, SEP_OF_TOPIC, name);
      topicGroupMustSubInAdvance->emplace(topic);
      if ((*iter)["saveToDisk"].as<bool>()) {
        topicGroupMustSave->emplace(topic);
      }
    }
    return {0, topicGroupMustSubInAdvance, topicGroupMustSave};

  } catch (const std::exception& e) {
    LOG_W("Get topic of {} failed. [{}]", nodeName, e.what());
    return {-1, emptyTopicGroup, emptyTopicGroup};
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
