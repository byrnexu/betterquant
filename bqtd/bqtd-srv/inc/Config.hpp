#pragma once

#include "util/ConfigBase.hpp"
#include "util/Pch.hpp"

namespace bq::td::srv {

class Config : public ConfigBase,
               public boost::serialization::singleton<Config> {};

}  // namespace bq::td::srv
