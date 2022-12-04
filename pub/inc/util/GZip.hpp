/*!
 * \file GZip.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/27
 *
 * \brief
 */

#pragma once

#include "def/Def.hpp"
#include "util/Pch.hpp"

namespace bq {

class GZip {
 public:
  static StringSPtr comp(const StringSPtr& data);
  static StringSPtr decomp(const StringSPtr& data);

  static std::string comp(const std::string& data);
  static std::string decomp(const std::string& data);
};

}  // namespace bq
