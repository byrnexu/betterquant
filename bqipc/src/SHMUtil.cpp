#include "SHMUtil.hpp"

namespace bq {

std::once_flag& GetOnceFlagOfAssignAppName() {
  static std::once_flag ret;
  return ret;
}

}  // namespace bq
