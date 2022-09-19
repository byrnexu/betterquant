#include "ReqParserOfBinance.hpp"

#include "Config.hpp"
#include "MDSvc.hpp"
#include "MDSvcOfBinanceUtil.hpp"
#include "util/Json.hpp"
#include "util/String.hpp"

namespace bq::md::svc::binance {

/*
 * {
 *  "method": "SUBSCRIBE",
 *   "params": [
 *    "btcusdt@aggTrade",
 *    "btcusdt@depth"
 *   ],
 *   "id": 1
 * }
 *
 * MD@Binance@Spot@BTC/USDT@candle@1D
 */
std::vector<std::string> ReqParserOfBinance::doGetTopicGroupForSubOrUnSubAgain(
    const std::string& req) {
  std::vector<std::string> ret;
  Doc doc;
  if (doc.Parse(req.data()).HasParseError()) {
    LOG_W("Parse data failed. {0} [offset {1}] {2}",
          GetParseError_En(doc.GetParseError()), doc.GetErrorOffset(), req);
    return ret;
  }

  const auto& marketCode = mdSvc_->getMarketCode();
  const auto& symbolType = mdSvc_->getSymbolType();
  const auto prefix =
      fmt::format("{}{}{}{}{}", TOPIC_PREFIX_OF_MARKET_DATA, SEP_OF_TOPIC,
                  marketCode, SEP_OF_TOPIC, symbolType);

  for (std::size_t i = 0; i < doc["params"].Size(); ++i) {
    const std::string params = doc["params"][i].GetString();
    const auto [splitRet, exchSymbolCode, exchMDType] =
        SplitStrIntoTwoParts(params, SEP_OF_EXCH_PARAMS);

    const auto [retOfGetSym, symbolCode] =
        mdSvc_->getTBLMonitorOfSymbolInfo()->getSymbolCode(
            marketCode, symbolType, exchSymbolCode);
    if (retOfGetSym != 0) {
      LOG_W("Get topic group for sub or unsub again failed. {}", req);
      continue;
    }

    const auto [retOfGetMDType, mdType] = GetMDType(exchMDType);
    if (retOfGetMDType != 0) {
      LOG_W("Get topic group for sub or unsub again failed. {}", req);
      continue;
    }

    const auto topic = fmt::format("{}{}{}{}{}", prefix, SEP_OF_TOPIC,
                                   symbolCode, SEP_OF_TOPIC, mdType);
    ret.emplace_back(topic);
  }

  return ret;
}

}  // namespace bq::md::svc::binance
