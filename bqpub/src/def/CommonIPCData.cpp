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

CommonIPCDataSPtr MakeCommonIPCData(const std::string& str) {
  void* buff = malloc(sizeof(CommonIPCData) + str.size() + 1);
  std::shared_ptr<CommonIPCData> ret(static_cast<CommonIPCData*>(buff));
  ret->dataLen_ = str.size();
  memcpy(ret->data_, str.c_str(), str.size());
  return ret;
}

}  // namespace bq
