/*!
 * \file File.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "util/File.hpp"

#include "util/Logger.hpp"

namespace bq {

bool AppendStrToFile(const std::string& filename, const std::string& filecont) {
  assert(!filename.empty());
  assert(!filecont.empty());

  std::ofstream out(filename.c_str(), std::ios::app);
  if (out.is_open() == false) {
#ifdef __linux__
    LOG_W(
        "Open file failed when append string to file. [filename = {}, errno = "
        "{}, errmsg = {}]",
        filename, errno, strerror(errno));
#endif
    return false;
  }
  out << filecont;
  out.close();

  return true;
}

bool OverwriteStrToFile(const std::string& filename,
                        const std::string& filecont) {
  assert(!filename.empty());
  assert(!filecont.empty());

  std::ofstream out(filename.c_str(), std::ios::trunc);
  if (out.is_open() == false) {
#ifdef __linux__
    LOG_W(
        "Open file failed when overwrite string to file. [filename = {}, errno "
        "= {}, errmsg = {}]",
        filename, errno, strerror(errno));
#endif
    return false;
  }
  out << filecont;
  out.close();
  return true;
}

std::string LoadFileContToStr(const std::string& filename) {
  std::ifstream in(filename.c_str(), std::ios::binary | std::ios::ate);
  const std::streampos pos = in.tellg();
  if (pos == std::streampos(-1)) {
    LOG_D("Failed to read the file to a string. [filename = {}]", filename);
    return "";
  }
  std::string str(pos, '\0');
  in.seekg(0);
  in.read(&str[0], pos);
  in.close();
  return str;
};

int ClearFilecont(const std::string& filename) {
  std::ofstream out;
  out.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
  if (out.is_open() == false) {
    return -1;
  }
  out.close();
  return 0;
}

std::tuple<int, std::vector<boost::filesystem::path>>
GetFileGroupFromPathRecursively(const std::string& path) {
  std::vector<boost::filesystem::path> fileGroup;
  try {
    boost::filesystem::path p(path);
    for (const auto& rec : boost::filesystem::directory_iterator(p)) {
      if (boost::filesystem::is_directory(rec.path())) {
        continue;
      }
      fileGroup.push_back(rec.path());
    }
    return {0, fileGroup};

  } catch (const std::exception& e) {
    LOG_W("Get file group from path {} recursively failed. [{}]", path,
          e.what());
    return {-1, fileGroup};
  }
}

}  // namespace bq
