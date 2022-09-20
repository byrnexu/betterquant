/*!
 * \file WebParam.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "def/Def.hpp"
#include "util/Pch.hpp"

namespace bq::web {

const static std::string SEP_OF_REC_IN_WS_CLI_PARAM = ";";
const static std::string SEP_OF_FIELD_IN_WS_CLI_PARAM = "=";

struct WSParam;
using WSParamSPtr = std::shared_ptr<WSParam>;

struct WSParam {
  WSParam() = default;
  WSParam(const std::string& svcName, std::uint32_t timeoutOfOpenHandshake,
          std::uint32_t timeoutOfCloseHandshake,
          std::uint32_t intervalOfReconnect,
          std::uint32_t intervalOfSendPingAndCheckConn,
          std::uint32_t expireTimeOfConn);

  std::string svcName_{""};
  std::uint32_t timeoutOfOpenHandshake_{30000};
  std::uint32_t timeoutOfCloseHandshake_{30000};
  std::uint32_t intervalOfReconnect_{3000};
  std::uint32_t intervalOfSendPingAndCheckConn_{1000};
  bool sendPing_{false};
  std::uint32_t expireTimeOfConn_{30000};
};

std::tuple<int, WSParamSPtr> MakeWSParam(const std::string& wsParamInStrFmt);

}  // namespace bq::web
