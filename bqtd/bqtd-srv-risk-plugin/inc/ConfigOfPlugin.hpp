/*!
 * \file ConfigOfPlugin.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "util/ConfigBase.hpp"
#include "util/Pch.hpp"

namespace bq::td::srv {

#define CONFIG_OF_PLUGIN ConfigOfPlugin::get_mutable_instance().get()

class ConfigOfPlugin : public ConfigBase,
                       public boost::serialization::singleton<ConfigOfPlugin> {
};

}  // namespace bq::td::srv
