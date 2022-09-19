#pragma once

#include "SHMHeader.hpp"
#include "SHMIPCMsgId.hpp"
#include "def/BQDefIF.hpp"

namespace bq::stg {

struct StgSignal {
  StgSignal(MsgId msgId, StgInstId stgInstId)
      : shmHeader_(msgId), stgInstId_(stgInstId) {}
  SHMHeader shmHeader_;
  StgInstId stgInstId_{1};
};

}  // namespace bq::stg
