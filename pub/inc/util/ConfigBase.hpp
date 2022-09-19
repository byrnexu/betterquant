#pragma once

#include "util/Pch.hpp"

namespace bq {

class ConfigBase {
 public:
  int init(const std::string& configFilename);

 private:
  virtual int beforeInit(const std::string& configFilename);
  virtual int doInit(const std::string& configFilename);
  virtual int afterInit(const std::string& configFilename);

 public:
  YAML::Node& get() { return node_; }

 protected:
  YAML::Node node_;
};

std::tuple<int, YAML::Node> InitConfig(const std::string& configFilename);

}  // namespace bq
