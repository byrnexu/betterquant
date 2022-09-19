#pragma once

#include "SHMHeader.hpp"
#include "util/Pch.hpp"

namespace bq::td::svc {

struct ApiInfo;
using ApiInfoSPtr = std::shared_ptr<ApiInfo>;

struct ApiInfo {
  std::string apiKey_;
  std::string secKey_;
  std::string password_;
};

struct TDSrvSignal {
  TDSrvSignal(MsgId msgId) : shmHeader_(msgId) {}
  SHMHeader shmHeader_;
};

}  // namespace bq::td::svc
