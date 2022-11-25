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
  boost::split(fieldGroup, topic, boost::is_any_of(SEP_OF_SHM_SVC));
  if (fieldGroup.size() < 3) {
    LOG_W("Get pub channel from topic failed because of invalid topic {}.",
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
