/*!
 * \file DBEngImpl.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "db/DBEngImpl.hpp"

#include "db/DBConnpool.hpp"
#include "db/DBEngDef.hpp"
#include "def/Const.hpp"
#include "util/Logger.hpp"
#include "util/Pch.hpp"
#include "util/Random.hpp"
#include "util/String.hpp"

namespace bq::db {

DBEngImpl::DBEngImpl(const DBEngParamSPtr& dbEngParam, ConnType connType)
    : dbEngParam_(dbEngParam),
      connPool_(std::make_shared<DBConnpool>(dbEngParam, connType)) {}

int DBEngImpl::init() { return connPool_->init(); }

std::tuple<int, std::string> DBEngImpl::execUSP(const std::string& identity,
                                                const std::string& sql,
                                                WriteLog writeLog) {
  return asyncOrSyncExecSql(identity, sql, writeLog);
}

std::tuple<int, std::string> DBEngImpl::execSql(const std::string& identity,
                                                const std::string& sql,
                                                WriteLog writeLog) {
  auto conn = connPool_->getIdleConn();
  if (writeLog == WriteLog::True) {
    LOG_D(
        "Get an idle db connection successful. "
        "[conn no = {}, identity = {}, sql = {}]",
        conn->no_, identity, sql);
  }

  std::string jsonFmtOfRet;
  try {
    jsonFmtOfRet = execSqlImpl(conn, sql);
  } catch (const std::exception& e) {
    connPool_->giveBackConn(conn);
    jsonFmtOfRet = getJsonFmtOfStatus(-1, e.what());
    LOG_E(
        "Exec sql exception. "
        "[conn no = {}, identity = {}, sql = {}, exception = {}]",
        conn->no_, identity, sql, e.what());
    return {-1, jsonFmtOfRet};
  }

  connPool_->giveBackConn(conn);
  jsonFmtOfRet = getJsonFmtOfStatus(0, "Success") + "," + jsonFmtOfRet;
  if (writeLog == WriteLog::True) {
    LOG_D("Exec sql success. [conn no = {}, identity = {}, sql = {}]",
          conn->no_, identity, sql);
  }

  return {0, jsonFmtOfRet};
}

std::string DBEngImpl::execSqlImpl(const ConnSPtr& conn,
                                   const std::string& sql) {
  std::shared_ptr<sql::PreparedStatement> pstmt;
  pstmt.reset(conn->sqlConn_->prepareStatement(sql));

  std::shared_ptr<sql::ResultSet> res;
  res.reset(pstmt->executeQuery());

  std::string jsonFmtOfRecordSetGroup = "\"";
  jsonFmtOfRecordSetGroup = jsonFmtOfRecordSetGroup + "recordSetGroup";
  jsonFmtOfRecordSetGroup = jsonFmtOfRecordSetGroup + "\"" + ":[";
  for (int no = 0;; ++no) {
    const auto jsonFmtOfRecordSet = handleRecordSet(res, no);
    jsonFmtOfRecordSetGroup =
        jsonFmtOfRecordSetGroup + jsonFmtOfRecordSet + ",";
    if (pstmt->getMoreResults()) {
      res.reset(pstmt->getResultSet());
    } else {
      break;
    }
  }
  jsonFmtOfRecordSetGroup.pop_back();
  jsonFmtOfRecordSetGroup = jsonFmtOfRecordSetGroup + "]";
  return jsonFmtOfRecordSetGroup;
}

std::string DBEngImpl::handleRecordSet(
    const std::shared_ptr<sql::ResultSet>& res, int no) {
  sql::ResultSetMetaData* resMetadata = res->getMetaData();
  int fieldCount = resMetadata->getColumnCount();

  std::vector<int> fieldTypeGroup;
  for (int i = 0; i < fieldCount; ++i) {
    fieldTypeGroup.emplace_back(resMetadata->getColumnType(i + 1));
  }

  std::vector<std::string> fieldNameGroup;
  for (int i = 0; i < fieldCount; ++i) {
    fieldNameGroup.emplace_back(resMetadata->getColumnLabel(i + 1));
  }

  std::string jsonFmtOfRecordSet =
      getJsonFmtOfRecordSet(no, res, fieldTypeGroup, fieldNameGroup);
  return jsonFmtOfRecordSet;
}

std::string DBEngImpl::getJsonFmtOfRecordSet(
    int no, const std::shared_ptr<sql::ResultSet>& res,
    const std::vector<int>& fieldTypeGroup,
    const std::vector<std::string>& fieldNameGroup) {
  std::string jsonFmtOfRecordSet = "[";
  while (res->next()) {
    const auto jsonFmtOfRec =
        getJsonFmtOfRec(res, fieldTypeGroup, fieldNameGroup);
    jsonFmtOfRecordSet = jsonFmtOfRecordSet + jsonFmtOfRec + ",";
  }
  jsonFmtOfRecordSet.pop_back();
  if (jsonFmtOfRecordSet.empty() == false) {
    jsonFmtOfRecordSet = jsonFmtOfRecordSet + "]";
  }
  return jsonFmtOfRecordSet;
}

std::string DBEngImpl::getJsonFmtOfRec(
    const std::shared_ptr<sql::ResultSet>& res,
    const std::vector<int>& fieldTypeGroup,
    const std::vector<std::string>& fieldNameGroup) {
  std::string jsonFmtOfRec = "{";
  for (std::size_t i = 0; i < fieldNameGroup.size(); ++i) {
    if (fieldTypeGroup[i] == (int)sql::DataType::TINYINT) {
      int fieldValue = res->getInt(fieldNameGroup[i]);
      jsonFmtOfRec =
          jsonFmtOfRec + getJsonFmtOfField(fieldNameGroup[i], fieldValue);
    }
    if (fieldTypeGroup[i] == (int)sql::DataType::SMALLINT) {
      int fieldValue = res->getInt(fieldNameGroup[i]);
      jsonFmtOfRec =
          jsonFmtOfRec + getJsonFmtOfField(fieldNameGroup[i], fieldValue);
    }
    if (fieldTypeGroup[i] == (int)sql::DataType::INTEGER) {
      int fieldValue = res->getInt(fieldNameGroup[i]);
      jsonFmtOfRec =
          jsonFmtOfRec + getJsonFmtOfField(fieldNameGroup[i], fieldValue);
    }
    if (fieldTypeGroup[i] == (int)sql::DataType::BIGINT) {
      std::int64_t fieldValue = res->getInt64(fieldNameGroup[i]);
      jsonFmtOfRec =
          jsonFmtOfRec + getJsonFmtOfField(fieldNameGroup[i], fieldValue);
    }
    if (fieldTypeGroup[i] == (int)sql::DataType::DOUBLE) {
      double fieldValue = res->getDouble(fieldNameGroup[i]);
      jsonFmtOfRec =
          jsonFmtOfRec + getJsonFmtOfField(fieldNameGroup[i], fieldValue);
    }
    if (fieldTypeGroup[i] == (int)sql::DataType::DECIMAL) {
      std::string fieldValue =
          RemoveTrailingZero(res->getString(fieldNameGroup[i]));
      jsonFmtOfRec =
          jsonFmtOfRec + getJsonFmtOfField(fieldNameGroup[i], fieldValue);
    }
    if (fieldTypeGroup[i] == (int)sql::DataType::CHAR) {
      std::string fieldValue = res->getString(fieldNameGroup[i]);
      jsonFmtOfRec =
          jsonFmtOfRec + getJsonFmtOfField(fieldNameGroup[i], fieldValue);
    }
    if (fieldTypeGroup[i] == (int)sql::DataType::VARCHAR) {
      std::string fieldValue = res->getString(fieldNameGroup[i]);
      jsonFmtOfRec =
          jsonFmtOfRec + getJsonFmtOfField(fieldNameGroup[i], fieldValue);
    }
    if (fieldTypeGroup[i] == (int)sql::DataType::TIMESTAMP) {
      std::string fieldValue = res->getString(fieldNameGroup[i]);
      if (fieldValue.size() == 19) {
        fieldValue.append(".");
      }
      fieldValue.resize(26, '0');
      jsonFmtOfRec =
          jsonFmtOfRec + getJsonFmtOfField(fieldNameGroup[i], fieldValue);
    }
  }
  jsonFmtOfRec.pop_back();
  jsonFmtOfRec = jsonFmtOfRec + "}";
  return jsonFmtOfRec;
}

std::string DBEngImpl::getJsonFmtOfField(const std::string& fieldName,
                                         const std::string& fieldValue) {
  const auto jsonFieldValue =
      boost::algorithm::replace_all_copy(fieldValue, "\"", "\\\"");
  return "\"" + fieldName + "\":\"" + jsonFieldValue + "\",";
}

std::string DBEngImpl::getJsonFmtOfField(const std::string& fieldName,
                                         int fieldValue) {
  return "\"" + fieldName + "\":" + std::to_string(fieldValue) + ",";
}

std::string DBEngImpl::getJsonFmtOfField(const std::string& fieldName,
                                         std::int64_t fieldValue) {
  return "\"" + fieldName + "\":" + std::to_string(fieldValue) + ",";
}

std::string DBEngImpl::getJsonFmtOfField(const std::string& fieldName,
                                         double fieldValue) {
  return "\"" + fieldName + "\":" + std::to_string(fieldValue) + ",";
}

std::string DBEngImpl::getJsonFmtOfStatus(int statusCode,
                                          const std::string& statusMsg) {
  return "\"statusCode\":" + std::to_string(statusCode) + "," +
         "\"statusMsg\":\"" + statusMsg + "\"";
}

}  // namespace bq::db
