#pragma once

#include "db/DBEngDef.hpp"
#include "def/Const.hpp"
#include "util/Datetime.hpp"

namespace bq::db {

struct DBTask {
 public:
  DBTask(const std::string& identity, const std::string& sql, WriteLog writeLog)
      : identity_(identity), sql_(sql), writeLog_(writeLog) {}

  std::string toStr() {
    const auto ret = fmt::format("identity={}; sql={}", identity_, sql_);
    return ret;
  }

  const std::string identity_;
  const std::string sql_;
  const WriteLog writeLog_;
};

struct DBExecRet {
  DBExecRet(const DBTaskSPtr& dbTask, const StringSPtr& execRet)
      : localTs_(GetTotalUSSince1970()), dbTask_(dbTask), execRet_(execRet) {}
  const std::uint64_t localTs_;
  DBTaskSPtr dbTask_;
  StringSPtr execRet_;
};

}  // namespace bq::db
