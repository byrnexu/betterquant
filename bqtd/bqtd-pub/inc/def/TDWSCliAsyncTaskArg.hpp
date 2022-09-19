#pragma once

#include "def/BQConst.hpp"
#include "def/Def.hpp"

namespace bq::td {

struct WSCliAsyncTaskArg;
using WSCliAsyncTaskArgSPtr = std::shared_ptr<WSCliAsyncTaskArg>;

struct WSCliAsyncTaskArg {
  WSCliAsyncTaskArg(WSMsgType wsMsgType, const std::any& extData)
      : wsMsgType_(wsMsgType), extData_(extData) {}
  WSMsgType wsMsgType_;
  std::any extData_;
};

}  // namespace bq::td
