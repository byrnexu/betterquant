/*!
 * \file MDPlayback.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/26
 *
 * \brief
 */

#include "MDPlayback.hpp"

#include "Config.hpp"
#include "MDCache.hpp"
#include "MDSim.hpp"
#include "MDSimConst.hpp"
#include "MDSimDef.hpp"
#include "SHMHeader.hpp"
#include "SHMIPCUtil.hpp"
#include "SHMSrv.hpp"
#include "def/Def.hpp"
#include "util/BQUtil.hpp"
#include "util/Datetime.hpp"
#include "util/Logger.hpp"

namespace bq::md {

void MDPlayback::start() {
  keepRunning_.store(true);
  threadPlayback_ = std::make_unique<std::thread>([this]() { playback(); });
}

void MDPlayback::stop() {
  keepRunning_.store(false);
  if (threadPlayback_->joinable()) {
    threadPlayback_->join();
  }
}

void MDPlayback::playback() {
  const auto intervalBetweenCacheCheck =
      CONFIG["milliSecIntervalBetweenCacheCheck"].as<std::uint32_t>(1);

  notifySubscribersSimedMDWillBeSend();
  while (keepRunning_.load()) {
    const auto ts2MarketDataOfSimGroup = mdSim_->getMDCache()->pop();
    if (ts2MarketDataOfSimGroup == nullptr) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(intervalBetweenCacheCheck));
    } else {
      playback(ts2MarketDataOfSimGroup);
    }
  }
}

void MDPlayback::notifySubscribersSimedMDWillBeSend() {
  const auto shmSrvGroup = mdSim_->getSHMSrvGroup();
  for (const auto& rec : shmSrvGroup) {
    const auto& addrOfSHMSrv = rec.first;
    const auto& shmSrv = rec.second;
    const auto [statusCode, channel] = GetChannelFromAddr(addrOfSHMSrv);
    if (statusCode != 0) {
      continue;
    }
    const auto topic =
        fmt::format("{}{}{}-{}", channel, SEP_OF_TOPIC, PrefixOfAppName,
                    TOPIC_PREFIX_OF_MARKET_DATA);
    const auto topicData = fmt::format("{} will be started.", channel);
    PubTopic(shmSrv, topic, topicData);
  }
}

void MDPlayback::playback(
    const Ts2MarketDataOfSimGroupSPtr& ts2MarketDataOfSimGroup) {
  if (ts2MarketDataOfSimGroup->empty()) return;

  auto playbackSpeed = CONFIG["playbackSpeed"].as<std::uint32_t>(1);
  if (playbackSpeed == 0) playbackSpeed = UINT32_MAX;

  LOG_I("Begin to playback {} num of market data between {} - {}",
        ts2MarketDataOfSimGroup->size(),
        ConvertTsToPtime(std::begin(*ts2MarketDataOfSimGroup)->first),
        ConvertTsToPtime(std::rbegin(*ts2MarketDataOfSimGroup)->first));

  for (const auto& rec : *ts2MarketDataOfSimGroup) {
    if (keepRunning_.load() == false) break;

    const auto& marketDataOfSim = rec.second;
    const auto topic = static_cast<SHMHeader*>(marketDataOfSim->data_)->topic_;
    const auto [statusCode, addrOfSHMSrv] =
        GetAddrFromTopic(Config::get_const_instance().getAppName(), topic);
    if (statusCode != 0) {
      LOG_W("Get addr from topic {} failed.", topic);
      continue;
    }

    const auto shmSrv = mdSim_->getSHMSrv(addrOfSHMSrv);
    if (shmSrv == nullptr) {
      LOG_W("Get shmSrv from addr {} failed.", addrOfSHMSrv);
      continue;
    }

    const auto delay = marketDataOfSim->delay_ / playbackSpeed / 1000;
    LOG_T("Playback topic {} of ts {} after {} ms.", topic, rec.first, delay);
    if (delay != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }

    const auto msgId = static_cast<SHMHeader*>(marketDataOfSim->data_)->msgId_;
    shmSrv->pushMsgWithZeroCopy(
        [&](void* shmBuf) {
          memcpy(shmBuf, marketDataOfSim->data_, marketDataOfSim->dataLen_);
        },
        PUB_CHANNEL, msgId, marketDataOfSim->dataLen_);
  }
}

}  // namespace bq::md
