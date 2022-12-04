/*!
 * \file CommonIPCData.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/25
 *
 * \brief
 */

#pragma once

#include "SHMHeader.hpp"
#include "util/PchBase.hpp"

namespace bq {

struct CommonIPCData {
  SHMHeader shmHeader_;
  std::uint32_t dataLen_{0};
  char data_[0];
  std::string toJson() const;
};
using CommonIPCDataSPtr = std::shared_ptr<CommonIPCData>;

CommonIPCDataSPtr MakeCommonIPCData(const std::string& str);

}  // namespace bq
