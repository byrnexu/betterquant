/*!
 * \file Config.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/26
 *
 * \brief
 */

#pragma once

#include "util/ConfigBase.hpp"
#include "util/Pch.hpp"

namespace bq::md {

class Config : public ConfigBase,
               public boost::serialization::singleton<Config> {
 public:
  std::vector<std::string> getTopicGroup() const { return topicGroup_; }

  std::string getAppName() const { return appName_; }
  std::vector<std::string> getAddrOfSHMSrvGroup() const {
    return addrOfShmSrvGroup_;
  }

 private:
  int afterInit(const std::string& configFilename) final;

 private:
  std::vector<std::string> topicGroup_;

  std::string appName_;
  std::vector<std::string> addrOfShmSrvGroup_;
};

}  // namespace bq::md
