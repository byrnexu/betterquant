#pragma once

#include "def/BQDef.hpp"
#include "util/ConfigBase.hpp"
#include "util/Pch.hpp"

namespace bq::md::svc {

class Config : public ConfigBase,
               public boost::serialization::singleton<Config> {
 public:
  std::tuple<int, TopicGroupSPtr> topicGroupMustSubInAdvance();
  std::tuple<int, TopicGroupSPtr> topicGroupInBlackList();

  bool topicMustSubInAdvance(const std::string& topic) const;
  bool topicInBlackList(const std::string& topic) const;

 private:
  std::tuple<int, TopicGroupSPtr> getTopicGroupInConf(
      const std::string& nodeName) const;

 private:
  TopicGroupSPtr topicGroupMustSubInAdvance_{std::make_shared<TopicGroup>()};
  TopicGroupSPtr cacheOfTopicGroupMustSubInAdvance_{
      std::make_shared<TopicGroup>()};
  mutable std::mutex mtxCacheOfTopicGroupMustSubInAdvance_;

  TopicGroupSPtr topicGroupInBlackList_{std::make_shared<TopicGroup>()};
  TopicGroupSPtr cacheOfTopicGroupInBlackList_{std::make_shared<TopicGroup>()};
  mutable std::mutex mtxCacheOfTopicGroupInBlackList_;
};

}  // namespace bq::md::svc
