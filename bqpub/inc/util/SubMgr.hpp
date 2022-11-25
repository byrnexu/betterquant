/*!
 * \file SubMgr.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "SHMIPCDef.hpp"
#include "def/Def.hpp"
#include "util/Pch.hpp"
#include "util/StdExt.hpp"

namespace bq {

using SubscriberGroup = std::vector<ClientChannel>;
using TopicHash2SubscriberGroup =
    absl::node_hash_map<TopicHash, SubscriberGroup>;

class SubMgr {
 public:
  SubMgr(const SubMgr&) = delete;
  SubMgr& operator=(const SubMgr&) = delete;
  SubMgr(const SubMgr&&) = delete;
  SubMgr& operator=(const SubMgr&&) = delete;

  SubMgr(const std::string& appNameOfSubscriber,
         const DataRecvCallback& dataRecvCallback);
  ~SubMgr();

 public:
  int sub(ClientChannel subscriber, const std::string& topic);
  int unSub(ClientChannel subscriber, const std::string& topic);

 private:
  int startSHMCliIfNotExists(const std::string& topic);

 public:
  SubscriberGroup getSubscriberGroupByTopicHash(TopicHash topicHash);

 private:
  void initSHMCli(const std::string& addr);

 private:
  std::string appNameOfSubscriber_;
  DataRecvCallback dataRecvCallback_{nullptr};

  TopicHash2SubscriberGroup topicHash2SubscriberGroup_;
  std::ext::spin_mutex mtxTopicHash2SubscriberGroup_;

  std::map<std::string, SHMCliSPtr> addr2SHMCliGroup_;
  std::ext::spin_mutex mtxAddr2SHMCliGroup_;
};

}  // namespace bq
