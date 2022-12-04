/*!
 * \file SHMIPCUtil.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "SHMHeader.hpp"
#include "SHMIPCConst.hpp"
#include "SHMIPCDef.hpp"
#include "util/PchBase.hpp"

namespace bq {

std::once_flag& GetOnceFlagOfAssignAppName();

template <typename T>
void InitMsgBody(void* target, const T source) {
  const auto targetAddr = static_cast<char*>(target) + sizeof(SHMHeader);
  const auto sourceAddr =
      reinterpret_cast<const char*>(&source) + sizeof(SHMHeader);
  const auto len = sizeof(T) - sizeof(SHMHeader);
  memcpy(targetAddr, sourceAddr, len);
}

struct TopicContent {
  SHMHeader shmHeader_;
  std::uint32_t dataLen_{0};
  char data_[0];
  std::string toJson() const;
};
using TopicContentSPtr = std::shared_ptr<TopicContent>;

void PubTopic(const SHMSrvSPtr& shmSrv, const std::string& topic,
              const std::string& data_);

}  // namespace bq
