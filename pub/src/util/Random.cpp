/*!
 * \file Random.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "util/Random.hpp"

namespace bq {

void RandomStr::init() { generator_ = std::mt19937(rd_()); }

std::string RandomStr::get(int len) {
  std::string ret;
  ret.reserve(len + 1);

  const auto charGroupSize = charGroup.size() - 1;
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxRandom_);
    while (len > 0) {
      auto randNum = generator_();
      while (randNum > charGroupSize && len--) {
        ret.push_back(charGroup[randNum % charGroupSize]);
        randNum /= charGroupSize;
      }
    }
  }
  return ret;
}

void RandomInt::init() { generator_ = std::mt19937(rd_()); }

std::uint64_t RandomInt::get() {
  std::lock_guard<std::ext::spin_mutex> guard(mtxRandom_);
  return dis_(generator_);
}

}  // namespace bq
