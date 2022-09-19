#include "SHMIPCBase.hpp"

#include "SHMHeader.hpp"
#include "SHMIPCConst.hpp"
#include "SHMUtil.hpp"
#include "util/Logger.hpp"

namespace bq {

SHMIPCBase::SHMIPCBase(const std::string& addr,
                       const DataRecvCallback& dataRecvCallback) {
  std::vector<std::string> fieldGroup;
  boost::split(fieldGroup, addr, boost::is_any_of(SEP_OF_SHM_SVC));
  appName_ = fieldGroup[0];
  service_ = fieldGroup[1];
  instance_ = fieldGroup[2];
  event_ = fieldGroup[3];
  dataRecvCallback_ = dataRecvCallback;
  keepRunning_ = true;
  std::call_once(GetOnceFlagOfAssignAppName(), [this]() {
    iox::cxx::string<64> name(iox::cxx::TruncateToCapacity, appName_);
    iox::runtime::PoshRuntime::initRuntime(name);
  });
}

SHMIPCBase::SHMIPCBase(const std::string& appName, const std::string& service,
                       const std::string& instance, const std::string& event,
                       const DataRecvCallback& dataRecvCallback)
    : appName_(appName),
      service_(service),
      instance_(instance),
      event_(event),
      dataRecvCallback_(dataRecvCallback),
      keepRunning_(true) {
  std::call_once(GetOnceFlagOfAssignAppName(), [this]() {
    iox::cxx::string<64> name(iox::cxx::TruncateToCapacity, appName_);
    iox::runtime::PoshRuntime::initRuntime(name);
  });
}

void SHMIPCBase::init() {
  keepRunning_.store(true);
  beforeInit();

  instanceWithIdentity_ = instance_;
  if (clientChannel_ != std::nullopt) {
    instanceWithIdentity_ = fmt::format("{}-{}", instance_, *clientChannel_);
  }
  subscriberName_ = fmt::format("{}{}{}{}{}", service_, SEP_OF_SHM_SVC,
                                instanceWithIdentity_, SEP_OF_SHM_SVC, event_);

  subscriber_ = new iox::popo::UntypedSubscriber(
      {iox::capro::IdString_t(iox::cxx::TruncateToCapacity, service_),
       iox::capro::IdString_t(iox::cxx::TruncateToCapacity,
                              instanceWithIdentity_),
       iox::capro::IdString_t(iox::cxx::TruncateToCapacity, event_)},
      makeSubscriberOptions());
  subscriber_->subscribe();
  LOG_I("Create subscriber. {} [{}]", appName_, subscriberName_);

  shutdownTrigger_ = new iox::popo::UserTrigger();
  waitset_ = new iox::popo::WaitSet<>();
  waitset_->attachEvent(*shutdownTrigger_).or_else([this](auto) {
    LOG_E("Failed to attach shutdown trigger. {} [{}]", appName_,
          subscriberName_);
  });

  waitset_
      ->attachEvent(
          *subscriber_, iox::popo::SubscriberEvent::DATA_RECEIVED,
          iox::popo::createNotificationCallback(dataInSHMRecvCallback, *this))
      .or_else([this](auto) {
        LOG_E("Failed to attach subscriber. {} [{}]", appName_,
              subscriberName_);
      });

  afterInit();
}

void SHMIPCBase::dataInSHMRecvCallback(
    iox::popo::UntypedSubscriber* const subscriber, SHMIPCBase* self) {
  while (self->subscriber_->hasData()) {
    self->subscriber_->take()
        .and_then([&](const void* userPayload) {
          auto chunkHeader =
              iox::mepoo::ChunkHeader::fromUserPayload(userPayload);
          if (self->dataRecvCallback_) {
            self->dataRecvCallback_(userPayload,
                                    chunkHeader->userPayloadSize());
          }
          subscriber->release(userPayload);
        })
        .or_else([self](auto& result) {
          if (result != iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE) {
            LOG_E("Error receiving chunk. {} [{}]", self->appName_,
                  self->subscriberName_);
          }
        });
  }
}

void SHMIPCBase::uninit() {
  beforeUninit();
  if (subscriber_) {
    delete subscriber_;
    subscriber_ = nullptr;
  }
  if (waitset_) {
    delete waitset_;
    waitset_ = nullptr;
  }
  if (shutdownTrigger_) {
    delete shutdownTrigger_;
    shutdownTrigger_ = nullptr;
  }
}

void SHMIPCBase::start() {
  init();
  LOG_I("Start SHM IPC Channel. {} [{}]", appName_, subscriberName_);
  futureDataInSHMRecv_ =
      std::async(std::launch::async, [this]() { startDataInSHMRecvThread(); });
}

void SHMIPCBase::startDataInSHMRecvThread() {
  while (keepRunning_.load()) {
    auto notificationVector = waitset_->wait();
    for (auto& notification : notificationVector) {
      (*notification)();
    }
  }
}

void SHMIPCBase::stop() {
  LOG_I("Stop SHM IPC Channel. {} [{}]", appName_, subscriberName_);
  keepRunning_.store(false);
  shutdownTrigger_->trigger();
  waitForDataInSHMRecvThreadToEnd();
  uninit();
}

void SHMIPCBase::waitForDataInSHMRecvThreadToEnd() {
  if (futureDataInSHMRecv_.valid()) {
    try {
      futureDataInSHMRecv_.get();
    } catch (const std::exception& e) {
      LOG_W("Stop io exception. {} [{}] [{}]", appName_, subscriberName_,
            e.what());
    }
  } else {
    LOG_W("Invalid future of data in recv thread. {} [{}]", appName_,
          subscriberName_);
  }
}

iox::popo::PublisherOptions SHMIPCBase::makePublisherOptions() const {
  iox::popo::PublisherOptions publisherOptions;
  publisherOptions.offerOnCreate = false;
  publisherOptions.historyCapacity = 16U;
  publisherOptions.subscriberTooSlowPolicy =
      iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;
  return publisherOptions;
}

iox::popo::SubscriberOptions SHMIPCBase::makeSubscriberOptions() const {
  iox::popo::SubscriberOptions subscriberOptions;
  subscriberOptions.subscribeOnCreate = false;
  subscriberOptions.queueCapacity = 16U;
  subscriberOptions.queueFullPolicy =
      iox::popo::QueueFullPolicy::BLOCK_PRODUCER;
  return subscriberOptions;
}

}  // namespace bq
