/*!
 * \file DBEngParam.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "db/DBEngParam.hpp"

#include "util/Logger.hpp"
#include "util/String.hpp"

namespace bq::db {

DBEngParam::DBEngParam(const std::string& svcName, const std::string& host,
                       int port, const std::string& username,
                       const std::string& password, const std::string& dbname,
                       int connPoolSizeOfSyncReq, int connPoolSizeOfAsyncReq,
                       std::uint32_t numOfUnprocessedTaskAlert,
                       std::uint32_t timeDurOfWaitForTask)
    : svcName_(svcName),
      host_(host),
      port_(port),
      username_(username),
      password_(password),
      dbname_(dbname),
      connPoolSizeOfSyncReq_(connPoolSizeOfSyncReq),
      connPoolSizeOfAsyncReq_(connPoolSizeOfAsyncReq),
      numOfUnprocessedTaskAlert_(numOfUnprocessedTaskAlert),
      timeDurOfWaitForTask_(timeDurOfWaitForTask) {}

std::tuple<int, DBEngParamSPtr> MakeDBEngParam(
    const std::string& dbEngParamInStrFmt) {
  auto [retOfStr2Map, dbEngParamTable] = Str2Map(
      dbEngParamInStrFmt, SEP_OF_REC_IN_DBE_PARAM, SEP_OF_FIELD_IN_DBE_PARAM);
  if (retOfStr2Map != 0) {
    LOG_E("Make db engine param failed. {}", dbEngParamInStrFmt);
    return {-1, DBEngParamSPtr()};
  }

  std::string fieldName;
  std::string fieldValue;
  auto ret = std::make_shared<DBEngParam>();
  try {
    fieldName = "svcname";
    fieldValue = dbEngParamTable[fieldName];
    ret->svcName_ = fieldValue;

    fieldName = "host";
    fieldValue = dbEngParamTable[fieldName];
    ret->host_ = fieldValue;

    fieldName = "port";
    fieldValue = dbEngParamTable[fieldName];
    ret->port_ = CONV(int, fieldValue);

    fieldName = "username";
    fieldValue = dbEngParamTable[fieldName];
    ret->username_ = fieldValue;

    fieldName = "password";
    fieldValue = dbEngParamTable[fieldName];
    ret->password_ = fieldValue;

    fieldName = "dbname";
    fieldValue = dbEngParamTable[fieldName];
    ret->dbname_ = fieldValue;

    fieldName = "connpoolsizeofsyncreq";
    fieldValue = dbEngParamTable[fieldName];
    ret->connPoolSizeOfSyncReq_ = CONV(int, fieldValue);

    fieldName = "connpoolsizeofasyncreq";
    fieldValue = dbEngParamTable[fieldName];
    ret->connPoolSizeOfAsyncReq_ = CONV(int, fieldValue);

    fieldName = "numofunprocessedtaskalert";
    fieldValue = dbEngParamTable[fieldName];
    ret->numOfUnprocessedTaskAlert_ = CONV(std::uint32_t, fieldValue);

    fieldName = "timedurofwaitfortask";
    fieldValue = dbEngParamTable[fieldName];
    ret->timeDurOfWaitForTask_ = CONV(std::uint32_t, fieldValue);

  } catch (const std::exception& e) {
    LOG_E("Make db engine param failed because of invalid field info of {}. {}",
          fieldName, e.what());
    return {-1, DBEngParamSPtr()};
  }

  return {0, ret};
}

}  // namespace bq::db
