#include "HisMD.hpp"

#include "def/Def.hpp"
#include "def/StatusCode.hpp"
#include "util/Datetime.hpp"
#include "util/Logger.hpp"

namespace bq {

boost::filesystem::path HisMD::GetPathOfHisMD(const std::string& rootPath,
                                              const std::string& topic) {
  boost::filesystem::path ret = rootPath;
  std::vector<std::string> topicFieldGroup;
  boost::split(topicFieldGroup, topic, boost::is_any_of(SEP_OF_TOPIC));
  for (const auto& topicField : topicFieldGroup) {
    ret /= topicField;
  }
  return ret;
}

std::tuple<bool, std::uint64_t, std::string> HisMD::GetExchTsAndMDFromLine(
    const std::string& line) {
  std::vector<std::string> fieldGroup;
  boost::split(fieldGroup, line, boost::is_any_of(SEP_OF_HIS_MD_DATA));
  if (fieldGroup.size() < 3) {
    LOG_W("Get exch ts and market data from line failed. line = {}", line);
    return {false, 0, ""};
  }

  const auto t =
      CONV_OPT(std::uint64_t, fieldGroup[2].substr(0, fieldGroup[2].find(",")));
  if (t == boost::none) {
    LOG_W("Get exch ts from line failed. line = {}", line);
    return {false, 0, ""};
  }
  const auto exchTs = t.value();
  return {true, exchTs, line};
}

ExchTs2HisMDGroupSPtr HisMD::LoadHisMDOfDate(const std::string& storageRootPath,
                                             const std::string& topic,
                                             const std::string& date) {
  const auto filenameOfHisMD = fmt::format("{}.{}", date, HIS_MD_FILE_EXT);
  const auto pathOfHisMD =
      GetPathOfHisMD(storageRootPath, topic) / filenameOfHisMD;
  std::ifstream fileOfHisMD(pathOfHisMD.c_str());
  if (!fileOfHisMD.is_open()) {
    return std::make_shared<ExchTs2HisMDGroup>();
  }

  auto exchTs2HisMDGroup = std::make_shared<ExchTs2HisMDGroup>();
  std::string line;
  while (std::getline(fileOfHisMD, line)) {
    if (line.empty()) continue;
    const auto [ret, exchTs, marketData] = GetExchTsAndMDFromLine(line);
    if (!ret) {
      LOG_W("Load his market data for date {} failed. topic = {}", date, topic);
      continue;
    }
    exchTs2HisMDGroup->emplace(exchTs, marketData);
  }

  return exchTs2HisMDGroup;
}

std::tuple<int, ExchTs2HisMDGroupSPtr> HisMD::LoadHisMDBetweenTs(
    const std::string& storageRootPath, const std::string& topic,
    std::uint64_t tsBegin, std::uint64_t tsEnd,
    std::uint32_t maxNumOfHisMDCanBeQeuryEachTime) {
  using namespace boost::gregorian;

  if (tsBegin > tsEnd) {
    const auto statusMsg = fmt::format(
        "Load his market data between 2 ts failed because "
        "tsBegin {} greater than tsEnd {}. topic = {}",
        tsBegin, tsEnd, topic);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_INVALID_TS, std::make_shared<ExchTs2HisMDGroup>()};
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
    return {SCODE_HIS_MD_INVALID_TS, std::make_shared<ExchTs2HisMDGroup>()};
  }

  const auto dateEnd = GetDateFromTs(tsEnd / 1000000);
  if (dateEnd < from_undelimited_string(MIN_DATE_OF_HIS_MD)) {
    const auto statusMsg = fmt::format(
        "Load his market data between 2 ts failed because "
        "tsEnd {} less than {}. topic = {}",
        tsEnd, MIN_DATE_OF_HIS_MD, topic);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_INVALID_TS, std::make_shared<ExchTs2HisMDGroup>()};
  }

  LOG_D("Begin to load his market data between {} and {}.",
        to_iso_string(dateBegin), to_iso_string(dateEnd));
  auto exchTs2HisMDGroup = std::make_shared<ExchTs2HisMDGroup>();
  for (day_iterator iter(dateBegin); iter <= dateEnd; ++iter) {
    const auto date = to_iso_string(*iter);
    auto exchTs2HisMDGroupOfDate =
        LoadHisMDOfDate(storageRootPath, topic, date);
    if (*iter == dateBegin) {
      const auto iterBegin = exchTs2HisMDGroupOfDate->lower_bound(tsBegin);
      exchTs2HisMDGroupOfDate->erase(std::begin(*exchTs2HisMDGroupOfDate),
                                     iterBegin);
    }
    if (*iter == dateEnd) {
      const auto iterEnd = exchTs2HisMDGroupOfDate->lower_bound(tsEnd);
      exchTs2HisMDGroupOfDate->erase(iterEnd,
                                     std::end(*exchTs2HisMDGroupOfDate));
    }
    exchTs2HisMDGroup->merge(*exchTs2HisMDGroupOfDate);
    if (exchTs2HisMDGroup->size() > maxNumOfHisMDCanBeQeuryEachTime) {
      const auto statusMsg = fmt::format(
          "Load his market data between ts failed because "
          "rec num of result greater than the query limit {}. topic = {}",
          maxNumOfHisMDCanBeQeuryEachTime, topic);
      LOG_W(statusMsg);
      return {SCODE_HIS_MD_NUM_OF_RECORDS_GREATER_THAN_LIMIT,
              std::make_shared<ExchTs2HisMDGroup>()};
    }
  }

  LOG_D(
      "Load his market data between 2 ts success. topic = {}, tsBegin = {}, "
      "tsEnd = {}",
      topic, tsBegin, tsEnd);
  return {0, exchTs2HisMDGroup};
}

std::tuple<int, ExchTs2HisMDGroupSPtr> HisMD::LoadHisMDBeforeTs(
    const std::string& storageRootPath, const std::string& topic,
    std::uint64_t ts, std::uint32_t num,
    std::uint32_t maxNumOfHisMDCanBeQeuryEachTime) {
  using namespace boost::gregorian;

  if (num > maxNumOfHisMDCanBeQeuryEachTime) {
    const auto statusMsg = fmt::format(
        "Load his market data before ts failed because "
        "rec num of result {} greater than the query limit {}. topic = {}",
        num, maxNumOfHisMDCanBeQeuryEachTime, topic);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_NUM_OF_RECORDS_GREATER_THAN_LIMIT,
            std::make_shared<ExchTs2HisMDGroup>()};
  }

  const auto dateBegin = GetDateFromTs(ts / 1000000);
  if (dateBegin < from_undelimited_string(MIN_DATE_OF_HIS_MD)) {
    const auto statusMsg = fmt::format(
        "Load his markete data before ts failed because "
        "ts {} less than {}. topic = {}",
        ts, MIN_DATE_OF_HIS_MD, topic);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_INVALID_TS, std::make_shared<ExchTs2HisMDGroup>()};
  }

  if (dateBegin > day_clock::universal_day() + date_duration(MAX_DATE_OFFSET)) {
    const auto statusMsg = fmt::format(
        "Load his market data before ts failed because "
        "ts {} greater than the day after tomorrow. topic = {}",
        ts, topic);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_INVALID_TS, std::make_shared<ExchTs2HisMDGroup>()};
  }

  const auto dateEnd = from_undelimited_string(MIN_DATE_OF_HIS_MD);
  LOG_D("Begin to load {} numbers of rec from his market data before {}.", num,
        to_iso_string(dateBegin));

  auto exchTs2HisMDGroup = std::make_shared<ExchTs2HisMDGroup>();
  for (day_iterator iter(dateBegin); iter >= dateEnd; --iter) {
    const auto date = to_iso_string(*iter);
    const auto exchTs2HisMDGroupOfDate =
        LoadHisMDOfDate(storageRootPath, topic, date);
    exchTs2HisMDGroup->merge(*exchTs2HisMDGroupOfDate);
    if (*iter == dateBegin) {
      auto iterUpperBound = exchTs2HisMDGroup->upper_bound(ts);
      exchTs2HisMDGroup->erase(iterUpperBound, std::end(*exchTs2HisMDGroup));
    }
    if (exchTs2HisMDGroup->size() >= num) {
      const auto offset = exchTs2HisMDGroup->size() - num;
      exchTs2HisMDGroup->erase(
          std::begin(*exchTs2HisMDGroup),
          std::next(std::begin(*exchTs2HisMDGroup), offset));
      break;
    }
  }

  if (exchTs2HisMDGroup->size() < num) {
    const auto statusMsg = fmt::format(
        "Load his market data before ts failed because "
        "rec num of result {} less than {}. topic = {}, ts = {}",
        exchTs2HisMDGroup->size(), num, topic, ts);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_RECORDS_LESS_THAN_NUM_OF_QURIES, exchTs2HisMDGroup};
  }

  LOG_D("Load his market data before ts success. topic = {}, ts = {}, num = {}",
        topic, ts, num);
  return {0, exchTs2HisMDGroup};
}

std::tuple<int, ExchTs2HisMDGroupSPtr> HisMD::LoadHisMDAfterTs(
    const std::string& storageRootPath, const std::string& topic,
    std::uint64_t ts, std::uint32_t num,
    std::uint32_t maxNumOfHisMDCanBeQeuryEachTime) {
  using namespace boost::gregorian;

  if (num > maxNumOfHisMDCanBeQeuryEachTime) {
    const auto statusMsg = fmt::format(
        "Load his market data after ts failed because "
        "rec num of result {} greater than the query limit {}. topic = {}",
        num, maxNumOfHisMDCanBeQeuryEachTime, topic);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_NUM_OF_RECORDS_GREATER_THAN_LIMIT,
            std::make_shared<ExchTs2HisMDGroup>()};
  }

  const auto dateBegin = GetDateFromTs(ts / 1000000);
  if (dateBegin < from_undelimited_string(MIN_DATE_OF_HIS_MD)) {
    const auto statusMsg = fmt::format(
        "Load his market data after ts failed because "
        "ts {} less than {}. topic = {}",
        ts, MIN_DATE_OF_HIS_MD, topic);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_INVALID_TS, std::make_shared<ExchTs2HisMDGroup>()};
  }

  const auto dateEnd =
      day_clock::universal_day() + date_duration(MAX_DATE_OFFSET);
  if (dateBegin > dateEnd) {
    const auto statusMsg = fmt::format(
        "Load his market data after ts failed because "
        "ts {} greater than the day after tomorrow. topic = {}",
        ts, topic);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_INVALID_TS, std::make_shared<ExchTs2HisMDGroup>()};
  }

  LOG_D("Begin to load {} numbers of rec from his market data after {}.", num,
        to_iso_string(dateBegin));
  auto exchTs2HisMDGroup = std::make_shared<ExchTs2HisMDGroup>();
  for (day_iterator iter(dateBegin); iter <= dateEnd; ++iter) {
    const auto date = to_iso_string(*iter);
    const auto exchTs2HisMDGroupOfDate =
        LoadHisMDOfDate(storageRootPath, topic, date);
    exchTs2HisMDGroup->merge(*exchTs2HisMDGroupOfDate);
    if (*iter == dateBegin) {
      auto iterLowerBound = exchTs2HisMDGroup->lower_bound(ts);
      exchTs2HisMDGroup->erase(std::begin(*exchTs2HisMDGroup), iterLowerBound);
    }
    if (exchTs2HisMDGroup->size() >= num) {
      exchTs2HisMDGroup->erase(std::next(std::begin(*exchTs2HisMDGroup), num),
                               std::end(*exchTs2HisMDGroup));
      break;
    }
  }

  if (exchTs2HisMDGroup->size() < num) {
    const auto statusMsg = fmt::format(
        "Load his market data after ts failed because "
        "rec num {} less than {}. topic = {}, ts = {}",
        exchTs2HisMDGroup->size(), num, topic, ts);
    LOG_W(statusMsg);
    return {SCODE_HIS_MD_RECORDS_LESS_THAN_NUM_OF_QURIES, exchTs2HisMDGroup};
  }

  LOG_D(
      "Load his market data after ts success. "
      "topic = {}, ts = {}, num = {}",
      topic, ts, num);
  return {0, exchTs2HisMDGroup};
}

std::string HisMD::ToJson(int statusCode,
                          const ExchTs2HisMDGroupSPtr& exchTs2HisMDGroup) {
  std::string ret;
  ret.reserve(exchTs2HisMDGroup->size() * 512);

  ret = fmt::format(R"({{"statusCode":{},"statusMsg":"{}",)", statusCode,
                    GetStatusMsg(statusCode));
  ret = ret + R"("hisMDGroup":[)";
  for (const auto& exchTs2HisMd : *exchTs2HisMDGroup) {
    const auto& hisMD = exchTs2HisMd.second;
    ret.append(hisMD);
    ret.append(",");
  }
  if (!exchTs2HisMDGroup->empty()) ret.pop_back();
  ret += "]}";
  return ret;
}

}  // namespace bq
