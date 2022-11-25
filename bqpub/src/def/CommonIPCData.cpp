/*!
 * \file CommonIPCData.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/25
 *
 * \brief
 */

#include "def/CommonIPCData.hpp"

namespace bq {

std::string CommonIPCData::toJson() const {
  std::string ret;
  ret = R"({"shmHeader":)" + shmHeader_.toJson() + ",";
  ret = ret + R"("data":)" + R"(")" + data_ + R"("})";
  return ret;
}

}  // namespace bq
