#include "db/DBConnpool.hpp"

#include "db/DBEngParam.hpp"
#include "util/Logger.hpp"

namespace bq::db {

DBConnpool::DBConnpool(const DBEngParamSPtr& dbEngParam, ConnType connType)
    : dbEngParam_(dbEngParam), connType_(connType) {}

int DBConnpool::init() {
  try {
    const auto connPoolSize = connType_ == ConnType::Sync
                                  ? dbEngParam_->connPoolSizeOfSyncReq_
                                  : dbEngParam_->connPoolSizeOfAsyncReq_;
    sql::Driver* driver = get_driver_instance();
    for (int no = 0; no < connPoolSize; ++no) {
      auto connProperties = makeConnProperties(dbEngParam_);
      std::shared_ptr<sql::Connection> sqlConn(driver->connect(connProperties));
      {
        std::lock_guard<std::ext::spin_mutex> lock(mtxConnGroup_);
        connGroup_.emplace_back(std::make_shared<Conn>(no, true, sqlConn));
      }
    }
  } catch (const std::exception& e) {
    LOG_E("Init failed. [{}]", e.what());
    return -1;
  }
  return 0;
}

sql::ConnectOptionsMap DBConnpool::makeConnProperties(
    const DBEngParamSPtr& dbEngParam) const {
  sql::ConnectOptionsMap connProperties;
  connProperties["hostName"] = dbEngParam->host_;
  connProperties["userName"] = dbEngParam->username_;
  connProperties["password"] = dbEngParam->password_;
  connProperties["schema"] = dbEngParam->dbname_;
  connProperties["port"] = dbEngParam->port_;
  connProperties["OPT_RECONNECT"] = true;
  return connProperties;
}

std::uint32_t DBConnpool::getSize() const {
  std::lock_guard<std::ext::spin_mutex> lock(mtxConnGroup_);
  return connGroup_.size();
}

ConnSPtr DBConnpool::getIdleConn() const {
  while (true) {
    {
      std::lock_guard<std::ext::spin_mutex> lock(mtxConnGroup_);
      for (const auto& conn : connGroup_) {
        if (conn->idle_) {
          conn->idle_ = false;
          return conn;
        }
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

void DBConnpool::giveBackConn(const ConnSPtr& conn) {
  std::lock_guard<std::ext::spin_mutex> lock(mtxConnGroup_);
  for (const auto& curConn : connGroup_) {
    if (curConn->no_ == conn->no_) {
      curConn->idle_ = true;
    }
  }
}

}  // namespace bq::db
