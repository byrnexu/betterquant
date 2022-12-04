/*!
 * \file BQUtil.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "util/BQUtil.hpp"

#include "SHMIPC.hpp"
#include "def/BQConst.hpp"
#include "def/DataStruOfAssets.hpp"
#include "def/DataStruOfOthers.hpp"
#include "def/DataStruOfStg.hpp"
#include "def/DataStruOfTD.hpp"
#include "def/StatusCode.hpp"
#include "util/Logger.hpp"
#include "util/String.hpp"

namespace bq {

std::tuple<int, std::string> GetAddrFromTopic(const std::string& appName,
                                              const std::string& topic) {
  std::vector<std::string> fieldGroup;
  boost::split(fieldGroup, topic, boost::is_any_of(SEP_OF_TOPIC));
  if (fieldGroup.size() < 3) {
    LOG_W("Get shm svc addr from topic failed because of invalid topic {}.",
          topic);
    return {SCODE_BQPUB_INVALID_TOPIC, ""};
  }
  const auto topicType = fieldGroup[0];
  const auto marketCode = fieldGroup[1];
  const auto symbolType = fieldGroup[2];
  const auto ret =
      fmt::format("{}{}{}{}{}{}{}", appName, SEP_OF_SHM_SVC, topicType,
                  SEP_OF_SHM_SVC, marketCode, SEP_OF_SHM_SVC, symbolType);
  return {0, ret};
}

std::tuple<int, std::string> GetChannelFromAddr(const std::string& shmSvcAddr) {
  std::vector<std::string> fieldGroup;
  boost::split(fieldGroup, shmSvcAddr, boost::is_any_of(SEP_OF_SHM_SVC));
  if (fieldGroup.size() < 4) {
    LOG_W("Get channel from shm svc addr failed because of invalid addr {}.",
          shmSvcAddr);
    return {SCODE_BQPUB_INVALID_TOPIC, ""};
  }
  const auto marketCode = fieldGroup[2];
  const auto symbolType = fieldGroup[3];
  const auto ret =
      fmt::format("{}{}{}{}{}", TOPIC_PREFIX_OF_MARKET_DATA, SEP_OF_SHM_SVC,
                  marketCode, SEP_OF_SHM_SVC, symbolType);
  return {0, ret};
}

AcctId GetAcctIdFromTask(const SHMIPCTaskSPtr& task) {
  AcctId ret = 0;
  const auto shmHeader = static_cast<const SHMHeader*>(task->data_);
  switch (shmHeader->msgId_) {
    case MSG_ID_ON_ORDER:
    case MSG_ID_ON_CANCEL_ORDER:
    case MSG_ID_ON_ORDER_RET:
    case MSG_ID_ON_CANCEL_ORDER_RET:
      ret = static_cast<const OrderInfo*>(task->data_)->acctId_;
      break;
    case MSG_ID_SYNC_ASSETS:
      ret = static_cast<const AssetInfoNotify*>(task->data_)->acctId_;
      break;
    default:
      break;
  }
  return ret;
}

std::string convertTopic(const std::string& topic) {
  if (boost::starts_with(topic, "shm") == false) return topic;
  std::string ret = topic;
  boost::algorithm::erase_first(ret, "shm://");
  boost::algorithm::replace_all(ret, ".", SEP_OF_TOPIC);
  boost::algorithm::replace_all(ret, "/", SEP_OF_TOPIC);
  return ret;
}

std::string ToPrettyStr(Decimal value) {
  std::stringstream sstr;
  sstr << std::setprecision(DBL_PREC) << value;
  std::string ret = sstr.str();
  return RemoveTrailingZero(ret);
}

std::string MakeCommonHttpBody(int statusCode, std::string data) {
  const auto statusMsg = GetStatusMsg(statusCode);
  auto ret = fmt::format(R"({{"statusCode":{},"stausMsg":"{}")", statusCode,
                         statusMsg);
  if (!data.empty()) {
    data[0] = ',';
  } else {
    data = "}";
  }
  ret = ret + data;
  return ret;
}

std::tuple<std::string, TopicHash> MakeTopicInfo(const std::string& marketCode,
                                                 const std::string& symbolType,
                                                 const std::string& symbolCode,
                                                 MDType mdType,
                                                 const std::string& ext) {
  auto topic = fmt::format("{}{}{}{}{}{}{}{}{}", TOPIC_PREFIX_OF_MARKET_DATA,
                           SEP_OF_TOPIC, marketCode,  //
                           SEP_OF_TOPIC, symbolType,  //
                           SEP_OF_TOPIC, symbolCode,  //
                           SEP_OF_TOPIC, magic_enum::enum_name(mdType));
  if (!ext.empty()) {
    topic.append(fmt::format("{}{}", SEP_OF_TOPIC, ext));
  }
  const auto topicHash = XXH3_64bits(topic.data(), topic.size());
  return {topic, topicHash};
}

// clang-format off
void PrintLogo() {
  std::cout <<
  R"(
        __         __  __                                       __ 
       / /_  ___  / /_/ /____  _____   ____ ___  ______ _____  / /_
      / __ \/ _ \/ __/ __/ _ \/ ___/  / __ `/ / / / __ `/ __ \/ __/
     / /_/ /  __/ /_/ /_/  __/ /     / /_/ / /_/ / /_/ / / / / /_  
    /_.___/\___/\__/\__/\___/_/      \__, /\__,_/\__,_/_/ /_/\__/  
                                       /_/   
  )" 
  << std::endl 
  << std::endl;
}
// clang-format on

}  // namespace bq
