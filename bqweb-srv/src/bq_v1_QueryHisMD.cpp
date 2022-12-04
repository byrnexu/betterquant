/*!
 * \file bq_v1_QueryHisMD.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/25
 *
 * \brief
 */

#include "bq_v1_QueryHisMD.hpp"

#include "Config.hpp"
#include "SHMIPCMsgId.hpp"
#include "SHMSrv.hpp"
#include "WebSrv.hpp"
#include "def/CommonIPCData.hpp"
#include "def/Def.hpp"
#include "def/StatusCode.hpp"
#include "util/BQMDHis.hpp"
#include "util/BQUtil.hpp"
#include "util/Logger.hpp"

using namespace bq::v1;

void QueryHisMD::queryBetween2Ts(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback,
    std::string &&marketCode, std::string &&symbolType,
    std::string &&symbolCode, std::string &&mdType, std::string &&detail,
    std::uint32_t level, std::uint64_t tsBegin, std::uint64_t tsEnd) {
  const auto storageRootPath = CONFIG["storageRootPath"].as<std::string>();
  // topic = MD@Binance@Spot@BTC-USDT@trades
  auto topic = fmt::format("{}{}{}{}{}{}{}{}{}", TOPIC_PREFIX_OF_MARKET_DATA,
                           SEP_OF_TOPIC, marketCode, SEP_OF_TOPIC, symbolType,
                           SEP_OF_TOPIC, symbolCode, SEP_OF_TOPIC, mdType);
  if (mdType == magic_enum::enum_name(MDType::Books)) {
    topic = topic + SEP_OF_TOPIC + std::to_string(level);
  } else if (mdType == magic_enum::enum_name(MDType::Candle)) {
    if (boost::to_lower_copy(detail) == "true") {
      topic = topic + SEP_OF_TOPIC + SUFFIX_OF_CANDLE_DETAIL;
    }
  }
  const auto [ret, ts2HisMDGroup] =
      md::MDHis::LoadHisMDBetweenTs(storageRootPath, topic, tsBegin, tsEnd);
  const auto body = md::MDHis::ToJson(ret, ts2HisMDGroup);

  auto resp = HttpResponse::newHttpResponse();
  resp->setStatusCode(k200OK);
  resp->setContentTypeCode(CT_APPLICATION_JSON);
  resp->setBody(body);

  callback(resp);
}

void QueryHisMD::queryBasedOnOffsetOfTs(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback,
    std::string &&marketCode, std::string &&symbolType,
    std::string &&symbolCode, std::string &&mdType, std::string &&detail,
    std::uint32_t level, std::uint64_t ts, int offset) const {
  const auto storageRootPath = CONFIG["storageRootPath"].as<std::string>();
  // topic = MD@Binance@Spot@BTC-USDT@trades
  auto topic = fmt::format("{}{}{}{}{}{}{}{}{}", TOPIC_PREFIX_OF_MARKET_DATA,
                           SEP_OF_TOPIC, marketCode, SEP_OF_TOPIC, symbolType,
                           SEP_OF_TOPIC, symbolCode, SEP_OF_TOPIC, mdType);
  if (mdType == magic_enum::enum_name(MDType::Books)) {
    topic = topic + SEP_OF_TOPIC + std::to_string(level);
  } else if (mdType == magic_enum::enum_name(MDType::Candle)) {
    if (boost::to_lower_copy(detail) == "true") {
      topic = topic + SEP_OF_TOPIC + SUFFIX_OF_CANDLE_DETAIL;
    }
  }
  int ret = 0;
  auto ts2HisMDGroup = std::make_shared<md::Ts2HisMDGroup>();
  if (offset < 0) {
    std::tie(ret, ts2HisMDGroup) =
        md::MDHis::LoadHisMDBeforeTs(storageRootPath, topic, ts, offset * -1);
  } else {
    std::tie(ret, ts2HisMDGroup) =
        md::MDHis::LoadHisMDAfterTs(storageRootPath, topic, ts, offset);
  }
  const auto body = md::MDHis::ToJson(ret, ts2HisMDGroup);

  auto resp = HttpResponse::newHttpResponse();
  resp->setStatusCode(k200OK);
  resp->setContentTypeCode(CT_APPLICATION_JSON);
  resp->setBody(body);
  callback(resp);
}

void QueryHisMD::queryBeforeTs(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback,
    std::string &&marketCode, std::string &&symbolType,
    std::string &&symbolCode, std::string &&mdType, std::string &&detail,
    std::uint32_t level, std::uint64_t ts, int num) const {
  const auto storageRootPath = CONFIG["storageRootPath"].as<std::string>();
  // topic = MD@Binance@Spot@BTC-USDT@trades
  auto topic = fmt::format("{}{}{}{}{}{}{}{}{}", TOPIC_PREFIX_OF_MARKET_DATA,
                           SEP_OF_TOPIC, marketCode, SEP_OF_TOPIC, symbolType,
                           SEP_OF_TOPIC, symbolCode, SEP_OF_TOPIC, mdType);
  if (mdType == magic_enum::enum_name(MDType::Books)) {
    topic = topic + SEP_OF_TOPIC + std::to_string(level);
  } else if (mdType == magic_enum::enum_name(MDType::Candle)) {
    if (boost::to_lower_copy(detail) == "true") {
      topic = topic + SEP_OF_TOPIC + SUFFIX_OF_CANDLE_DETAIL;
    }
  }
  const auto [statusCode, ts2HisMDGroup] =
      md::MDHis::LoadHisMDBeforeTs(storageRootPath, topic, ts, num);
  const auto body = md::MDHis::ToJson(statusCode, ts2HisMDGroup);

  auto resp = HttpResponse::newHttpResponse();
  resp->setStatusCode(k200OK);
  resp->setContentTypeCode(CT_APPLICATION_JSON);
  resp->setBody(body);
  callback(resp);
}

void QueryHisMD::queryAfterTs(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback,
    std::string &&marketCode, std::string &&symbolType,
    std::string &&symbolCode, std::string &&mdType, std::string &&detail,
    std::uint32_t level, std::uint64_t ts, int num) const {
  const auto storageRootPath = CONFIG["storageRootPath"].as<std::string>();
  // topic = MD@Binance@Spot@BTC-USDT@trades
  auto topic = fmt::format("{}{}{}{}{}{}{}{}{}", TOPIC_PREFIX_OF_MARKET_DATA,
                           SEP_OF_TOPIC, marketCode, SEP_OF_TOPIC, symbolType,
                           SEP_OF_TOPIC, symbolCode, SEP_OF_TOPIC, mdType);
  if (mdType == magic_enum::enum_name(MDType::Books)) {
    topic = topic + SEP_OF_TOPIC + std::to_string(level);
  } else if (mdType == magic_enum::enum_name(MDType::Candle)) {
    if (boost::to_lower_copy(detail) == "true") {
      topic = topic + SEP_OF_TOPIC + SUFFIX_OF_CANDLE_DETAIL;
    }
  }
  const auto [statusCode, ts2HisMDGroup] =
      md::MDHis::LoadHisMDAfterTs(storageRootPath, topic, ts, num);
  const auto body = md::MDHis::ToJson(statusCode, ts2HisMDGroup);

  auto resp = HttpResponse::newHttpResponse();
  resp->setStatusCode(k200OK);
  resp->setContentTypeCode(CT_APPLICATION_JSON);
  resp->setBody(body);
  callback(resp);
}
