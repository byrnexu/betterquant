#include "WebParam.hpp"

#include "def/Def.hpp"
#include "util/Logger.hpp"
#include "util/Pch.hpp"
#include "util/String.hpp"

namespace bq::web {

WSParam::WSParam(const std::string& svcName,
                 std::uint32_t timeoutOfOpenHandshake,
                 std::uint32_t timeoutOfCloseHandshake,
                 std::uint32_t intervalOfReconnect,
                 std::uint32_t intervalOfSendPingAndCheckConn,
                 std::uint32_t expireTimeOfConn)
    : svcName_(svcName),
      timeoutOfOpenHandshake_(timeoutOfOpenHandshake),
      timeoutOfCloseHandshake_(timeoutOfCloseHandshake),
      intervalOfReconnect_(intervalOfReconnect),
      intervalOfSendPingAndCheckConn_(intervalOfSendPingAndCheckConn),
      expireTimeOfConn_(expireTimeOfConn) {}

std::tuple<int, WSParamSPtr> MakeWSParam(const std::string& wsParamInStrFmt) {
  auto [retOfStr2Map, wsParamTable] =
      Str2Map(wsParamInStrFmt, SEP_OF_REC_IN_WS_CLI_PARAM,
              SEP_OF_FIELD_IN_WS_CLI_PARAM);
  if (retOfStr2Map != 0) {
    LOG_E("Make ws cli param failed. {}", wsParamInStrFmt);
    return {-1, nullptr};
  }

  std::string fieldName;
  std::string fieldValue;
  auto ret = std::make_shared<WSParam>();
  try {
    fieldName = "svcname";
    fieldValue = wsParamTable[fieldName];
    ret->svcName_ = fieldValue;

    fieldName = "timeoutofopenhandshake";
    fieldValue = wsParamTable[fieldName];
    ret->timeoutOfOpenHandshake_ = CONV(std::uint32_t, fieldValue);

    fieldName = "timeoutofclosehandshake";
    fieldValue = wsParamTable[fieldName];
    ret->timeoutOfCloseHandshake_ = CONV(std::uint32_t, fieldValue);

    fieldName = "intervalofreconnect";
    fieldValue = wsParamTable[fieldName];
    ret->intervalOfReconnect_ = CONV(std::uint32_t, fieldValue);

    fieldName = "intervalofsendpingandcheckconn";
    fieldValue = wsParamTable[fieldName];
    ret->intervalOfSendPingAndCheckConn_ = CONV(std::uint32_t, fieldValue);

    fieldName = "sendping";
    fieldValue = wsParamTable[fieldName];
    ret->sendPing_ = fieldValue == "0" ? false : true;

    fieldName = "expiretimeofconn";
    fieldValue = wsParamTable[fieldName];
    ret->expireTimeOfConn_ = CONV(std::uint32_t, fieldValue);

  } catch (const std::exception& e) {
    LOG_E("Make ws cli param failed because of invalid field info of {}. {}",
          fieldName, e.what());
    return {-1, nullptr};
  }

  return {0, ret};
}

}  // namespace bq::web
