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
#include "HisMD.hpp"
#include "SHMIPCMsgId.hpp"
#include "SHMSrv.hpp"
#include "WebSrv.hpp"
#include "def/CommonIPCData.hpp"
#include "def/Def.hpp"
#include "def/StatusCode.hpp"
#include "util/BQUtil.hpp"
#include "util/Logger.hpp"

using namespace bq::v1;

void QueryHisMD::queryBetween2Ts(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback,
    std::string &&marketCode, std::string &&symbolType,
    std::string &&symbolCode, std::string &&mdType, std::uint32_t level,
    std::uint64_t tsBegin, std::uint64_t tsEnd) {
  const auto storageRootPath = CONFIG["storageRootPath"].as<std::string>();
  // topic = MD@Binance@Spot@BTC-USDT@trades
  auto topic = fmt::format("{}{}{}{}{}{}{}{}{}", TOPIC_PREFIX_OF_MARKET_DATA,
                           SEP_OF_TOPIC, marketCode, SEP_OF_TOPIC, symbolType,
                           SEP_OF_TOPIC, symbolCode, SEP_OF_TOPIC, mdType);
  if (mdType == magic_enum::enum_name(MDType::Books)) {
    topic = topic + SEP_OF_TOPIC + std::to_string(level);
  }
  const auto [ret, exchTs2HisMDGroup] =
      HisMD::LoadHisMDBetweenTs(storageRootPath, topic, tsBegin, tsEnd);
  const auto body = HisMD::ToJson(ret, exchTs2HisMDGroup);

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
    std::string &&symbolCode, std::string &&mdType, std::uint32_t level,
    std::uint64_t ts, int offset) const {
  const auto storageRootPath = CONFIG["storageRootPath"].as<std::string>();
  // topic = MD@Binance@Spot@BTC-USDT@trades
  auto topic = fmt::format("{}{}{}{}{}{}{}{}{}", TOPIC_PREFIX_OF_MARKET_DATA,
                           SEP_OF_TOPIC, marketCode, SEP_OF_TOPIC, symbolType,
                           SEP_OF_TOPIC, symbolCode, SEP_OF_TOPIC, mdType);
  if (mdType == magic_enum::enum_name(MDType::Books)) {
    topic = topic + SEP_OF_TOPIC + std::to_string(level);
  }
  int ret = 0;
  ExchTs2HisMDGroupSPtr exchTs2HisMDGroup =
      std::make_shared<ExchTs2HisMDGroup>();
  if (offset < 0) {
    std::tie(ret, exchTs2HisMDGroup) =
        HisMD::LoadHisMDBeforeTs(storageRootPath, topic, ts, offset * -1);
  } else {
    std::tie(ret, exchTs2HisMDGroup) =
        HisMD::LoadHisMDAfterTs(storageRootPath, topic, ts, offset);
  }
  const auto body = HisMD::ToJson(ret, exchTs2HisMDGroup);

  auto resp = HttpResponse::newHttpResponse();
  resp->setStatusCode(k200OK);
  resp->setContentTypeCode(CT_APPLICATION_JSON);
  resp->setBody(body);
  callback(resp);
}
