/*!
 * \file SHMHeader.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "SHMHeader.hpp"

#include "SHMIPCMsgId.hpp"
#include "util/Datetime.hpp"

namespace bq {

std::string SHMHeader::toStr() const {
  const auto topicName = strlen(topicName_) == 0 ? "empty" : topicName_;
  const auto ret = fmt::format(
      "{} Channel: {} {} {} topicHash: {} topicName: {}", GetMsgName(msgId_),
      clientChannel_, magic_enum::enum_name(direction_),
      ConvertTsToPtime(timestamp_), topicHash_, topicName);
  return ret;
}

}  // namespace bq
