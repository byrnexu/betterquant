/*!
 * \file bq_v1_QueryHisMD.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/25
 *
 * \brief
 */

#pragma once

#include <drogon/HttpController.h>

#include "def/BQDef.hpp"

using namespace drogon;

namespace bq {
namespace v1 {

class QueryHisMD : public drogon::HttpController<QueryHisMD> {
 public:
  METHOD_LIST_BEGIN
  // http://192.168.19.113/v1/QueryHisMD/between/Binance/Spot/BTC-USDT/Trades?tsBegin=1668989747663000&tsEnd=1668989747697000
  // http://192.168.19.113/v1/QueryHisMD/between/Binance/Spot/BTC-USDT/Books?level=20&tsBegin=1669032414507000&tsEnd=1669032415008000
  // http://192.168.19.113/v1/QueryHisMD/between/Binance/Spot/BTC-USDT/Candle?detail=true&tsBegin=1669032414507000&tsEnd=1669032415008000
  ADD_METHOD_TO(
      QueryHisMD::queryBetween2Ts,
      "/v1/QueryHisMD/between/{marketCode}/{symbolType}/{symbolCode}/"
      "{mdType}?detail={detail}&level={level}&tsBegin={tsBegin}&tsEnd={tsEnd}",
      Get);
  // http://192.168.19.113/v1/QueryHisMD/offset/Binance/Spot/BTC-USDT/Trades?ts=1668989747697000&offset=1
  // http://192.168.19.113/v1/QueryHisMD/offset/Binance/Spot/BTC-USDT/Books?level=20&ts=1669032414507000&offset=1000
  // http://192.168.19.113/v1/QueryHisMD/offset/Binance/Spot/BTC-USDT/Candle?ts=1669032414507000&offset=1000
  ADD_METHOD_TO(
      QueryHisMD::queryBasedOnOffsetOfTs,
      "/v1/QueryHisMD/offset/{marketCode}/{symbolType}/{symbolCode}/"
      "{mdType}?detail={detail}&level={level}&ts={ts}&offset={offset}",
      Get);

  ADD_METHOD_TO(QueryHisMD::queryBeforeTs,
                "/v1/QueryHisMD/before/{marketCode}/{symbolType}/{symbolCode}/"
                "{mdType}?detail={detail}&level={level}&ts={ts}&num={num}",
                Get);

  ADD_METHOD_TO(QueryHisMD::queryAfterTs,
                "/v1/QueryHisMD/after/{marketCode}/{symbolType}/{symbolCode}/"
                "{mdType}?detail={detail}&level={level}&ts={ts}&num={num}",
                Get);

  METHOD_LIST_END

  void queryBetween2Ts(const HttpRequestPtr &req,
                       std::function<void(const HttpResponsePtr &)> &&callback,
                       std::string &&marketCode, std::string &&symbolType,
                       std::string &&symbolCode, std::string &&mdType,
                       std::string &&detail, std::uint32_t level,
                       std::uint64_t tsBegin, std::uint64_t tsEnd);

  void queryBasedOnOffsetOfTs(
      const HttpRequestPtr &req,
      std::function<void(const HttpResponsePtr &)> &&callback,
      std::string &&marketCode, std::string &&symbolType,
      std::string &&symbolCode, std::string &&mdType, std::string &&detail,
      std::uint32_t level, std::uint64_t ts, int offset) const;

  void queryBeforeTs(const HttpRequestPtr &req,
                     std::function<void(const HttpResponsePtr &)> &&callback,
                     std::string &&marketCode, std::string &&symbolType,
                     std::string &&symbolCode, std::string &&mdType,
                     std::string &&detail, std::uint32_t level,
                     std::uint64_t ts, int num) const;

  void queryAfterTs(const HttpRequestPtr &req,
                    std::function<void(const HttpResponsePtr &)> &&callback,
                    std::string &&marketCode, std::string &&symbolType,
                    std::string &&symbolCode, std::string &&mdType,
                    std::string &&detail, std::uint32_t level, std::uint64_t ts,
                    int num) const;
};

}  // namespace v1
}  // namespace bq
