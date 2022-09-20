/*!
 * \file ClientChannelGroup.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "ClientChannelGroup.hpp"

#include "util/Datetime.hpp"
#include "util/Logger.hpp"

namespace bq::td::srv {

void ClientChannelGroup::update(ClientChannel clientChannel) {
  const auto now = GetTotalSecSince1970();
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxChannel2ActiveTime_);
    channel2ActiveTime_[clientChannel] = now;
  }
}

void ClientChannelGroup::removeExpiredChannel() {
  const auto now = GetTotalSecSince1970();
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxChannel2ActiveTime_);
    std::ext::erase_if(channel2ActiveTime_, [this, now](const auto& rec) {
      if (now - rec.second > 300) {
        LOG_I("[{}] Channel {} expired. [activeTime = {}]", moduleName_,
              rec.first, ConvertTsToPtime(rec.second * 1000 * 1000));
        return true;
      } else {
        return false;
      }
    });
  }
}

bool ClientChannelGroup::exists(ClientChannel clientChannel) {
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxChannel2ActiveTime_);
    const auto iter = channel2ActiveTime_.find(clientChannel);
    if (iter == std::end(channel2ActiveTime_)) {
      return false;
    }
  }
  return true;
}

}  // namespace bq::td::srv
