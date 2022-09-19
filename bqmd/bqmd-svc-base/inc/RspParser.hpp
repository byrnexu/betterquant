#pragma once

#include "MDSvcDef.hpp"
#include "def/BQDef.hpp"
#include "util/Pch.hpp"

namespace bq::md::svc {

class MDSvc;

class RspParser {
 public:
  RspParser(const RspParser&) = delete;
  RspParser& operator=(const RspParser&) = delete;
  RspParser(const RspParser&&) = delete;
  RspParser& operator=(const RspParser&&) = delete;

  RspParser(MDSvc* mdSvc) : mdSvc_(mdSvc) {}

 public:
  TopicGroupNeedMaintSPtr getTopicGroupForSubOrUnSubAgain(
      WSCliAsyncTaskSPtr& asyncTask) {
    return doGetTopicGroupForSubOrUnSubAgain(asyncTask);
  }

 private:
  virtual TopicGroupNeedMaintSPtr doGetTopicGroupForSubOrUnSubAgain(
      WSCliAsyncTaskSPtr& asyncTask) = 0;

 protected:
  MDSvc* mdSvc_{nullptr};
};

}  // namespace bq::md::svc
