#pragma once

#include <iceoryx_hoofs/cxx/string.hpp>
#include <iceoryx_hoofs/posix_wrapper/signal_watcher.hpp>
#include <iceoryx_posh/popo/untyped_publisher.hpp>
#include <iceoryx_posh/popo/untyped_subscriber.hpp>
#include <iceoryx_posh/popo/user_trigger.hpp>
#include <iceoryx_posh/runtime/posh_runtime.hpp>

#include "SHMHeader.hpp"
#include "SHMIPCDef.hpp"
#include "util/Pch.hpp"

namespace bq {

class SHMIPCBase;
using SHMIPCBaseSPtr = std::shared_ptr<SHMIPCBase>;

class SHMIPCBase {
 public:
  SHMIPCBase(const SHMIPCBase&) = delete;
  SHMIPCBase& operator=(const SHMIPCBase&) = delete;
  SHMIPCBase(const SHMIPCBase&&) = delete;
  SHMIPCBase& operator=(const SHMIPCBase&&) = delete;

  SHMIPCBase(const std::string& addr, const DataRecvCallback& dataRecvCallback);
  SHMIPCBase(const std::string& appName, const std::string& service,
             const std::string& instance, const std::string& event,
             const DataRecvCallback& dataRecvCallback);

 private:
  virtual void beforeInit() {}
  void init();
  virtual void afterInit() {}

 private:
  static void dataInSHMRecvCallback(
      iox::popo::UntypedSubscriber* const subscriber, SHMIPCBase* self);

 private:
  virtual void beforeUninit() {}
  void uninit();

 public:
  void start();

 private:
  void startDataInSHMRecvThread();

 public:
  void stop();

 private:
  void waitForDataInSHMRecvThreadToEnd();

 protected:
  iox::popo::PublisherOptions makePublisherOptions() const;
  iox::popo::SubscriberOptions makeSubscriberOptions() const;

 protected:
  std::string appName_;
  std::string service_;
  std::string instance_;
  std::string event_;

  std::optional<ClientChannel> clientChannel_{std::nullopt};
  std::string instanceWithIdentity_;

  std::string subscriberName_;

 private:
  DataRecvCallback dataRecvCallback_{nullptr};
  std::atomic_bool keepRunning_{true};

  iox::popo::UntypedSubscriber* subscriber_{nullptr};
  iox::popo::UserTrigger* shutdownTrigger_{nullptr};
  iox::popo::WaitSet<>* waitset_{nullptr};

  std::future<void> futureDataInSHMRecv_;
};

}  // namespace bq
