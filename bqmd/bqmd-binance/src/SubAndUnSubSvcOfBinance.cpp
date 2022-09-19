#include "SubAndUnSubSvcOfBinance.hpp"

#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "MDSvcOfBinanceConst.hpp"
#include "MDSvcOfBinanceUtil.hpp"
#include "TopicGroupMustSubMaint.hpp"

namespace bq::md::svc::binance {

std::tuple<WSReqGroup, WSReqGroup> SubAndUnSubSvcOfBinance::convertTopicToWSReq(
    TopicGroupNeedMaintSPtr& topicGroupNeedMaint) {
  auto subReqGroup = makeReqGroup(topicGroupNeedMaint, TopicOP::Sub);
  auto unSubReqGroup = makeReqGroup(topicGroupNeedMaint, TopicOP::UnSub);
  return {subReqGroup, unSubReqGroup};
}

/*
 * {
 *  "method": "SUBSCRIBE",
 *   "params": [
 *    "btcusdt@aggTrade",
 *    "btcusdt@depth"
 *   ],
 *   "id": 1
 * }
 */
WSReqGroup SubAndUnSubSvcOfBinance::makeReqGroup(
    TopicGroupNeedMaintSPtr& topicGroupNeedMaint, TopicOP topicOP) {
  TopicGroup topicGroup;
  std::string exchOP;
  if (topicOP == TopicOP::Sub) {
    topicGroup = topicGroupNeedMaint->topicGroupNeedSub_;
    exchOP = EXCH_OP_SUB;
  } else {
    topicGroup = topicGroupNeedMaint->topicGroupNeedUnSub_;
    exchOP = EXCH_OP_UNSUB;
  }

  WSReqGroup wsReqGroup;
  for (const auto& topic : topicGroup) {
    const auto [ret, exchSymbolCode, exchMDType] = GetExchSymAndMDType(topic);
    if (ret != 0) {
      LOG_W("Make req failed when handle {}.", topic);
      continue;
    }
    const auto param =
        fmt::format("{}{}{}", exchSymbolCode, SEP_OF_EXCH_PARAMS, exchMDType);
    rapidjson::StringBuffer strBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
    writer.StartObject();
    writer.Key("method");
    writer.String(exchOP.c_str());
    writer.Key("params");
    writer.StartArray();
    writer.String(param.c_str());
    writer.EndArray();
    writer.String("id");
    writer.Uint64(++id_);
    writer.EndObject();
    const auto req = strBuf.GetString();
    wsReqGroup.emplace_back(req);
  }
  return wsReqGroup;
}

std::tuple<int, std::string, std::string>
SubAndUnSubSvcOfBinance::GetExchSymAndMDType(const std::string& topic) const {
  std::vector<std::string> topicFieldGroup;
  boost::split(topicFieldGroup, topic, boost::is_any_of(SEP_OF_TOPIC));
  const auto marketCode = topicFieldGroup[1];
  const auto symbolCode = topicFieldGroup[3];

  auto [retOfGESC, exchSymbolCode] =
      mdSvc_->getTBLMonitorOfSymbolInfo()->getExchSymbolCode(marketCode,
                                                             symbolCode);
  if (retOfGESC != 0) {
    const auto statusMsg = fmt::format(
        "Get exch symbol code and market data type of topic {} failed.", topic);
    LOG_W(statusMsg);
    return {retOfGESC, "", ""};
  }

  auto [retOfGEMD, exchMDType] = GetExchMDType(topicFieldGroup);
  if (retOfGEMD != 0) {
    const auto statusMsg = fmt::format(
        "Get exch symbol code and market data type of topic {} failed.", topic);
    LOG_W(statusMsg);
    return {retOfGEMD, "", ""};
  }

  return std::make_tuple(0, exchSymbolCode, exchMDType);
}

}  // namespace bq::md::svc::binance
