#pragma once

#include "def/Def.hpp"
#include "util/Pch.hpp"

namespace bq::db {

enum class ConnType { Sync = 1, Async = 2 };

class DBConnpool;
using DBConnpoolSPtr = std::shared_ptr<DBConnpool>;

struct DBEngParam;
using DBEngParamSPtr = std::shared_ptr<DBEngParam>;

struct Conn;
using ConnSPtr = std::shared_ptr<Conn>;

class DBConnpool;
using DBConnpoolSPtr = std::shared_ptr<DBConnpool>;

class DBEngImpl;
using DBEngImplSPtr = std::shared_ptr<DBEngImpl>;

class DBEngSync;
using DBEngSyncSPtr = std::shared_ptr<DBEngSync>;

class DBEngAsync;
using DBEngAsyncSPtr = std::shared_ptr<DBEngAsync>;

class DBEng;
using DBEngSPtr = std::shared_ptr<DBEng>;

struct DBTask;
using DBTaskSPtr = std::shared_ptr<DBTask>;
using CBOnExecRet =
    std::function<void(bq::db::DBTaskSPtr& dbTask, const StringSPtr& execRet)>;

}  // namespace bq::db
