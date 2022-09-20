/*!
 * \file StgAsyncTaskArg.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "def/BQDefIF.hpp"
#include "util/PchBase.hpp"

namespace bq::stg {

struct StgAsyncTaskArg;
using StgAsyncTaskArgSPtr = std::shared_ptr<StgAsyncTaskArg>;

struct StgAsyncTaskArg {
  explicit StgAsyncTaskArg(StgInstId stgInstId) : stgInstId_(stgInstId) {}
  StgInstId stgInstId_;
};

}  // namespace bq::stg
