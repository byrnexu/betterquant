/*!
 * \file MDSvcDef.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "MDSvcDef.hpp"

namespace bq::md::svc {

std::string TopicGroupNeedMaint::toStr() {
  const auto strNeedSub = boost::join(topicGroupNeedSub_, " ");
  const auto strNeedUnSub = boost::join(topicGroupNeedUnSub_, " ");
  if (needSubOrUnSub()) {
    const auto titleOfSub = "=== TOPIC_GROUP_NEED_SUB ===";
    const auto titleOfUnSub = "=== TOPIC_GROUP_NEED_UNSUB ===";
    const auto ret = fmt::format(
        "\n{} {}\n{}\n{} {}\n{}", titleOfSub, topicGroupNeedSub_.size(),
        strNeedSub, titleOfUnSub, topicGroupNeedUnSub_.size(), strNeedUnSub);
    return ret;
  }
  return "";
}

bool TopicGroupNeedMaint::needSubOrUnSub() const {
  return !topicGroupNeedSub_.empty() || !topicGroupNeedUnSub_.empty();
}

}  // namespace bq::md::svc
