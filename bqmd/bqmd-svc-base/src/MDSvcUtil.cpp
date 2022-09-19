#include "MDSvcUtil.hpp"

#include "def/BQConst.hpp"
#include "def/BQDef.hpp"

namespace bq::md::svc {

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

}  // namespace bq::md::svc
