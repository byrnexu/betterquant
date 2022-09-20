/*!
 * \file ReqParser.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "util/Pch.hpp"

namespace bq::md::svc {

class MDSvc;

class ReqParser {
 public:
  ReqParser(const ReqParser&) = delete;
  ReqParser& operator=(const ReqParser&) = delete;
  ReqParser(const ReqParser&&) = delete;
  ReqParser& operator=(const ReqParser&&) = delete;

  ReqParser(MDSvc* mdSvc) : mdSvc_(mdSvc) {}

 public:
  std::vector<std::string> getTopicGroupForSubOrUnSubAgain(
      const std::string& req) {
    return doGetTopicGroupForSubOrUnSubAgain(req);
  }

 private:
  virtual std::vector<std::string> doGetTopicGroupForSubOrUnSubAgain(
      const std::string& req) = 0;

 protected:
  MDSvc* mdSvc_{nullptr};
};

}  // namespace bq::md::svc
