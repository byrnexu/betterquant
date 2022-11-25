/*!
 * \file SvcBase.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "util/SvcBase.hpp"

#include "db/DBE.hpp"
#include "util/Logger.hpp"
#include "util/Random.hpp"
#include "util/SignalHandler.hpp"

namespace bq {

SvcBase::SvcBase(const std::string& configFilename,
                 InstallSignalHandler installSignalHandler)
    : configFilename_(configFilename) {
  RandomStr::get_mutable_instance().init();
  RandomInt::get_mutable_instance().init();
  if (installSignalHandler == InstallSignalHandler::True) {
    signalHandler_ = std::make_shared<SignalHandler>(
        "SVC_BASE", [this](const boost::system::error_code& ec,
                           int signalNumber) { exit(&ec, signalNumber); });
  }
}

int SvcBase::init(const std::string& configFilename,
                  InstallSignalHandler installSignalHandler) {
  if (configFilename_.empty()) {
    configFilename_ = configFilename;
    RandomStr::get_mutable_instance().init();
    RandomInt::get_mutable_instance().init();
  }

  if (signalHandler_ == nullptr &&
      installSignalHandler == InstallSignalHandler::True) {
    signalHandler_ = std::make_shared<SignalHandler>(
        "SVC_BASE", [this](const boost::system::error_code& ec,
                           int signalNumber) { exit(&ec, signalNumber); });
  }

  if (auto ret = prepareInit(); ret != 0) {
    LOG_E("Init svc failed.");
    return ret;
  }

  if (auto ret = beforeInit(); ret != 0) {
    LOG_E("Init svc failed.");
    return ret;
  }

  if (auto ret = doInit(); ret != 0) {
    LOG_E("Init svc failed.");
    return ret;
  }

  if (auto ret = afterInit(); ret != 0) {
    LOG_E("Init svc failed.");
    return ret;
  }

  return 0;
}

int SvcBase::run() {
  if (auto ret = beforeRun(); ret != 0) {
    LOG_E("Run svc failed.");
    return ret;
  }

  if (auto ret = doRun(); ret != 0) {
    LOG_E("Run svc failed.");
    return ret;
  }

  if (auto ret = afterRun(); ret != 0) {
    LOG_E("Run svc failed.");
    return ret;
  }
  return 0;
}

int SvcBase::afterRun() {
  if (installSignalHandler == InstallSignalHandler::True) {
    signalHandler_->run();
  }
  return 0;
}

void SvcBase::exit(const boost::system::error_code* ec, int signalNum) {
  beforeExit(ec, signalNum);
  doExit(ec, signalNum);
  afterExit(ec, signalNum);
}

}  // namespace bq
