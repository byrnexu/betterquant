/*!
 * \file ClientChannelGroup.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "SHMIPCDef.hpp"
#include "util/Pch.hpp"
#include "util/StdExt.hpp"

namespace bq::riskmgr {

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

 private:
  std::map<ClientChannel, std::uint64_t> channel2ActiveTime_;
  std::ext::spin_mutex mtxChannel2ActiveTime_;

  const std::string moduleName_;
};

}  // namespace bq::riskmgr
