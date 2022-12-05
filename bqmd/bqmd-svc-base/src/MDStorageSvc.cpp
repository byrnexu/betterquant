/*!
 * \file MDStorageSvc.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/16
 *
 * \brief
 */

#include "MDStorageSvc.hpp"

#include "Config.hpp"
#include "ConnMetadata.hpp"
#include "MDSvc.hpp"
#include "MDSvcConst.hpp"
#include "WSCli.hpp"
#include "WSCliOfExch.hpp"
#include "WSTask.hpp"
#include "def/BQConst.hpp"
#include "def/BQDef.hpp"
#include "def/MDWSCliAsyncTaskArg.hpp"
#include "util/BQMDUtil.hpp"
#include "util/Datetime.hpp"
#include "util/File.hpp"
#include "util/FlowCtrlSvc.hpp"
#include "util/Literal.hpp"
#include "util/MarketDataCond.hpp"
#include "util/String.hpp"
#include "util/TaskDispatcher.hpp"

namespace bq::md::svc {

MDStorageSvc::MDStorageSvc(MDSvc const* mdSvc)
    : mdSvc_(mdSvc), filename2MDGroup_(std::make_shared<Filename2MDGroup>()) {}

int MDStorageSvc::init() {
  const auto mdStorageSvcParamInStrFmt =
      SetParam(DEFAULT_TASK_DISPATCHER_PARAM,
               CONFIG["mdStorageSvcParam"].as<std::string>());
  const auto [ret, mdStorageSvcParam] =
      MakeTaskDispatcherParam(mdStorageSvcParamInStrFmt);
  if (ret != 0) {
    LOG_E("Init failed. {}", mdStorageSvcParamInStrFmt);
    return ret;
  }

  taskDispatcher_ = std::make_shared<TaskDispatcher<web::TaskFromSrvSPtr>>(
      mdStorageSvcParam, nullptr,
      [](auto& asyncTask, auto taskSpecificThreadPoolSize) {
        return RAND_THREAD;
      },
      [this](auto& asyncTask) { handleAsyncTask(asyncTask); });

  taskDispatcher_->init();
  return ret;
}

void MDStorageSvc::start() { taskDispatcher_->start(); }
void MDStorageSvc::stop() { taskDispatcher_->stop(); }

void MDStorageSvc::handle(WSCliAsyncTaskSPtr& asyncTask) {
  taskDispatcher_->dispatch(asyncTask);
}

void MDStorageSvc::handleAsyncTask(WSCliAsyncTaskSPtr& asyncTask) {
  cacheFilename2MDGroup(asyncTask);
  flushMDInCacheToDisk();
}

void MDStorageSvc::cacheFilename2MDGroup(WSCliAsyncTaskSPtr& asyncTask) {
  const auto arg = std::any_cast<WSCliAsyncTaskArgSPtr>(asyncTask->arg_);
  const auto [retOfCreateDirOfMDOfUnifiedFmt, storagePathOfMDOfUnifiedFmt] =
      createDirOfMDOfUnifiedFmt(asyncTask);
  if (retOfCreateDirOfMDOfUnifiedFmt != 0) {
    LOG_W("Cache filename2MDGroup failed. {}", arg->marketDataOfUnifiedFmt_);
  } else {
    cacheMDOfUnifiedFmt(asyncTask, storagePathOfMDOfUnifiedFmt);
  }

  const auto [retOfCreateDirOfMDOfOrig, storagePathOfMDOfOrig, exchHour] =
      createDirOfMDOfOrigFmt(asyncTask);
  if (retOfCreateDirOfMDOfOrig != 0) {
    LOG_W("Cache filename2MDGroup failed. {}",
          asyncTask->task_->msg_->get_payload());
    return;
  } else {
    cacheMDOfOrigFmt(asyncTask->task_->msg_->get_payload(),
                     storagePathOfMDOfOrig, exchHour);
  }
}

std::tuple<int, boost::filesystem::path>
MDStorageSvc::createDirOfMDOfUnifiedFmt(WSCliAsyncTaskSPtr& asyncTask) {
  const auto arg = std::any_cast<WSCliAsyncTaskArgSPtr>(asyncTask->arg_);
  const auto storageRootPath = CONFIG["storageRootPath"].as<std::string>();
  boost::filesystem::path storagePath = storageRootPath;
  std::vector<std::string> fieldGroup;
  // MD@Binance@Spot@BTC-USDT@Trades
  boost::algorithm::split(fieldGroup, arg->topic_,
                          boost::is_any_of(SEP_OF_TOPIC));
  if (fieldGroup.size() < 5) {
    const auto statusMsg =
        fmt::format("Create directories for {} failed. ", arg->topic_);
    LOG_W(statusMsg);
    return {-1, ""};
  }

  const auto mdType = fieldGroup[4];
  for (std::size_t i = 0; i < fieldGroup.size(); ++i) {
    if (i == 5 && mdType == magic_enum::enum_name(MDType::Books)) {
      storagePath /= std::to_string(mdSvc_->getBooksDepthLevelOfSave());
    } else {
      storagePath /= fieldGroup[i];
    }
  }

  try {
    if (!boost::filesystem::exists(storagePath)) {
      boost::filesystem::create_directories(storagePath);
    }
  } catch (const std::exception& e) {
    const auto statusMsg = fmt::format("Create directories {} failed. [{}]",
                                       storagePath.string(), e.what());
    LOG_W(statusMsg);
    return {-1, storagePath};
  }

  return {0, storagePath};
}

void MDStorageSvc::cacheMDOfUnifiedFmt(
    WSCliAsyncTaskSPtr& asyncTask, const boost::filesystem::path& storagePath) {
  const auto arg = std::any_cast<WSCliAsyncTaskArgSPtr>(asyncTask->arg_);
  if (!Config::get_const_instance().topicMustSaveToDisk(arg->topic_)) {
    return;
  }

  // topic = MD@Binance@Spot@BTC-USDT@Trades
  const auto [statusCode, marketDataCond] =
      GetMarketDataCondFromTopic(arg->topic_);
  if (statusCode != 0) {
    LOG_W("Cache market data failed because of invalid topic. {}", arg->topic_);
    return;
  }

  const auto exchDate = GetDateInStrFmtFromTs(arg->exchTs_);
  const auto fileNameOfMDOfUnifiedFmt =
      fmt::format("{}.{}", exchDate, HIS_MD_FILE_EXT);

  if (marketDataCond->mdType_ != MDType::Candle) {
    const auto pathOfMD = storagePath / fileNameOfMDOfUnifiedFmt;
    (*filename2MDGroup_)[pathOfMD.string()].emplace_back(
        arg->marketDataOfUnifiedFmt_ + "\n");

  } else {
    const auto fileOfCandleDetail = storagePath / fileNameOfMDOfUnifiedFmt;
    (*filename2MDGroup_)[fileOfCandleDetail.string()].emplace_back(
        arg->marketDataOfUnifiedFmt_ + "\n");

    const auto fileOfCandle =
        storagePath.parent_path() / fileNameOfMDOfUnifiedFmt;
    handleLastRecOfCandleInOneMinute(arg, fileOfCandle);
  }
}

void MDStorageSvc::handleLastRecOfCandleInOneMinute(
    const WSCliAsyncTaskArgSPtr argInCurCandle,
    const boost::filesystem::path& fileOfCandle) {
  const auto iter = candleTopic2CandleData_.find(argInCurCandle->topic_);
  if (iter == std::end(candleTopic2CandleData_)) {
    candleTopic2CandleData_[argInCurCandle->topic_] = argInCurCandle;
    return;
  }

  auto& argInCache = iter->second;

  const auto minuteInCurCandle = argInCurCandle->exchTs_ / 60000000;
  const auto minuteInCache = argInCache->exchTs_ / 60000000;
  if (minuteInCurCandle <= minuteInCache) {
    argInCache->marketDataOfUnifiedFmt_ =
        argInCurCandle->marketDataOfUnifiedFmt_;
    return;
  }

  for (std::uint64_t i = minuteInCache; i < minuteInCurCandle; ++i) {
    const auto& md = argInCache->marketDataOfUnifiedFmt_;
    const auto curExchTs = fmt::format("{}", i * 60000000);
    const auto d = ReplaceSubStrBetween2Str(md, curExchTs, ET_LTAG, ET_RTAG);
    (*filename2MDGroup_)[fileOfCandle.string()].emplace_back(d + "\n");
  }

  argInCache = argInCurCandle;
}

std::tuple<int, boost::filesystem::path, std::string>
MDStorageSvc::createDirOfMDOfOrigFmt(WSCliAsyncTaskSPtr& asyncTask) {
  const auto arg = std::any_cast<WSCliAsyncTaskArgSPtr>(asyncTask->arg_);
  const auto storageRootPath = CONFIG["storageRootPath"].as<std::string>();
  const auto marketCode = mdSvc_->getMarketCode();
  const auto symbolType = mdSvc_->getSymbolType();
  const auto ptime = ConvertTsToPtime(arg->exchTs_);
  const auto exchDate = ptime.substr(0, 8);
  const auto exchHour = ptime.substr(9, 2);
  boost::filesystem::path storagePath = storageRootPath;
  storagePath /= TOPIC_PREFIX_OF_MARKET_DATA;
  storagePath /= marketCode;
  storagePath /= symbolType;
  storagePath /= exchDate;

  try {
    if (!boost::filesystem::exists(storagePath)) {
      boost::filesystem::create_directories(storagePath);
    }
  } catch (const std::exception& e) {
    const auto statusMsg = fmt::format("Create directories {} failed. [{}]",
                                       storagePath.string(), e.what());
    LOG_W(statusMsg);
    return {-1, storagePath, exchHour};
  }

  return {0, storagePath, exchHour};
}

void MDStorageSvc::cacheMDOfOrigFmt(const std::string& marketDataOfOrigFmt,
                                    const boost::filesystem::path& storagePath,
                                    const std::string& exchHour) {
  auto pathOfMDOfOrigFmt = storagePath;
  const auto fileNameOfMDOfOrigFmt =
      fmt::format("{}.{}", exchHour, HIS_MD_FILE_EXT);
  pathOfMDOfOrigFmt /= fileNameOfMDOfOrigFmt;
  const auto str = fmt::format("{}\n", marketDataOfOrigFmt);
  (*filename2MDGroup_)[pathOfMDOfOrigFmt.string()].emplace_back(str);
}

void MDStorageSvc::flushMDInCacheToDisk() {
  const auto thresholdOfMDRowNumInCache =
      CONFIG["thresholdOfMDRowNumInCache"].as<std::uint32_t>();

  auto filename2MDGroup = std::make_shared<Filename2MDGroup>();
  std::uint32_t totalRowNum{0};
  for (auto& rec : *filename2MDGroup_) {
    totalRowNum += rec.second.size();
  }
  if (totalRowNum < thresholdOfMDRowNumInCache) {
    if (totalRowNum % 1000 == 0) {
      LOG_D("Cache market data for batch write. [num = {}]", totalRowNum);
    }
    return;
  }
  filename2MDGroup.swap(filename2MDGroup_);

  for (auto& rec : *filename2MDGroup) {
    if (rec.second.empty()) continue;
    const auto fileName = rec.first;
    const auto fileCont = boost::join(rec.second, "");
    LOG_D(
        "Flush market data in cache to disk. "
        "[fileName = {}, row num = {}, size = {}kb]",
        fileName, rec.second.size(), fileCont.size() / 1024);
    AppendStrToFile(fileName, fileCont);
  }
}

}  // namespace bq::md::svc
