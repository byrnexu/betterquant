#pragma once

#include "SHMIPCDef.hpp"
#include "util/Pch.hpp"
#include "util/StdExt.hpp"

namespace bq::td::srv {

class ClientChannelGroup {
 public:
  ClientChannelGroup(const ClientChannelGroup&) = delete;
  ClientChannelGroup& operator=(const ClientChannelGroup&) = delete;
  ClientChannelGroup(const ClientChannelGroup&&) = delete;
  ClientChannelGroup& operator=(const ClientChannelGroup&&) = delete;

  explicit ClientChannelGroup(const std::string moduleName)
      : moduleName_(moduleName) {}

  void update(ClientChannel clientChannel);
  void removeExpiredChannel();

  bool exists(ClientChannel clientChannel);

 private:
  std::map<ClientChannel, std::uint64_t> channel2ActiveTime_;
  std::ext::spin_mutex mtxChannel2ActiveTime_;

  const std::string moduleName_;
};

}  // namespace bq::td::srv
