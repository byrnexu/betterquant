/*!
 * \file DBEngParam.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "db/DBEngDef.hpp"

namespace bq::db {

constexpr static std::string_view MODULE_DBE_PARAM = {"DBE_PARAM"};

const static std::string SEP_OF_REC_IN_DBE_PARAM = ";";
const static std::string SEP_OF_FIELD_IN_DBE_PARAM = "=";

struct DBEngParam {
  DBEngParam() = default;
  DBEngParam(const std::string& svcName, const std::string& host, int port,
             const std::string& username, const std::string& password,
             const std::string& dbname, int connPoolSizeOfSyncReq,
             int connPoolSizeOfAsyncReq,
             std::uint32_t numOfUnprocessedTaskAlert,
             std::uint32_t timeDurOfWaitForTask);
  std::string svcName_;
  std::string host_;
  int port_;
  std::string username_;
  std::string password_;
  std::string dbname_;
  int connPoolSizeOfSyncReq_{1};
  int connPoolSizeOfAsyncReq_{1};
  std::uint32_t numOfUnprocessedTaskAlert_{100};
  std::uint32_t timeDurOfWaitForTask_{500};
};

std::tuple<int, DBEngParamSPtr> MakeDBEngParam(
    const std::string& dbEngParamInStrFmt);

}  // namespace bq::db
