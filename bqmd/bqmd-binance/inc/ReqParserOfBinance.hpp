/*!
 * \file ReqParserOfBinance.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "ReqParser.hpp"
#include "util/Pch.hpp"

namespace bq::md::svc::binance {

class ReqParserOfBinance : public ReqParser {
 public:
  using ReqParser::ReqParser;

 private:
  std::vector<std::string> doGetTopicGroupForSubOrUnSubAgain(
      const std::string& req) final;
};

}  // namespace bq::md::svc::binance
