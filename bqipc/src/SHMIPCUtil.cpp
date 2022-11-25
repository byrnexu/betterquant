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

namespace bq {

std::once_flag& GetOnceFlagOfAssignAppName() {
  static std::once_flag ret;
  return ret;
}

std::string TopicContent::toJson() const {
  std::string ret;
  ret = R"({"shmHeader":)" + shmHeader_.toJson() + ",";
  ret = ret + R"("data":)" + R"(")" + data_ + R"("})";
  return ret;
}

void PubTopic(const SHMSrvSPtr& shmSrv, const std::string& topic,
              const std::string& data) {
  assert(shmSrv != nullptr && "shmSrv != nullptr");
  const auto topicHash = XXH3_64bits(topic.data(), topic.size());
  shmSrv->pushMsgWithZeroCopy(
      [&](void* shmBuf) {
        auto topicContent = static_cast<TopicContent*>(shmBuf);
        topicContent->shmHeader_.topicHash_ = topicHash;
        strncpy(topicContent->data_, data.c_str(),
                sizeof(topicContent->data_) - 1);
      },
      PUB_CHANNEL, MSG_ID_ON_PUSH_TOPIC, sizeof(TopicContent));
}

}  // namespace bq
