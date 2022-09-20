/*!
 * \file Util.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "util/Util.hpp"

namespace bq {

void SetThreadName(const std::thread t, const std::string& name) {
#ifdef __linux___
  pthread_setname_np(t.native_handle(), name.c_str());
#endif
}

}  // namespace bq
