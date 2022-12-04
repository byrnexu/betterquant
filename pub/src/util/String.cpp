/*!
 * \file String.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "util/String.hpp"

#include "def/Def.hpp"
#include "util/Logger.hpp"

namespace bq {

bool isNumber(const std::string& s) {
  return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) {
                         return !std::isdigit(c);
                       }) == s.end();
}

std::uint64_t getHashIfIsNotNum(const std::string& s) {
  if (s.empty()) return 0;
  if (isNumber(s)) return CONV(std::uint64_t, s);
  const auto hash = XXH3_64bits(s.data(), s.size());
  return hash;
}

constexpr std::tuple<bool, std::string_view, std::string_view>
SplitStrIntoTwoParts(std::string_view str, std::string_view sep) {
  const auto pos = str.find(sep);
  if (pos == std::string::npos) {
    return {false, str, ""};
  }
  const std::string_view first = str.substr(0, pos);
  const std::string_view second = str.substr(pos + 1, str.size() - pos - 1);
  return {true, first, second};
}

std::tuple<bool, std::string, std::string> SplitStrIntoTwoParts(
    const std::string& str, const std::string& sep) {
  const auto pos = str.find(sep);
  if (pos == std::string::npos) {
    return {false, str, ""};
  }
  const auto first = str.substr(0, pos);
  const auto second = str.substr(pos + 1, str.size() - pos - 1);
  return {true, first, second};
}

std::vector<std::string_view> SplitStr(std::string_view strv,
                                       std::string_view sep) {
  std::vector<std::string_view> ret;
  auto first = strv.begin();
  while (first != std::end(strv)) {
    const auto second = std::find_first_of(first, std::cend(strv),
                                           std::cbegin(sep), std::cend(sep));
    if (first != second) {
      ret.emplace_back(strv.substr(std::distance(strv.begin(), first),
                                   std::distance(first, second)));
    }
    if (second == std::end(strv)) break;
    first = std::next(second);
  }

  return ret;
}

std::tuple<int, std::map<std::string, std::string>> Str2Map(
    std::string str, const std::string& sepOfRec,
    const std::string& sepOfField) {
  boost::remove_erase_if(str, boost::is_any_of(" \r\n"));
  std::vector<std::string> recGroup;
  boost::split(recGroup, str, boost::is_any_of(sepOfRec));

  std::map<std::string, std::string> ret;
  for (const auto& rec : recGroup) {
    std::vector<std::string> fieldGroup;
    boost::split(fieldGroup, rec, boost::is_any_of(sepOfField));
    if (fieldGroup.size() != 2) {
      LOG_W("Convert str to map failed because of invalid rec in str. {}", rec);
      return {-1, ret};
    }
    ret.emplace(boost::to_lower_copy(fieldGroup[0]), fieldGroup[1]);
  }
  return {0, ret};
}

std::string Map2Str(const std::map<std::string, std::string>& map,
                    const std::string& sepOfRec,
                    const std::string& sepOfField) {
  std::string ret;
  std::string sep = "";
  for (const auto& r : map) {
    const auto fieldName = r.first;
    const auto fieldValue = r.second;
    const auto rec = fmt::format("{}{}{}", fieldName, sepOfField, fieldValue);
    ret = ret + sep + rec;
    sep = sepOfRec;
  }
  return ret;
}

std::string SetParam(const std::string& origParam,
                     const std::string& updateParam,
                     const std::string& fixedParam, const std::string& sepOfRec,
                     const std::string& sepOfField) {
  auto [statusStackOfParseOrigParam, origMap] =
      Str2Map(origParam, sepOfRec, sepOfField);

  auto [statusStackOfParseUpdateParam, updateMap] =
      Str2Map(updateParam, sepOfRec, sepOfField);

  for (const auto& updateRec : updateMap) {
    const auto fieldName = updateRec.first;
    const auto fieldValue = updateRec.second;
    origMap[fieldName] = fieldValue;
  }

  if (!fixedParam.empty()) {
    auto [statusStackOfParseFixedParam, fixedMap] =
        Str2Map(fixedParam, sepOfRec, sepOfField);
    for (const auto& fixedRec : fixedMap) {
      const auto fieldName = fixedRec.first;
      const auto fieldValue = fixedRec.second;
      origMap[fieldName] = fieldValue;
    }
  }

  const auto ret = Map2Str(origMap, sepOfRec, sepOfField);
  return ret;
}

std::string RemoveTrailingZero(const std::string& value) {
  if (value.find('.') == std::string::npos) {
    return value;
  }

  std::uint32_t pos = 0;
  for (auto iter = value.rbegin(); iter != value.rend(); ++iter) {
    if (*iter == '0') {
      ++pos;
    } else {
      if (*iter == '.') {
        ++pos;
        break;
      } else {
        break;
      }
    }
  }

  return value.substr(0, value.size() - pos);
}

std::string MD5Sum(const char* const buffer, std::size_t buffer_size) {
  std::string ret;
  if (buffer == nullptr) {
    return ret;
  }
  boost::uuids::detail::md5 boost_md5;
  boost_md5.process_bytes(buffer, buffer_size);
  boost::uuids::detail::md5::digest_type digest;
  boost_md5.get_digest(digest);
  const auto char_digest = reinterpret_cast<const char*>(&digest);
  ret.clear();
  boost::algorithm::hex(
      char_digest, char_digest + sizeof(boost::uuids::detail::md5::digest_type),
      std::back_inserter(ret));
  return ret;
}

std::string MD5Sum(const std::string& str) {
  return MD5Sum(str.c_str(), str.size());
}

std::string Byte2Str(unsigned char* b, int n) {
  constexpr std::string_view HEX_CODES = "0123456789abcdef";
  std::string ret;
  for (int i = 0; i < n; ++i) {
    unsigned char binVal = b[i];
    ret += HEX_CODES[(binVal >> 4) & 0x0F];
    ret += HEX_CODES[binVal & 0x0F];
  }
  return ret;
}

std::string HMACSHA256(const std::string& str, const std::string& secKey,
                       int size) {
  unsigned char* data =
      reinterpret_cast<unsigned char*>(const_cast<char*>(str.c_str()));
  unsigned char* key =
      reinterpret_cast<unsigned char*>(const_cast<char*>(secKey.c_str()));
  unsigned char* digest;
  digest = HMAC(EVP_sha256(), key, int(secKey.size()), data, str.size(),
                nullptr, nullptr);
  const auto ret = Byte2Str(digest, size);
  return ret;
}

void ToUpper(char* s, std::size_t size) {
  for (std::size_t i = 0; i < size; ++i) {
    if (s[i] == '\0') break;
    s[i] = std::toupper(static_cast<unsigned char>(s[i]));
  }
}

std::string ReplaceSubStrBetween2Str(const std::string& str,
                                     const std::string& replaceStr,
                                     std::string_view leftStr,
                                     std::string_view rightStr) {
  const auto left = str.find(leftStr);
  const auto right = str.find(rightStr, left + leftStr.size());
  const auto ret = str.substr(0, left + leftStr.size()) + replaceStr +
                   str.substr(right, str.size());
  return ret;
}

}  // namespace bq
