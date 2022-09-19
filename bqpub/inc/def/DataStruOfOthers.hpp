#pragma once

#include "SHMHeader.hpp"
#include "SHMIPCMsgId.hpp"
#include "def/BQDef.hpp"
#include "def/Const.hpp"
#include "util/Pch.hpp"

namespace bq {

struct StgReg {
  SHMHeader shmHeader_;
};

struct TDGWReg {
  SHMHeader shmHeader_;
};

}  // namespace bq
