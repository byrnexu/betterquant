/*!
 * \file ConfigBase.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "util/ConfigBase.hpp"

#include "util/Logger.hpp"

namespace bq {

int ConfigBase::init(const std::string& configFilename) {
  if (const auto ret = beforeInit(configFilename); ret != 0) {
    const auto statusMsg = fmt::format("Init config failed.");
    std::cerr << statusMsg << std::endl;
    return -1;
  }

  if (const auto ret = doInit(configFilename); ret != 0) {
    const auto statusMsg = fmt::format("Init config failed.");
    std::cerr << statusMsg << std::endl;
    return -1;
  }

  if (const auto ret = afterInit(configFilename); ret != 0) {
    const auto statusMsg = fmt::format("Init config failed.");
    std::cerr << statusMsg << std::endl;
    return -1;
  }

  return 0;
}

int ConfigBase::beforeInit(const std::string& configFilename) { return 0; }

int ConfigBase::doInit(const std::string& configFilename) {
  try {
    node_ = YAML::LoadFile(configFilename);
  } catch (const std::exception& e) {
    const auto statusMsg = fmt::format("Init config by file {} failed. [{}]",
                                       configFilename, e.what());
    std::cerr << statusMsg << std::endl;
    return -1;
  }

  return 0;
}

int ConfigBase::afterInit(const std::string& configFilename) { return 0; }

std::tuple<int, YAML::Node> InitConfig(const std::string& configFilename) {
  YAML::Node config;
  try {
    config = YAML::LoadFile(configFilename);
  } catch (const std::exception& e) {
    LOG_W("Init config by file {} failed. [{}]", configFilename, e.what());
    return {-1, config};
  }
  return {0, config};
}

}  // namespace bq
