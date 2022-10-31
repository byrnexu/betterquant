/*!
 * \file SHMHeader.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "SHMIPCConst.hpp"
#include "SHMIPCDef.hpp"
#include "SHMIPCMsgId.hpp"
#include "def/DefIF.hpp"
#include "util/PchBase.hpp"

namespace bq {

struct SHMHeader {
  SHMHeader() = default;
  explicit SHMHeader(MsgId msgId) : msgId_(msgId) {}

  MsgId msgId_;
  ClientChannel clientChannel_{0};
  Direction direction_;
  std::uint64_t timestamp_{0};
  TopicHash topicHash_{0};
  char topicName_[MAX_TOPIC_NAME_LEN];

  std::string toStr() const;
  std::string toJson() const;
};

}  // namespace bq
