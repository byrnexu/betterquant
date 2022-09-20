/*!
 * \file TopicGroupMustSubMaint.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "MDSvcDef.hpp"
#include "util/Datetime.hpp"
#include "util/Pch.hpp"

namespace bq {
class Scheduler;
using SchedulerSPtr = std::shared_ptr<Scheduler>;
}  // namespace bq

namespace bq::md::svc {

using Topic2ActiveTimeGroup = std::map<std::string, std::uint64_t>;

class MDSvc;

class TopicGroupMustSubMaint {
 public:
  TopicGroupMustSubMaint(const TopicGroupMustSubMaint&) = delete;
  TopicGroupMustSubMaint& operator=(const TopicGroupMustSubMaint&) = delete;
  TopicGroupMustSubMaint(const TopicGroupMustSubMaint&&) = delete;
  TopicGroupMustSubMaint& operator=(const TopicGroupMustSubMaint&&) = delete;

  explicit TopicGroupMustSubMaint(MDSvc* const mdSvc) : mdSvc_(mdSvc) {}

 public:
  int start();

 private:
  int execTopicGroupMustSubMaint();

 public:
  void removeTopicForSubAgain(const std::string& topic);
  void addTopicForUnSubAgain(const std::string& topic);

 public:
  void addTopicOfSubscriber(const std::string& topic);
  void removeTopicOfSubscriber(const std::string& topic);
  TopicGroup getTopicOfSubscriber() const;

 private:
  TopicGroup addTopicGroupMustSub(const TopicGroup& origTopicGroup,
                                  const TopicGroup& topicGroupMustSub);
  TopicGroup removeTopicInBlackList(const TopicGroup& origTopicGroup,
                                    const TopicGroup& topicGroupInBlackList);
  TopicGroup mergeSameTypeTopicToAvoidDupSub(const TopicGroup& origTopicGroup);

  void removeTopicThatHaveNotRecvMDForALongTimeForReSub();
  std::tuple<int, std::map<std::string, std::uint32_t>>
  loadTopic2ThresholdOfReSubGroup();

  TopicGroupNeedMaintSPtr getTopicGroupNeedMaint(
      const TopicGroup& topicGroupMustSub) const;

  void updateTopicGroupAlreadySub(const TopicGroup& topicGroupMustSubOfAll);

 public:
  void clearTopicGroupAlreadySub();
  void updateTopicActiveTime(const std::string& topic,
                             const std::uint64_t value = GetTotalMSSince1970());

 public:
  void stop();

 private:
  MDSvc* const mdSvc_;

  SchedulerSPtr schedulerTopicGroupMustSubMaint_{nullptr};

  TopicGroup topicGroupOfSubscriber_;
  mutable std::mutex mtxTopicGroupOfSubscriber_;

  Topic2ActiveTimeGroup topicGroupAlreadySub_;
  mutable std::mutex mtxTopicGroupAlreadySub_;
};

}  // namespace bq::md::svc
