/*!
 * \file Config.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "util/ConfigBase.hpp"
#include "util/Pch.hpp"

namespace bq {

class Config : public ConfigBase,
               public boost::serialization::singleton<Config> {};

}  // namespace bq
