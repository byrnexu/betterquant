/*!
 * \file StgEngDef.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "SHMHeader.hpp"
#include "SHMIPCMsgId.hpp"
#include "def/BQDefIF.hpp"

namespace bq::stg {

struct StgSignal {
  StgSignal(MsgId msgId) : shmHeader_(msgId) {}
  SHMHeader shmHeader_;
};

}  // namespace bq::stg
