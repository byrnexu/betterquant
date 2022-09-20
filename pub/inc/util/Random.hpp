/*!
 * \file Random.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "util/Pch.hpp"
#include "util/StdExt.hpp"

namespace bq {

class RandomStr : public boost::serialization::singleton<RandomStr> {
 public:
  void init();
  std::string get(int len = 8);

 private:
  const std::string charGroup{
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"};

  std::random_device rd_;
  std::mt19937 generator_;
  std::ext::spin_mutex mtxRandom_;
};

class RandomInt : public boost::serialization::singleton<RandomInt> {
 public:
  void init();
  std::uint64_t get();

 private:
  std::random_device rd_;
  std::mt19937 generator_;
  std::uniform_int_distribution<std::uint64_t> dis_{
      std::numeric_limits<std::uint64_t>::min(),
      std::numeric_limits<std::uint64_t>::max()};

  std::ext::spin_mutex mtxRandom_;
};

}  // namespace bq
