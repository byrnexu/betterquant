#pragma once

#include "util/PchBase.hpp"

namespace bq {

inline std::uint32_t ThreadNo(std::uint32_t value) { return value; }
inline std::uint32_t MilliSecInterval(std::uint32_t value) { return value; }
inline std::uint64_t ExecTimes(std::uint64_t value) { return value; }

inline std::string QuoteCurrencyForCalc(const std::string& value) {
  return value;
}
inline std::string QuoteCurrencyForConv(const std::string& value) {
  return value;
}

}  // namespace bq
