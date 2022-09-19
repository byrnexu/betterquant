#pragma once

#include "util/Pch.hpp"

namespace bq::md {

enum class TopicOP { Sub = 1, UnSub = 2 };

template <typename T>
struct DepthData {
  explicit DepthData(const std::string& data) {
    std::vector<std::string> fieldGroup;
    boost::split(fieldGroup, data, boost::is_any_of("\"[],"));
    price_ = fieldGroup[2];
    size_ = fieldGroup[5];
    orderNum_ = fieldGroup[11];
  }

  DepthData(const T& price, const T& size)
      : price_(price), size_(size), orderNum_("0") {}

  T price_;
  T size_;
  std::string orderNum_;
};
template <typename T>
using DepthDataSPtr = std::shared_ptr<DepthData<T>>;

using Price = std::uint64_t;

template <typename T>
using Asks = std::map<Price, DepthDataSPtr<T>>;
template <typename T>
using AsksSPtr = std::shared_ptr<Asks<T>>;

template <typename T>
using Bids = std::map<Price, DepthDataSPtr<T>, std::greater<Price>>;
template <typename T>
using BidsSPtr = std::shared_ptr<Bids<T>>;

}  // namespace bq::md
