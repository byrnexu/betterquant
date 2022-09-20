/*!
 * \file BQMDUtil.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "util/BQMDUtil.hpp"

#include "def/BQConst.hpp"
#include "def/Def.hpp"

namespace bq::md {

std::string RemoveDepthInTopicOfBooks(const std::string& topic) {
  assert(topic.empty() == false && "topic.empty() == false");
  const auto booksIdentity =
      fmt::format("{}{}", SEP_OF_TOPIC, magic_enum::enum_name(MDType::Books));
  const auto range = boost::find_first(topic, booksIdentity);
  std::string ret;
  ret.assign(std::begin(topic), std::end(range));
  return ret;
}

}  // namespace bq::md
