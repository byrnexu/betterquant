#pragma once

#include "def/BQConst.hpp"
#include "util/Pch.hpp"

namespace bq {

using ExchTs2HisMDGroup = std::multimap<std::uint64_t, std::string>;
using ExchTs2HisMDGroupSPtr = std::shared_ptr<ExchTs2HisMDGroup>;

class HisMD {
 public:
  HisMD() = delete;
  HisMD(const HisMD&) = delete;
  HisMD& operator=(const HisMD&) = delete;
  HisMD(const HisMD&&) = delete;
  HisMD& operator=(const HisMD&&) = delete;

 public:
  static boost::filesystem::path GetPathOfHisMD(const std::string& rootPath,
                                                const std::string& topic);

  static std::tuple<bool, std::uint64_t, std::string> GetExchTsAndMDFromLine(
      const std::string& line);

  static ExchTs2HisMDGroupSPtr LoadHisMDOfDate(
      const std::string& storageRootPath, const std::string& topic,
      const std::string& date);

  static std::tuple<int, ExchTs2HisMDGroupSPtr> LoadHisMDBetweenTs(
      const std::string& storageRootPath, const std::string& topic,
      std::uint64_t tsBegin, std::uint64_t tsEnd,
      std::uint32_t maxNumOfHisMDCanBeQeuryEachTime = 10000);

  static std::tuple<int, ExchTs2HisMDGroupSPtr> LoadHisMDBeforeTs(
      const std::string& storageRootPath, const std::string& topic,
      std::uint64_t ts, std::uint32_t num,
      std::uint32_t maxNumOfHisMDCanBeQeuryEachTime = 10000);

  static std::tuple<int, ExchTs2HisMDGroupSPtr> LoadHisMDAfterTs(
      const std::string& storageRootPath, const std::string& topic,
      std::uint64_t ts, std::uint32_t num,
      std::uint32_t maxNumOfHisMDCanBeQeuryEachTime = 10000);

  static std::string ToJson(int statusCode,
                            const ExchTs2HisMDGroupSPtr& exchTs2HisMDGroup);

 private:
  inline const static int MAX_DATE_OFFSET{2};
};

}  // namespace bq
