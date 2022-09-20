/*!
 * \file Float.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "util/Pch.hpp"

namespace bq {

// implements relative method - do not use for comparing with zero
// use this most of the time, tolerance needs to be meaningful in your context
template <typename TReal>
static bool isApproximatelyEqual(
    TReal a, TReal b, TReal tolerance = std::numeric_limits<TReal>::epsilon()) {
  TReal diff = std::fabs(a - b);
  if (diff <= tolerance) return true;

  if (diff < std::fmax(std::fabs(a), std::fabs(b)) * tolerance) return true;

  return false;
}

// supply tolerance that is meaningful in your context
// for example, default tolerance may not work if you are comparing double with
// float
template <typename TReal>
static bool isApproximatelyZero(
    TReal a, TReal tolerance = std::numeric_limits<TReal>::epsilon()) {
  if (std::fabs(a) <= tolerance) return true;
  return false;
}

// use this when you want to be on safe side
// for example, don't start rover unless signal is above 1
template <typename TReal>
static bool isDefinitelyLessThan(
    TReal a, TReal b, TReal tolerance = std::numeric_limits<TReal>::epsilon()) {
  TReal diff = a - b;
  if (diff < tolerance) return true;

  if (diff < std::fmax(std::fabs(a), std::fabs(b)) * tolerance) return true;

  return false;
}

template <typename TReal>
static bool isDefinitelyGreaterThan(
    TReal a, TReal b, TReal tolerance = std::numeric_limits<TReal>::epsilon()) {
  TReal diff = a - b;
  if (diff > tolerance) return true;

  if (diff > std::fmax(std::fabs(a), std::fabs(b)) * tolerance) return true;

  return false;
}

template <typename TReal>
static bool isDefinitelyGreaterOrEqual(
    TReal a, TReal b, TReal tolerance = std::numeric_limits<TReal>::epsilon()) {
  return isDefinitelyGreaterThan(a, b, tolerance) ||
         isApproximatelyEqual(a, b, tolerance);
}

template <typename TReal>
static bool isDefinitelyLessOrEqual(
    TReal a, TReal b, TReal tolerance = std::numeric_limits<TReal>::epsilon()) {
  return isDefinitelyLessThan(a, b, tolerance) ||
         isApproximatelyEqual(a, b, tolerance);
}

}  // namespace bq
