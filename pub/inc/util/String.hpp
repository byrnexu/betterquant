/*!
 * \file String.hpp
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

bool isNumber(const std::string& s);
std::uint64_t getHashIfIsNotNum(const std::string& s);

constexpr std::tuple<bool, std::string_view, std::string_view>
SplitStrIntoTwoParts(std::string_view str, std::string_view sep);

std::tuple<bool, std::string, std::string> SplitStrIntoTwoParts(
    const std::string& str, const std::string& sep);

std::vector<std::string_view> SplitStr(std::string_view strv,
                                       std::string_view sep = " ");

std::tuple<int, std::map<std::string, std::string>> Str2Map(
    std::string str, const std::string& sepOfRec = ";",
    const std::string& sepOfField = "=");

std::string Map2Str(const std::map<std::string, std::string>& map,
                    const std::string& sepOfRec = ";",
                    const std::string& sepOfField = "=");

std::string SetParam(const std::string& origParam,
                     const std::string& updateParam,
                     const std::string& fixedParam = "",
                     const std::string& sepOfRec = ";",
                     const std::string& sepOfField = "=");

std::string RemoveTrailingZero(const std::string& value);

std::string MD5Sum(const char* const buffer, std::size_t buffer_size);
std::string MD5Sum(const std::string& str);

std::string Byte2Str(unsigned char* b, int n);
std::string HMACSHA256(const std::string& str, const std::string& secKey,
                       int size);

void ToUpper(char* s, std::size_t size);

template <bool b, unsigned N>
struct Int2StrInCompileTimeBase {
  typedef typename boost::mpl::push_back <
      typename Int2StrInCompileTimeBase<
          N<10, N / 10>::type, boost::mpl::char_<'0' + N % 10>>::type type;
};

template <>
struct Int2StrInCompileTimeBase<true, 0> {
  typedef boost::mpl::string<> type;
};

template <unsigned N>
struct Int2StrInCompileTime {
  typedef typename boost::mpl::c_str <
      typename Int2StrInCompileTimeBase<N<10, N>::type>::type type;
};

}  // namespace bq
