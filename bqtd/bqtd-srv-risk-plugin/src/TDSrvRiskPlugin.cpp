/*!
 * \file TDSrvRiskPlugin.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "TDSrvRiskPlugin.hpp"

#include "ConfigOfPlugin.hpp"
#include "TDSrv.hpp"
#include "util/File.hpp"
#include "util/Logger.hpp"
#include "util/String.hpp"

namespace bq::td::srv {

TDSrvRiskPlugin::TDSrvRiskPlugin(TDSrv* tdSrv) : tdSrv_(tdSrv) {}

int TDSrvRiskPlugin::init(const std::string& configFilename) {
  if (const auto ret =
          ConfigOfPlugin::get_mutable_instance().init(configFilename);
      ret != 0) {
    const auto statusMsg = fmt::format(
        "Init risk plug in {} failed "
        "because of init config by file {} failed. ",
        name(), configFilename);
    std::cerr << statusMsg << std::endl;
    return ret;
  }

  logger_ = makeLogger(configFilename);
  if (logger_ == nullptr) {
    const auto statusMsg = fmt::format(
        "Init risk plug in {} failed "
        "because of init logger by file {} failed. ",
        name(), configFilename);
    std::cerr << statusMsg << std::endl;
    return -1;
  }

  name_ = fmt::format("{}-v{}",  //
                      CONFIG_OF_PLUGIN["name"].as<std::string>(),
                      CONFIG_OF_PLUGIN["version"].as<std::string>());

  enable_ = CONFIG_OF_PLUGIN["enable"].as<bool>(false);

  const auto configFileCont = LoadFileContToStr(configFilename);
  md5SumOfConf_ = MD5Sum(configFileCont);

  return 0;
}

int TDSrvRiskPlugin::load() {
  LOG_I("Load risk plugin {}.", name());
  return doLoad();
}

void TDSrvRiskPlugin::unload() {
  LOG_I("Unload risk plugin {}.", name());
  doUnload();
}

TDSrv* TDSrvRiskPlugin::getTDSrv() const { return tdSrv_; }

}  // namespace bq::td::srv
