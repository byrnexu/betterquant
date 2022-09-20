/*!
 * \file MDSvcDef.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "def/BQDef.hpp"
#include "util/Datetime.hpp"
#include "util/Pch.hpp"

namespace bq::md::svc {

struct TopicGroupNeedMaint {
  TopicGroupNeedMaint(const TopicGroupNeedMaint&) = delete;
  TopicGroupNeedMaint& operator=(const TopicGroupNeedMaint&) = delete;
  TopicGroupNeedMaint(const TopicGroupNeedMaint&&) = delete;
  TopicGroupNeedMaint& operator=(const TopicGroupNeedMaint&&) = delete;

  TopicGroupNeedMaint() = default;
  explicit TopicGroupNeedMaint(std::uint64_t ts) : checkTs_(ts) {}

  std::string toStr();
  bool needSubOrUnSub() const;

  TopicGroup topicGroupNeedSub_;
  TopicGroup topicGroupNeedUnSub_;

  std::map<std::string, std::uint64_t> topic2ActiveTimeGroup_;
  std::uint64_t checkTs_{GetTotalMSSince1970()};
};
using TopicGroupNeedMaintSPtr = std::shared_ptr<TopicGroupNeedMaint>;

}  // namespace bq::md::svc
