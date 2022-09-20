/*!
 * \file SHMIPCConst.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "SHMIPCDef.hpp"
#include "util/PchBase.hpp"

namespace bq {

const static std::string SEP_OF_SHM_SVC = "@";
constexpr static ClientChannel PUB_CHANNEL = 0;

constexpr static int TIMES_OF_WAIT_FOR_SUBSCRIBER = 300;
constexpr static std::uint32_t MAX_TOPIC_NAME_LEN = 32;

}  // namespace bq
