/*!
 * \file Util.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "def/Def.hpp"
#include "util/Datetime.hpp"
#include "util/Pch.hpp"

namespace bq {

#define EXEC_PERF_TEST(hint, startTs, times, timesOfLogInterval)             \
  const auto now_f7ob25ln = GetTotalUSSince1970();                           \
  const auto td_f7ob25ln = now_f7ob25ln - startTs;                           \
                                                                             \
  static std::ext::spin_mutex mtx_f7ob25ln;                                  \
  static std::vector<std::uint64_t> tdGroup_f7ob25ln;                        \
  static std::uint64_t total_f7ob25ln = 0;                                   \
  {                                                                          \
    std::lock_guard<std::ext::spin_mutex> guard(mtx_f7ob25ln);               \
    if (tdGroup_f7ob25ln.size() < times - 1) {                               \
      total_f7ob25ln += td_f7ob25ln;                                         \
      tdGroup_f7ob25ln.emplace_back(td_f7ob25ln);                            \
      if (tdGroup_f7ob25ln.size() % timesOfLogInterval == 0)                 \
        LOG_I("{} Total num: {}; avg: {:.5f}; times: {}", (hint),            \
              tdGroup_f7ob25ln.size(),                                       \
              (double)total_f7ob25ln / (double)tdGroup_f7ob25ln.size(),      \
              times);                                                        \
    } else if (tdGroup_f7ob25ln.size() == times - 1) {                       \
      total_f7ob25ln += td_f7ob25ln;                                         \
      tdGroup_f7ob25ln.emplace_back(td_f7ob25ln);                            \
      std::sort(std::begin(tdGroup_f7ob25ln), std::end(tdGroup_f7ob25ln));   \
      const auto iter = std::next(std::begin(tdGroup_f7ob25ln),              \
                                  tdGroup_f7ob25ln.size() / 2);              \
      LOG_I("{} Total num: {}; avg: {:.5f}; med: {}; min: {}; max: {}",      \
            (hint), tdGroup_f7ob25ln.size(),                                 \
            (double)total_f7ob25ln / (double)tdGroup_f7ob25ln.size(), *iter, \
            *std::begin(tdGroup_f7ob25ln), *std::rbegin(tdGroup_f7ob25ln));  \
    }                                                                        \
  }

struct AutoFree {
  void operator()(void* buf) { SAFE_FREE(buf); }
};

struct AutoFreeYYDoc {
  void operator()(yyjson_doc* doc) {
    if (doc) {
      yyjson_doc_free(doc);
      doc = nullptr;
    }
  }
};

void SetThreadName(const std::thread t, const std::string& name);

}  // namespace bq
