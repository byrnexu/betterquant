/*!
 * \file MDPlayback.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/26
 *
 * \brief
 */

#pragma once

#include "def/BQConst.hpp"
#include "util/Pch.hpp"

namespace bq::md {

struct MarketDataOfSim;
using MarketDataOfSimSPtr = std::shared_ptr<MarketDataOfSim>;

using Ts2MarketDataOfSimGroup =
    std::multimap<std::uint64_t, MarketDataOfSimSPtr>;
using Ts2MarketDataOfSimGroupSPtr = std::shared_ptr<Ts2MarketDataOfSimGroup>;

class MDSim;

class MDPlayback;
using MDPlaybackSPtr = std::shared_ptr<MDPlayback>;

class MDPlayback {
 public:
  MDPlayback(const MDPlayback&) = delete;
  MDPlayback& operator=(const MDPlayback&) = delete;
  MDPlayback(const MDPlayback&&) = delete;
  MDPlayback& operator=(const MDPlayback&&) = delete;

  explicit MDPlayback(MDSim* mdSim) : mdSim_(mdSim) {}

 public:
  void start();
  void stop();

 private:
  void playback();
  void notifySubscribersSimedMDWillBeSend();
  void playback(const Ts2MarketDataOfSimGroupSPtr& ts2MarketDataOfSimGroup);

 private:
  MDSim* mdSim_{nullptr};

  std::atomic_bool keepRunning_{true};
  std::unique_ptr<std::thread> threadPlayback_{nullptr};
  std::uint32_t playbackSpeed_{1};
};

}  // namespace bq::md
