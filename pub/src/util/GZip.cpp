/*!
 * \file GZip.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/27
 *
 * \brief
 */

#include "util/GZip.hpp"

namespace bq {

StringSPtr GZip::comp(const StringSPtr& data) {
  const auto s = comp(*data);
  return std::make_shared<std::string>(s);
}

StringSPtr GZip::decomp(const StringSPtr& data) {
  const auto s = decomp(*data);
  return std::make_shared<std::string>(s);
}

std::string GZip::comp(const std::string& data) {
  namespace bio = boost::iostreams;

  std::stringstream compressed;
  std::stringstream origin(data);

  bio::filtering_streambuf<bio::input> out;
  out.push(bio::gzip_compressor(bio::gzip_params(bio::gzip::best_speed)));
  out.push(origin);
  bio::copy(out, compressed);

  return compressed.str();
}

std::string GZip::decomp(const std::string& data) {
  namespace bio = boost::iostreams;

  std::stringstream compressed(data);
  std::stringstream decompressed;

  bio::filtering_streambuf<bio::input> out;
  out.push(bio::gzip_decompressor());
  out.push(compressed);
  bio::copy(out, decompressed);

  return decompressed.str();
}

}  // namespace bq
