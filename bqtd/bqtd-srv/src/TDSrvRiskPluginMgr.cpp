/*!
 * \file TDSrvRiskPluginMgr.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "TDSrvRiskPluginMgr.hpp"

#include "TDSrv.hpp"
#include "TDSrvRiskPlugin.hpp"
#include "def/StatusCode.hpp"
#include "util/File.hpp"
#include "util/Logger.hpp"

namespace bq::td::srv {

TDSrvRiskPluginMgr::TDSrvRiskPluginMgr(TDSrv* tdSrv) : tdSrv_(tdSrv) {}

int TDSrvRiskPluginMgr::load() {
  const auto tdSrvRiskPluginPath =
      CONFIG["tdSrvRiskPluginPath"].as<std::string>();
  const auto [ret, fileGroup] =
      GetFileGroupFromPathRecursively(tdSrvRiskPluginPath);
  if (ret != 0) {
    LOG_W("Load trSrv risk plugin from {} failed.", tdSrvRiskPluginPath);
    return ret;
  }

  std::map<std::size_t, boost::dll::fs::path> no2libPath;
  for (const auto& file : fileGroup) {
    if (file.extension().string() != ".so") continue;

    boost::filesystem::path parentPathAndStem = file.parent_path();
    parentPathAndStem /= file.stem();

    std::vector<std::string> strGroup;
    boost::split(strGroup, parentPathAndStem.string(), boost::is_any_of("-"));
    boost::optional<std::size_t> optNo;
#ifndef NDEBUG
    if (strGroup.size() < 3) {
      LOG_W("Get invalid file {} when load risk plugin.", file.string());
      continue;
    }
    if (strGroup[strGroup.size() - 1] != "d") {
      continue;
    }
    optNo = CONV_OPT(std::size_t, strGroup[strGroup.size() - 2]);
#else
    if (strGroup.size() < 2) {
      LOG_W("Get invalid file {} when load risk plugin.", file.string());
      continue;
    }
    if (strGroup[strGroup.size() - 1] == "d") {
      continue;
    }
    optNo = CONV_OPT(std::size_t, strGroup[strGroup.size() - 1]);
#endif
    if (optNo == boost::none) {
      LOG_W("Get invalid file {} when load risk plugin.", file.string());
      continue;
    }

    const auto no = optNo.value();
    if (no >= MAX_TD_SRV_RISK_PLUGIN_NUM) {
      LOG_W("Get invalid file {} when load risk plugin.", file.string());
      continue;
    }

    boost::dll::fs::path libPath(file.parent_path());
    libPath /= file.stem();

    boost::dll::library_info info(file);
    std::vector<std::string> exports = info.symbols("PlugIn");
    if (exports.empty()) {
      LOG_W("Get invalid file {} when load risk plugin.", file.string());
      continue;
    }

    no2libPath.emplace(no, libPath);
  }

  for (std::size_t no = 0; no < MAX_TD_SRV_RISK_PLUGIN_NUM; ++no) {
    const auto iter = no2libPath.find(no);
    if (iter == std::end(no2libPath)) {
      const auto oldPlugin = safeTDSrvRiskPluginGroup_[no].get();
      if (oldPlugin != nullptr) {
        oldPlugin->unload();
        safeTDSrvRiskPluginGroup_[no].set(nullptr);
        LOG_I("Unload risk plugin {} - {} success.", no, oldPlugin->name());
      } else {
      }

    } else {
      const auto& libPath = iter->second;
      const auto newPlugin = createPlugin(no, libPath);
      if (!newPlugin) continue;

      const auto oldPlugin = safeTDSrvRiskPluginGroup_[no].get();
      if (oldPlugin != nullptr) {
        if (newPlugin->getMD5SumOfConf() != oldPlugin->getMD5SumOfConf()) {
          if (newPlugin->enable()) {
            if (oldPlugin->enable()) {
              // newPlugin and oldPlugin are all enable
              oldPlugin->unload();
              const auto ret = newPlugin->load();
              if (ret == 0) {
                safeTDSrvRiskPluginGroup_[no].set(newPlugin);
                LOG_I("Update risk plugin {} - {} to {} success.", no,
                      oldPlugin->name(), newPlugin->name());
              } else {
                LOG_W("Update risk plugin {} - {} to {} failed. [{} - {}]", no,
                      oldPlugin->name(), newPlugin->name(), ret,
                      GetStatusMsg(ret));
              }

            } else {
              // newPlugin enable oldPlugin disable
              const auto ret = newPlugin->load();
              if (ret == 0) {
                safeTDSrvRiskPluginGroup_[no].set(newPlugin);
                LOG_I("Load risk plugin {} - {} success.", no,
                      newPlugin->name());
              } else {
                LOG_W("Load risk plugin {} - {} failed. [{} - {}]", no,
                      newPlugin->name(), ret, GetStatusMsg(ret));
              }
            }

          } else {
            if (oldPlugin->enable()) {
              // newPlugin disable old plugin enable
              oldPlugin->unload();
              safeTDSrvRiskPluginGroup_[no].set(nullptr);
              LOG_I("Unload risk plugin {} - {} success.", no,
                    oldPlugin->name());

            } else {
              // newPlugin and oldPlugin are all disable
            }
          }
        } else {
        }

      } else {
        if (newPlugin->enable()) {
          const auto ret = newPlugin->load();
          if (ret == 0) {
            safeTDSrvRiskPluginGroup_[no].set(newPlugin);
            LOG_I("Load risk plugin {} - {} success.", no, newPlugin->name());
          } else {
            LOG_W("Load risk plugin {} - {} failed. [{} - {}]", no,
                  newPlugin->name(), ret, GetStatusMsg(ret));
          }
        } else {
          // newPlugin disable and oldPlugin == nullptr
        }
      }
    }
  }

  return 0;
}

TDSrvRiskPluginSPtr TDSrvRiskPluginMgr::createPlugin(
    std::size_t no, const boost::filesystem::path& libPath) {
  auto plugin = GetPlugin(libPath, "CreatePlugin", tdSrv_);
  const auto configFilename = fmt::format("{}.yaml", libPath.string());
  const auto statusCode = plugin->init(configFilename);
  if (statusCode != 0) {
    LOG_I("Load risk plugin {} - {} failed because of init by config {} failed",
          no, libPath.stem().string(), configFilename);
    return nullptr;
  }
  return plugin;
}

int TDSrvRiskPluginMgr::onOrder(const OrderInfoSPtr& order) {
  for (const auto& safePlugin : safeTDSrvRiskPluginGroup_) {
    auto plugin = safePlugin.get();
    if (!plugin) continue;
    const auto statusCode = plugin->onOrder(order);
    if (statusCode != 0) {
      return statusCode;
    }
  }
  return 0;
}

int TDSrvRiskPluginMgr::onCancelOrder(const OrderInfoSPtr& order) {
  for (const auto& safePlugin : safeTDSrvRiskPluginGroup_) {
    auto plugin = safePlugin.get();
    if (!plugin) continue;
    const auto statusCode = plugin->onCancelOrder(order);
    if (statusCode != 0) {
      return statusCode;
    }
  }
  return 0;
}

int TDSrvRiskPluginMgr::onOrderRet(const OrderInfoSPtr& order) {
  for (const auto& safePlugin : safeTDSrvRiskPluginGroup_) {
    auto plugin = safePlugin.get();
    if (!plugin) continue;
    const auto statusCode = plugin->onOrderRet(order);
    if (statusCode != 0) {
      return statusCode;
    }
  }
  return 0;
}

int TDSrvRiskPluginMgr::onCancelOrderRet(const OrderInfoSPtr& order) {
  for (const auto& safePlugin : safeTDSrvRiskPluginGroup_) {
    auto plugin = safePlugin.get();
    if (!plugin) continue;
    const auto statusCode = plugin->onCancelOrderRet(order);
    if (statusCode != 0) {
      return statusCode;
    }
  }
  return 0;
}

}  // namespace bq::td::srv
