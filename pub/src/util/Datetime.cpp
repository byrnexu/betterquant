/*!
 * \file Datetime.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "util/Datetime.hpp"

#include "util/Logger.hpp"
#include "util/StdExt.hpp"

namespace bq {

std::uint64_t GetTotalNSSince1970() {
  struct timespec time_start = {0, 0};
  clock_gettime(CLOCK_REALTIME, &time_start);
  std::uint64_t ret = time_start.tv_sec * 1000000000 + time_start.tv_nsec;
  return ret;
}

std::uint64_t GetTotalUSSince1970() {
  boost::posix_time::ptime epoch(
      boost::gregorian::date(1970, boost::gregorian::Jan, 1));
  const auto now = boost::posix_time::microsec_clock::universal_time();
  const auto time_duration = now - epoch;
  return time_duration.total_microseconds();
}

std::uint64_t GetTotalMSSince1970() {
  const auto ret = GetTotalUSSince1970() / 1000;
  return ret;
}

std::uint64_t GetTotalSecSince1970() {
  const auto ret = GetTotalUSSince1970() / 1000000;
  return ret;
}

std::string ConvertTsToDBTime(std::uint64_t ts) {
  const auto t = boost::posix_time::from_time_t(ts / 1000000);
  const auto us = ts % 1000000;
  std::string ret = boost::posix_time::to_iso_extended_string(t);
  ret[10] = ' ';
  ret = fmt::format("{}.{:06}", ret, us);
  return ret;
}

std::string ConvertTsToPtime(std::uint64_t ts) {
  const auto t = boost::posix_time::from_time_t(ts / 1000000);
  const auto us = ts % 1000000;
  const auto ret = fmt::format("{}.{:06}", to_iso_string(t), us);
  return ret;
}

std::uint64_t ConvertDBTimeToTS(std::string dbTime) {
  dbTime[10] = 'T';
  std::ext::erase_if(dbTime, [](char c) { return c == '-' || c == ':'; });
  const auto [ret, ts] = ConvertISODatetimeToTs(dbTime);
  return ts;
}

std::tuple<int, std::uint64_t> ConvertISODatetimeToTs(
    const std::string& isoDatetime) {
  try {
    const auto t = boost::posix_time::from_iso_string(isoDatetime);
    boost::posix_time::ptime epoch(
        boost::gregorian::date(1970, boost::gregorian::Jan, 1));
    const auto time_duration = t - epoch;
    return {0, time_duration.total_microseconds()};
  } catch (const std::exception& e) {
    const auto statusMsg = fmt::format(
        "Convert iso datetime {} to ts failed. [{}]", isoDatetime, e.what());
    LOG_W(statusMsg);
    return {-1, 0};
  }
}

std::string GetDateInStrFmtFromTs(std::uint64_t ts) {
  const auto datetime = ConvertTsToPtime(ts);
  const auto ret = datetime.substr(0, 8);
  return ret;
}

boost::gregorian::date GetDateFromTs(std::uint64_t ts) {
  const auto datetime = boost::posix_time::from_time_t(ts);
  const auto ret = datetime.date();
  return ret;
}

}  // namespace bq
