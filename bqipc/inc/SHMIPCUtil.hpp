/*!
 * \file SHMIPCUtil.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "SHMHeader.hpp"
#include "util/PchBase.hpp"

namespace bq {

template <typename T>
void InitMsgBody(void* target, const T source) {
  const auto targetAddr = static_cast<char*>(target) + sizeof(SHMHeader);
  const auto sourceAddr =
      reinterpret_cast<const char*>(&source) + sizeof(SHMHeader);
  const auto len = sizeof(T) - sizeof(SHMHeader);
  memcpy(targetAddr, sourceAddr, len);
}

}  // namespace bq
