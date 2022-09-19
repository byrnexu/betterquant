#pragma once

#include "RspParser.hpp"
#include "util/Pch.hpp"

namespace bq::md::svc::binance {

class RspParserOfBinance : public RspParser {
 public:
  using RspParser::RspParser;

 private:
  TopicGroupNeedMaintSPtr doGetTopicGroupForSubOrUnSubAgain(
      WSCliAsyncTaskSPtr& asyncTask) final;
};

}  // namespace bq::md::svc::binance
