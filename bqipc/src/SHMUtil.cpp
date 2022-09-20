/*!
 * \file SHMUtil.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "SHMUtil.hpp"

namespace bq {

std::once_flag& GetOnceFlagOfAssignAppName() {
  static std::once_flag ret;
  return ret;
}

}  // namespace bq
