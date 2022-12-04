/*!
 * \file Config.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/26
 *
 * \brief
 */

#include "Config.hpp"

#include "MDSimConst.hpp"
#include "def/BQConst.hpp"
#include "util/BQUtil.hpp"
#include "util/Logger.hpp"
#include "util/MarketDataCond.hpp"

namespace bq::md {

int Config::afterInit(const std::string& configFilename) {
  const auto& topicGroup = node_["playbackTopicGroup"];
  for (auto iter = topicGroup.begin(); iter != topicGroup.end(); ++iter) {
    const auto topic = iter->as<std::string>();
    topicGroup_.emplace_back(topic);
  }

  for (const auto& topic : topicGroup_) {
    const auto [statusCodeOfGetMDC, marketDataCond] =
        GetMarketDataCondFromTopic(topic);
    if (statusCodeOfGetMDC != 0) {
      LOG_E("Invalid topic {} in config.", topic);
      return -1;
    }

    appName_ =
        fmt::format("{}-{}", PrefixOfAppName, TOPIC_PREFIX_OF_MARKET_DATA);

    const auto [statusCodeOfGAFT, addr] = GetAddrFromTopic(appName_, topic);
    if (statusCodeOfGAFT != 0) {
      LOG_E("Invalid topic {} in config.", topic);
      return -1;
    }
    addrOfShmSrvGroup_.emplace_back(addr);
  }

  return 0;
}

}  // namespace bq::md
