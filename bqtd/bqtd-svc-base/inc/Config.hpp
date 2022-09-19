#pragma once

#include "util/ConfigBase.hpp"
#include "util/Pch.hpp"

namespace bq::td::svc {

class Config : public ConfigBase,
               public boost::serialization::singleton<Config> {
 private:
  int afterInit(const std::string& configFilename) final;
};

}  // namespace bq::td::svc
