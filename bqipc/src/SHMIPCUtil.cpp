/*!
 * \file SHMIPCUtil.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "SHMIPCUtil.hpp"

#include <cassert>

#include "SHMIPCMsgId.hpp"
#include "SHMSrv.hpp"
#include "util/Logger.hpp"

namespace bq {

std::once_flag& GetOnceFlagOfAssignAppName() {
  static std::once_flag ret;
  return ret;
}

std::string TopicContent::toJson() const {
  std::string ret;
  ret = R"({"shmHeader":)" + shmHeader_.toJson();
  if (dataLen_ != 0) {
    ret = ret + R"(,"data":)" + R"(")" + data_ + R"(")";
  }
  ret = ret + "}";
  return ret;
}

void PubTopic(const SHMSrvSPtr& shmSrv, const std::string& topic,
              const std::string& data) {
  assert(shmSrv != nullptr && "shmSrv != nullptr");
  LOG_I("PUB {}, topic data is {}", topic, data);
  const auto topicHash = XXH3_64bits(topic.data(), topic.size());
  shmSrv->pushMsgWithZeroCopy(
      [&](void* shmBuf) {
        auto topicContent = static_cast<TopicContent*>(shmBuf);
        topicContent->shmHeader_.topicHash_ = topicHash;
        topicContent->dataLen_ = data.size();
        strncpy(topicContent->data_, data.c_str(), data.size());
      },
      PUB_CHANNEL, MSG_ID_ON_PUSH_TOPIC,
      sizeof(TopicContent) + data.size() + 1);
}

}  // namespace bq
