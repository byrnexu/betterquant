#pragma once

#include "util/PchBase.hpp"

namespace bq {

using ClientChannel = std::uint16_t;
enum class Direction : std::uint8_t { Req = 1, Rsp, Push };

using FillSHMBufCallback = std::function<void(void* shmBuf)>;
using DataRecvCallback =
    std::function<void(const void* shmBuf, std::size_t shmBufLen)>;

struct SHMIPCTask;
using SHMIPCTaskSPtr = std::shared_ptr<SHMIPCTask>;

class SHMSrv;
using SHMSrvSPtr = std::shared_ptr<SHMSrv>;

class SHMCli;
using SHMCliSPtr = std::shared_ptr<SHMCli>;

}  // namespace bq
