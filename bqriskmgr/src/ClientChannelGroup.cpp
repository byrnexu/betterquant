#include "ClientChannelGroup.hpp"

#include "util/Datetime.hpp"
#include "util/Logger.hpp"

namespace bq::riskmgr {

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

}  // namespace bq::riskmgr
