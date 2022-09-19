#pragma once

#include <gflags/gflags.h>

#include "util/PchBase.hpp"

namespace bq {

struct GFlagsHolder {
  GFlagsHolder(GFlagsHolder&) = delete;
  GFlagsHolder(GFlagsHolder&&) = delete;
  GFlagsHolder& operator=(GFlagsHolder&) = delete;
  GFlagsHolder& operator=(GFlagsHolder&&) = delete;

  GFlagsHolder(int argc, char** argv, const std::string& version,
               const std::string& params) {
    initGFlags(argc, argv, version, params);
  }
  ~GFlagsHolder() { uninitGFlags(); }

 private:
  void initGFlags(int argc, char** argv, const std::string& version,
                  const std::string& params) {
    google::SetVersionString(version);
    gflags::SetUsageMessage("Usage: " + std::string(argv[0]) + " " + params);
    google::ParseCommandLineFlags(&argc, &argv, true);
  }

  void uninitGFlags() { google::ShutDownCommandLineFlags(); }
};

static bool validateConf(const char* flagname, const std::string& value) {
  if (value.empty()) {
    std::cerr << "Invalid value of " << flagname << ". ";
    return false;
  }
  return true;
}

DECLARE_string(conf);
DEFINE_string(conf, "", "configuration");
DEFINE_validator(conf, &validateConf);

}  // namespace bq
