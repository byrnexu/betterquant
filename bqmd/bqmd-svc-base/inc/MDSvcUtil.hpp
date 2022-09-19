#pragma once

#include "def/BQConst.hpp"
#include "def/BQDef.hpp"
#include "def/Def.hpp"
#include "util/Pch.hpp"

namespace bq {
enum class MDType : std::uint8_t;
}

namespace bq::md::svc {

std::tuple<std::string, TopicHash> MakeTopicInfo(const std::string& marketCode,
                                                 const std::string& symbolType,
                                                 const std::string& symbolCode,
                                                 MDType mdType,
                                                 const std::string& ext = "");

}
