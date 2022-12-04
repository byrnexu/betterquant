/*!
 * \file BQMDHis.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/26
 *
 * \brief
 */

#pragma once

#include "def/BQConst.hpp"
#include "util/Pch.hpp"

namespace bq::md {

enum class IndexType { ByExchTs, ByLocalTs };

using Ts2HisMDGroup = std::multimap<std::uint64_t, std::string>;
using Ts2HisMDGroupSPtr = std::shared_ptr<Ts2HisMDGroup>;

#pragma pack(push, 1)
struct Index {
  std::uint64_t ts_;
  std::uint64_t offset_;
  std::uint32_t lineLen_;
};
#pragma pack(pop)
using IndexSPtr = std::shared_ptr<Index>;

struct IndexWithFilename {
  std::uint64_t ts_;
  std::uint64_t offset_;
  std::uint32_t lineLen_;
  std::string filename_;
};
using IndexWithFilenameSPtr = std::shared_ptr<IndexWithFilename>;

using Ts2IndexWithFilenameGroup =
    boost::container::flat_multimap<std::uint64_t, IndexWithFilenameSPtr>;
using Ts2IndexWithFilenameGroupSPtr =
    std::shared_ptr<Ts2IndexWithFilenameGroup>;

class MDHis {
 public:
  MDHis() = delete;
  MDHis(const MDHis&) = delete;
  MDHis& operator=(const MDHis&) = delete;
  MDHis(const MDHis&&) = delete;
  MDHis& operator=(const MDHis&&) = delete;

 public:
  static std::tuple<int, Ts2HisMDGroupSPtr> LoadHisMDBetweenTs(
      const std::string& storageRootPath, const std::string& topic,
      std::uint64_t tsBegin, std::uint64_t tsEnd,
      IndexType indexType = IndexType::ByExchTs,
      std::uint32_t maxNumOfHisMDCanBeQeuryEachTime = 10000);

  static std::tuple<int, Ts2HisMDGroupSPtr> LoadHisMDBeforeTs(
      const std::string& storageRootPath, const std::string& topic,
      std::uint64_t ts, std::uint32_t num,
      IndexType indexType = IndexType::ByExchTs,
      std::uint32_t maxNumOfHisMDCanBeQeuryEachTime = 10000);

  static std::tuple<int, Ts2HisMDGroupSPtr> LoadHisMDAfterTs(
      const std::string& storageRootPath, const std::string& topic,
      std::uint64_t ts, std::uint32_t num,
      IndexType indexType = IndexType::ByExchTs,
      std::uint32_t maxNumOfHisMDCanBeQeuryEachTime = 10000);

  static int CreateIdxFileIfNotExists(const std::string& filename,
                                      IndexType indexType);

  static std::string ToJson(int statusCode,
                            const Ts2HisMDGroupSPtr& ts2HisMDGroup);

 private:
  static std::tuple<int, Ts2HisMDGroupSPtr> LoadTs2HisMDGroup(
      const Ts2IndexWithFilenameGroupSPtr& ts2IndexWithFilenameGroup);

  static boost::filesystem::path GetPathPrefixOfHisMD(
      const std::string& rootPath, const std::string& topic);

 public:
  static std::string GetMDFilenameByIdxFilename(const std::string& idxFilename,
                                                IndexType indexType);

 public:
  static std::tuple<int, std::vector<IndexSPtr>> MakeIndexGroup(
      const std::string& filename, IndexType indexType);

  static std::tuple<int, std::uint64_t> GetTsFromLine(const std::string& line,
                                                      IndexType indexType);

  static int SaveIndexGroupToFile(const std::string& filename,
                                  const std::vector<IndexSPtr>& indexGroup);

  static std::tuple<int, Ts2IndexWithFilenameGroupSPtr> LoadIndexGroupFromFile(
      const std::string& filename, IndexType indexType);

 private:
  inline const static int MAX_DATE_OFFSET{2};
};

}  // namespace bq::md
