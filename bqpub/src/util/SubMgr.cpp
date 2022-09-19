#include "util/SubMgr.hpp"

#include "SHMCli.hpp"
#include "SHMIPCConst.hpp"
#include "def/StatusCode.hpp"
#include "util/BQUtil.hpp"
#include "util/Logger.hpp"

namespace bq {

SubMgr::SubMgr(const std::string& appName,
               const DataRecvCallback& dataRecvCallback)
    : appName_(appName), dataRecvCallback_(dataRecvCallback) {}

void SubMgr::start() {}

void SubMgr::stop() {
  std::map<std::string, SHMCliSPtr> addr2SHMCliGroupCopy;
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxAddr2SHMCliGroup_);
    addr2SHMCliGroupCopy.insert(std::begin(addr2SHMCliGroup_),
                                std::end(addr2SHMCliGroup_));
  }
  for (const auto& rec : addr2SHMCliGroupCopy) {
    auto& shmCli = rec.second;
    shmCli->stop();
    LOG_I("Channel {} stopped.", rec.first);
  }
}

int SubMgr::sub(ClientChannel subscriber, const std::string& topic) {
  const auto internalTopic = convertTopic(topic);
  const auto topicHash =
      XXH3_64bits(internalTopic.data(), internalTopic.size());

  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxTopicHash2SubscriberGroup_);
    const auto iter = topicHash2SubscriberGroup_.find(topicHash);
    if (iter == std::end(topicHash2SubscriberGroup_)) {
      SubscriberGroup subscriberGroup;
      subscriberGroup.emplace_back(subscriber);
      topicHash2SubscriberGroup_[topicHash] = subscriberGroup;
      LOG_I("SUB {} [{}]", topic, internalTopic);
    } else {
      auto& subscriberGroup = iter->second;
      auto iterSubscriber = std::find(std::begin(subscriberGroup),
                                      std::end(subscriberGroup), subscriber);
      if (iterSubscriber == std::end(subscriberGroup)) {
        subscriberGroup.emplace_back(subscriber);
        LOG_I("SUB {} [{}]", topic, internalTopic);
      } else {
        return SCODE_BQPUB_TOPIC_ALREADY_SUB;
      }
    }
  }

  const auto ret = startSHMCliIfNotExists(internalTopic);
  return ret;
}

int SubMgr::startSHMCliIfNotExists(const std::string& topic) {
  const auto [ret, addr] = GetAddrFromTopic(appName_, topic);
  if (ret != 0) {
    return ret;
  }
  initSHMCli(addr);
  return 0;
}

void SubMgr::initSHMCli(const std::string& addr) {
  SHMCliSPtr shmCli;
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxAddr2SHMCliGroup_);
    const auto iter = addr2SHMCliGroup_.find(addr);
    if (iter != std::end(addr2SHMCliGroup_)) {
      return;
    }
    shmCli = std::make_shared<SHMCli>(addr, dataRecvCallback_);
    shmCli->setClientChannel(PUB_CHANNEL);
    addr2SHMCliGroup_.emplace(addr, shmCli);
  }
  shmCli->start();
  LOG_I("Channel {} started.", addr);
}

int SubMgr::unSub(ClientChannel subscriber, const std::string& topic) {
  const auto internalTopic = convertTopic(topic);
  const auto topicHash =
      XXH3_64bits(internalTopic.data(), internalTopic.size());
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxTopicHash2SubscriberGroup_);
    const auto iter = topicHash2SubscriberGroup_.find(topicHash);
    if (iter == std::end(topicHash2SubscriberGroup_)) {
      LOG_W("Subscriber {} not sub topic {}. [topicHash = {}]", subscriber,
            internalTopic, topicHash);
      return SCODE_BQPUB_TOPIC_NOT_SUB;
    }

    auto& subscriberGroup = iter->second;
    auto iterSubscriber = std::find(std::begin(subscriberGroup),
                                    std::end(subscriberGroup), subscriber);
    if (iterSubscriber == std::end(subscriberGroup)) {
      LOG_W("Subscriber {} not sub topic {}. [topicHash = {}]", subscriber,
            internalTopic, topicHash);
      return SCODE_BQPUB_TOPIC_NOT_SUB;
    }
    subscriberGroup.erase(iterSubscriber);
    LOG_I("UNSUB {} [{}]", topic, internalTopic);
  }
  return 0;
}

SubscriberGroup SubMgr::getSubscriberGroupByTopicHash(TopicHash topicHash) {
  SubscriberGroup ret;
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxTopicHash2SubscriberGroup_);
    const auto iter = topicHash2SubscriberGroup_.find(topicHash);
    if (iter != std::end(topicHash2SubscriberGroup_)) {
      const auto& subscriberGroup = iter->second;
      ret.assign(std::begin(subscriberGroup), std::end(subscriberGroup));
    }
  }
  return ret;
}

}  // namespace bq
