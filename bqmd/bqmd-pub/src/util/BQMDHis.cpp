/*!
 * \file BQMDHis.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/26
 *
 * \brief
 */

#include "util/BQMDHis.hpp"

#include "def/Const.hpp"
#include "def/Def.hpp"
#include "def/StatusCode.hpp"
#include "util/File.hpp"
#include "util/Logger.hpp"
#include "util/Util.hpp"

namespace bq::md {

std::tuple<int, Ts2HisMDGroupSPtr> MDHis::LoadHisMDBetweenTs(
    const std::string& storageRootPath, const std::string& topic,
    std::uint64_t tsBegin, std::uint64_t tsEnd, IndexType indexType,
    std::uint32_t maxNumOfHisMDCanBeQeuryEachTime) {
  using namespace boost::gregorian;

  if (tsBegin > tsEnd) {
    const auto statusMsg = fmt::format(
        "Load his market data between 2 ts failed because "
        "tsBegin {} greater than tsEnd {}. topic = {}",
        tsBegin, tsEnd, topic);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_INVALID_TS, std::make_shared<Ts2HisMDGroup>()};
  }

  const auto dateBegin = GetDateFromTs(tsBegin / 1000000);
  const auto maxDataOfHisMD =
      day_clock::universal_day() + date_duration(MAX_DATE_OFFSET);
  if (dateBegin > maxDataOfHisMD) {
    const auto statusMsg = fmt::format(
        "Load his market data between 2 ts failed because "
        "tsBegin {} greater than {}. topic = {}",
        tsBegin, to_iso_string(maxDataOfHisMD), topic);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_INVALID_TS, std::make_shared<Ts2HisMDGroup>()};
  }

  const auto dateEnd = GetDateFromTs(tsEnd / 1000000);
  if (dateEnd < from_undelimited_string(MIN_DATE_OF_HIS_MD)) {
    const auto statusMsg = fmt::format(
        "Load his market data between 2 ts failed because "
        "tsEnd {} less than {}. topic = {}",
        tsEnd, MIN_DATE_OF_HIS_MD, topic);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_INVALID_TS, std::make_shared<Ts2HisMDGroup>()};
  }

  LOG_D("Begin to load index of his market data between {} and {}.",
        to_iso_string(dateBegin), to_iso_string(dateEnd));

  const auto pathPrefix = GetPathPrefixOfHisMD(storageRootPath, topic);

  auto ts2IndexWithFilenameGroup =
      std::make_shared<Ts2IndexWithFilenameGroup>();
  for (day_iterator iter(dateBegin); iter <= dateEnd; ++iter) {
    const auto date = to_iso_string(*iter);

    const auto idxFilename =
        fmt::format("{}.{}.{}", date, HIS_MD_FILE_EXT,
                    indexType == IndexType::ByExchTs ? HIS_MD_INDEX_BY_ET_EXT
                                                     : HIS_MD_INDEX_BY_LT_EXT);
    const auto pathOfIdx = pathPrefix / idxFilename;

    CreateIdxFileIfNotExists(pathOfIdx.string(), indexType);
    auto [statusCodeOfLoad, ts2IndexWithFilenameGroupOfCurDate] =
        LoadIndexGroupFromFile(pathOfIdx.string(), indexType);
    if (statusCodeOfLoad != 0) {
      continue;
    }

    if (*iter == dateBegin) {
      const auto iterBegin =
          ts2IndexWithFilenameGroupOfCurDate->lower_bound(tsBegin);
      ts2IndexWithFilenameGroupOfCurDate->erase(
          std::begin(*ts2IndexWithFilenameGroupOfCurDate), iterBegin);
    }
    if (*iter == dateEnd) {
      const auto iterEnd =
          ts2IndexWithFilenameGroupOfCurDate->lower_bound(tsEnd);
      ts2IndexWithFilenameGroupOfCurDate->erase(
          iterEnd, std::end(*ts2IndexWithFilenameGroupOfCurDate));
    }
    ts2IndexWithFilenameGroup->merge(*ts2IndexWithFilenameGroupOfCurDate);
    if (ts2IndexWithFilenameGroup->size() > maxNumOfHisMDCanBeQeuryEachTime) {
      const auto statusMsg = fmt::format(
          "Load index of his market data between ts failed because "
          "rec num of result greater than the query limit {}. topic = {}",
          maxNumOfHisMDCanBeQeuryEachTime, topic);
      LOG_W(statusMsg);
      return {SCODE_HIS_MD_NUM_OF_RECORDS_GREATER_THAN_LIMIT,
              std::make_shared<Ts2HisMDGroup>()};
    }
  }

  const auto [statusCode, ts2HisMDGroup] =
      LoadTs2HisMDGroup(ts2IndexWithFilenameGroup);
  if (statusCode != 0) {
    return {statusCode, ts2HisMDGroup};
  }

  return {0, ts2HisMDGroup};
}

std::string MDHis::GetMDFilenameByIdxFilename(const std::string& idxFilename,
                                              IndexType indexType) {
  const auto extnameOfIdx = fmt::format(".{}", indexType == IndexType::ByExchTs
                                                   ? HIS_MD_INDEX_BY_ET_EXT
                                                   : HIS_MD_INDEX_BY_LT_EXT);
  const auto ret =
      boost::algorithm::erase_tail_copy(idxFilename, extnameOfIdx.size());
  return ret;
}

std::tuple<int, Ts2HisMDGroupSPtr> MDHis::LoadHisMDBeforeTs(
    const std::string& storageRootPath, const std::string& topic,
    std::uint64_t ts, std::uint32_t num, IndexType indexType,
    std::uint32_t maxNumOfHisMDCanBeQeuryEachTime) {
  using namespace boost::gregorian;

  if (num > maxNumOfHisMDCanBeQeuryEachTime) {
    const auto statusMsg = fmt::format(
        "Load his market data before ts failed because "
        "rec num of result {} greater than the query limit {}. topic = {}",
        num, maxNumOfHisMDCanBeQeuryEachTime, topic);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_NUM_OF_RECORDS_GREATER_THAN_LIMIT,
            std::make_shared<Ts2HisMDGroup>()};
  }

  const auto dateBegin = GetDateFromTs(ts / 1000000);
  if (dateBegin < from_undelimited_string(MIN_DATE_OF_HIS_MD)) {
    const auto statusMsg = fmt::format(
        "Load his markete data before ts failed because "
        "ts {} less than {}. topic = {}",
        ts, MIN_DATE_OF_HIS_MD, topic);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_INVALID_TS, std::make_shared<Ts2HisMDGroup>()};
  }

  if (dateBegin > day_clock::universal_day() + date_duration(MAX_DATE_OFFSET)) {
    const auto statusMsg = fmt::format(
        "Load his market data before ts failed because "
        "ts {} greater than the day after tomorrow. topic = {}",
        ts, topic);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_INVALID_TS, std::make_shared<Ts2HisMDGroup>()};
  }

  const auto dateEnd = from_undelimited_string(MIN_DATE_OF_HIS_MD);
  LOG_D("Begin to load {} numbers of index of his market data before {}.", num,
        to_iso_string(dateBegin));

  const auto pathPrefix = GetPathPrefixOfHisMD(storageRootPath, topic);

  auto ts2IndexWithFilenameGroup =
      std::make_shared<Ts2IndexWithFilenameGroup>();
  for (day_iterator iter(dateBegin); iter >= dateEnd; --iter) {
    const auto date = to_iso_string(*iter);

    const auto idxFilename =
        fmt::format("{}.{}.{}", date, HIS_MD_FILE_EXT,
                    indexType == IndexType::ByExchTs ? HIS_MD_INDEX_BY_ET_EXT
                                                     : HIS_MD_INDEX_BY_LT_EXT);
    const auto pathOfIdx = pathPrefix / idxFilename;

    CreateIdxFileIfNotExists(pathOfIdx.string(), indexType);
    auto [statusCodeOfLoad, ts2IndexWithFilenameGroupOfCurDate] =
        LoadIndexGroupFromFile(pathOfIdx.string(), indexType);
    if (statusCodeOfLoad != 0) {
      continue;
    }

    ts2IndexWithFilenameGroup->merge(*ts2IndexWithFilenameGroupOfCurDate);
    if (*iter == dateBegin) {
      auto iterUpperBound = ts2IndexWithFilenameGroup->upper_bound(ts);
      ts2IndexWithFilenameGroup->erase(iterUpperBound,
                                       std::end(*ts2IndexWithFilenameGroup));
    }
    if (ts2IndexWithFilenameGroup->size() >= num) {
      const auto offset = ts2IndexWithFilenameGroup->size() - num;
      ts2IndexWithFilenameGroup->erase(
          std::begin(*ts2IndexWithFilenameGroup),
          std::next(std::begin(*ts2IndexWithFilenameGroup), offset));
      break;
    }
  }

  const auto [statusCode, ts2HisMDGroup] =
      LoadTs2HisMDGroup(ts2IndexWithFilenameGroup);
  if (statusCode != 0) {
    return {statusCode, ts2HisMDGroup};
  }

  if (ts2HisMDGroup->size() < num) {
    const auto statusMsg = fmt::format(
        "Load his market data before ts failed because "
        "rec num of result {} less than {}. topic = {}, ts = {}",
        ts2HisMDGroup->size(), num, topic, ts);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_RECORDS_LESS_THAN_NUM_OF_QURIES, ts2HisMDGroup};
  }

  LOG_D("Load his market data before ts success. topic = {}, ts = {}, num = {}",
        topic, ts, num);
  return {0, ts2HisMDGroup};
}

std::tuple<int, Ts2HisMDGroupSPtr> MDHis::LoadHisMDAfterTs(
    const std::string& storageRootPath, const std::string& topic,
    std::uint64_t ts, std::uint32_t num, IndexType indexType,
    std::uint32_t maxNumOfHisMDCanBeQeuryEachTime) {
  using namespace boost::gregorian;

  if (num > maxNumOfHisMDCanBeQeuryEachTime) {
    const auto statusMsg = fmt::format(
        "Load his market data after ts failed because "
        "rec num of result {} greater than the query limit {}. topic = {}",
        num, maxNumOfHisMDCanBeQeuryEachTime, topic);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_NUM_OF_RECORDS_GREATER_THAN_LIMIT,
            std::make_shared<Ts2HisMDGroup>()};
  }

  const auto dateBegin = GetDateFromTs(ts / 1000000);
  if (dateBegin < from_undelimited_string(MIN_DATE_OF_HIS_MD)) {
    const auto statusMsg = fmt::format(
        "Load his market data after ts failed because "
        "ts {} less than {}. topic = {}",
        ts, MIN_DATE_OF_HIS_MD, topic);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_INVALID_TS, std::make_shared<Ts2HisMDGroup>()};
  }

  const auto dateEnd =
      day_clock::universal_day() + date_duration(MAX_DATE_OFFSET);
  if (dateBegin > dateEnd) {
    const auto statusMsg = fmt::format(
        "Load his market data after ts failed because "
        "ts {} greater than the day after tomorrow. topic = {}",
        ts, topic);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_INVALID_TS, std::make_shared<Ts2HisMDGroup>()};
  }

  LOG_D("Begin to load {} numbers of index from his market data after {}.", num,
        to_iso_string(dateBegin));

  const auto pathPrefix = GetPathPrefixOfHisMD(storageRootPath, topic);

  auto ts2IndexWithFilenameGroup =
      std::make_shared<Ts2IndexWithFilenameGroup>();
  for (day_iterator iter(dateBegin); iter <= dateEnd; ++iter) {
    const auto date = to_iso_string(*iter);

    const auto idxFilename =
        fmt::format("{}.{}.{}", date, HIS_MD_FILE_EXT,
                    indexType == IndexType::ByExchTs ? HIS_MD_INDEX_BY_ET_EXT
                                                     : HIS_MD_INDEX_BY_LT_EXT);
    const auto pathOfIdx = pathPrefix / idxFilename;

    CreateIdxFileIfNotExists(pathOfIdx.string(), indexType);
    auto [statusCodeOfLoad, ts2IndexWithFilenameGroupOfCurDate] =
        LoadIndexGroupFromFile(pathOfIdx.string(), indexType);
    if (statusCodeOfLoad != 0) {
      continue;
    }

    ts2IndexWithFilenameGroup->merge(*ts2IndexWithFilenameGroupOfCurDate);
    if (*iter == dateBegin) {
      auto iterLowerBound = ts2IndexWithFilenameGroup->lower_bound(ts);
      ts2IndexWithFilenameGroup->erase(std::begin(*ts2IndexWithFilenameGroup),
                                       iterLowerBound);
    }
    if (ts2IndexWithFilenameGroup->size() >= num) {
      ts2IndexWithFilenameGroup->erase(
          std::next(std::begin(*ts2IndexWithFilenameGroup), num),
          std::end(*ts2IndexWithFilenameGroup));
      break;
    }
  }

  const auto [statusCode, ts2HisMDGroup] =
      LoadTs2HisMDGroup(ts2IndexWithFilenameGroup);
  if (statusCode != 0) {
    return {statusCode, ts2HisMDGroup};
  }

  if (ts2HisMDGroup->size() < num) {
    const auto statusMsg = fmt::format(
        "Load his market data before ts failed because "
        "rec num of result {} less than {}. topic = {}, ts = {}",
        ts2HisMDGroup->size(), num, topic, ts);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_RECORDS_LESS_THAN_NUM_OF_QURIES, ts2HisMDGroup};
  }

  LOG_D(
      "Load his market data after ts success. "
      "topic = {}, ts = {}, num = {}",
      topic, ts, num);
  return {0, ts2HisMDGroup};
}

int MDHis::CreateIdxFileIfNotExists(const std::string& filenameOfIdx,
                                    IndexType indexType) {
  const auto filenameOfMD =
      GetMDFilenameByIdxFilename(filenameOfIdx, indexType);

  bool mustCreateIdxFile = false;
  std::time_t lastWriteTimeOfMDFile;
  std::time_t lastWriteTimeOfIDXFile;
  if (boost::filesystem::exists(filenameOfIdx)) {
    try {
      lastWriteTimeOfMDFile = boost::filesystem::last_write_time(filenameOfMD);
      lastWriteTimeOfIDXFile =
          boost::filesystem::last_write_time(filenameOfIdx);
    } catch (const std::exception& e) {
      LOG_W(
          "Create idx file failed because of "
          "get last write time of file {} failed. [{}]",
          filenameOfIdx, e.what());
      return -1;
    }
    if (lastWriteTimeOfMDFile > lastWriteTimeOfIDXFile) {
      mustCreateIdxFile = true;
    }
  } else {
    mustCreateIdxFile = true;
  }

  if (mustCreateIdxFile) {
    int statusCode = 0;
    std::vector<IndexSPtr> indexGroup;
    std::tie(statusCode, indexGroup) = MakeIndexGroup(filenameOfMD, indexType);
    if (statusCode != 0) {
      return statusCode;
    }

    statusCode = SaveIndexGroupToFile(filenameOfIdx, indexGroup);
    if (statusCode != 0) {
      return statusCode;
    }
  }

  return 0;
}

std::tuple<int, Ts2HisMDGroupSPtr> MDHis::LoadTs2HisMDGroup(
    const Ts2IndexWithFilenameGroupSPtr& ts2IndexWithFilenameGroup) {
  auto ts2HisMDGroup = std::make_shared<Ts2HisMDGroup>();

  std::map<std::string, std::shared_ptr<std::ifstream>> filename2In;
  for (const auto& rec : *ts2IndexWithFilenameGroup) {
    const auto& idx = rec.second;

    std::shared_ptr<std::ifstream> in;
    const auto iter = filename2In.find(idx->filename_);
    if (iter != std::end(filename2In)) {
      in = iter->second;
    } else {
      in = std::make_shared<std::ifstream>(idx->filename_.c_str(),
                                           std::ios::binary);
    }

    std::string line(idx->lineLen_, '\0');
    in->seekg(idx->offset_);
    in->read(&line[0], idx->lineLen_);
    in->close();

    ts2HisMDGroup->emplace(idx->ts_, line);
  }

  return {0, ts2HisMDGroup};
}

std::string MDHis::ToJson(int statusCode,
                          const Ts2HisMDGroupSPtr& ts2HisMDGroup) {
  std::string ret;
  ret.reserve(ts2HisMDGroup->size() * 512);

  ret = fmt::format(R"({{"statusCode":{},"statusMsg":"{}",)", statusCode,
                    GetStatusMsg(statusCode));
  ret = ret + R"("hisMDGroup":[)";
  for (const auto& ts2HisMd : *ts2HisMDGroup) {
    const auto& hisMD = ts2HisMd.second;
    ret.append(hisMD);
    ret.append(",");
  }
  if (!ts2HisMDGroup->empty()) ret.pop_back();
  ret += "]}";
  return ret;
}

boost::filesystem::path MDHis::GetPathPrefixOfHisMD(const std::string& rootPath,
                                                    const std::string& topic) {
  boost::filesystem::path ret = rootPath;
  std::vector<std::string> topicFieldGroup;
  boost::split(topicFieldGroup, topic, boost::is_any_of(SEP_OF_TOPIC));
  for (const auto& topicField : topicFieldGroup) {
    ret /= topicField;
  }
  return ret;
}

std::tuple<int, std::vector<IndexSPtr>> MDHis::MakeIndexGroup(
    const std::string& filename, IndexType indexType) {
  std::vector<IndexSPtr> indexGroup;
  std::ifstream in(filename.c_str());
  if (!in.is_open()) {
    LOG_D("Make index group failed because of open file {} failed.", filename);
    return {SCODE_HIS_MD_MAKE_INDEX_GROUP_FAILED, indexGroup};
  }

  std::string line;
  std::uint64_t offset = 0;
  while (std::getline(in, line)) {
    if (line.empty()) continue;

    const auto [statusCode, ts] = GetTsFromLine(line, indexType);
    if (statusCode != 0) {
      LOG_W("Make index group of file {} failed.", filename);
      return {statusCode, indexGroup};
    }

    auto index = std::make_shared<Index>();
    index->ts_ = ts;
    index->offset_ = offset;
    index->lineLen_ = line.size();
    indexGroup.emplace_back(index);

    offset += (line.size() + 1);
  }

  return {0, indexGroup};
}

std::tuple<int, std::uint64_t> MDHis::GetTsFromLine(const std::string& line,
                                                    IndexType indexType) {
  Doc doc;
  if (doc.Parse(line.data()).HasParseError()) {
    LOG_W("Get ts from line failed. line = {}", line);
    return {SCODE_HIS_MD_GET_EXCH_TS_FAILED, 0};
  }

  if (!doc.HasMember("mdHeader") || !doc["mdHeader"].IsObject()) {
    return {-1, 0};
  }

  const auto indexFieldName =
      indexType == IndexType::ByExchTs ? "exchTs" : "localTs";

  if (!doc["mdHeader"].HasMember(indexFieldName) ||
      !doc["mdHeader"][indexFieldName].IsUint64()) {
    return {SCODE_HIS_MD_GET_EXCH_TS_FAILED, 0};
  }

  const auto ts = doc["mdHeader"][indexFieldName].GetUint64();
  return {0, ts};
}

int MDHis::SaveIndexGroupToFile(const std::string& filename,
                                const std::vector<IndexSPtr>& indexGroup) {
  ClearFilecont(filename);

  std::ofstream out(filename.c_str(), std::ios::binary | std::ios::app);
  if (!out.is_open()) {
    LOG_W("Save index group failed because of open file {} failed. ", filename);
    return SCODE_HIS_MD_SAVE_INDEX_GROUP_FAILED;
  }
  for (const auto& index : indexGroup) {
    out.write(reinterpret_cast<char*>(index.get()), sizeof(Index));
  }
  out.close();
  return 0;
}

std::tuple<int, Ts2IndexWithFilenameGroupSPtr> MDHis::LoadIndexGroupFromFile(
    const std::string& filename, IndexType indexType) {
  auto ts2IndexGroup = std::make_shared<Ts2IndexWithFilenameGroup>();

  std::ifstream in(filename.c_str(), std::ios::binary | std::ios::ate);
  if (!in.is_open()) {
    LOG_D("Load index group failed because of open file {} failed. ", filename);
    return {SCODE_HIS_MD_LOAD_INDEX_GROUP_FAILED, ts2IndexGroup};
  }

  const auto pos = in.tellg();
  if (pos % sizeof(Index) != 0) {
    LOG_W(
        "Load index group failed because of "
        "the file size is not an integer multiple of Index.",
        filename);
    return {SCODE_HIS_MD_LOAD_INDEX_GROUP_FAILED, ts2IndexGroup};
  }
  ts2IndexGroup->reserve(pos / sizeof(Index));

  auto buffer = static_cast<char*>(calloc(1, pos));
  in.seekg(0);
  in.read(buffer, pos);
  in.close();

  Index* index = nullptr;
  for (int i = 0; i < pos; i += sizeof(Index)) {
    index = reinterpret_cast<Index*>(buffer + i);
    auto indexWithFilename = std::make_shared<IndexWithFilename>();
    indexWithFilename->ts_ = index->ts_;
    indexWithFilename->offset_ = index->offset_;
    indexWithFilename->lineLen_ = index->lineLen_;
    indexWithFilename->filename_ =
        GetMDFilenameByIdxFilename(filename, indexType);
    ts2IndexGroup->emplace(index->ts_, indexWithFilename);
  }
  if (index) {
    LOG_D("Load index group from file success. size of target file is {}.",
          index->offset_ + index->lineLen_ + 1);  // len of 1 is \n
  }

  SAFE_FREE(buffer);
  return {0, ts2IndexGroup};
}

}  // namespace bq::md
