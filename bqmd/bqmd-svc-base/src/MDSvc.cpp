/*!
 * \file MDSvc.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "MDSvc.hpp"

#include "Config.hpp"
#include "MDStorageSvc.hpp"
#include "SHMIPCConst.hpp"
#include "SHMSrv.hpp"
#include "SHMSrvMsgHandler.hpp"
#include "SubAndUnSubSvc.hpp"
#include "SymbolTableMaint.hpp"
#include "TopicGroupMustSubMaint.hpp"
#include "WSCli.hpp"
#include "WSCliOfExch.hpp"
#include "db/DBE.hpp"
#include "db/DBEngConst.hpp"
#include "db/TBLMonitorOfSymbolInfo.hpp"
#include "def/Const.hpp"
#include "util/FlowCtrlSvc.hpp"
#include "util/Literal.hpp"
#include "util/String.hpp"

namespace bq::md::svc {

int MDSvc::prepareInit() {
  auto retOfConfInit = Config::get_mutable_instance().init(configFilename_);
  if (retOfConfInit != 0) {
    const auto statusMsg = fmt::format("Prepare init failed.");
    std::cerr << statusMsg << std::endl;
    return retOfConfInit;
  }

  const auto retOfLoggerInit = InitLogger(CONFIG);
  if (retOfLoggerInit != 0) {
    const auto statusMsg = fmt::format("Prepare init failed.");
    std::cerr << statusMsg << std::endl;
    return retOfLoggerInit;
  }

  return 0;
}

int MDSvc::doInit() {
  if (auto ret = initDBEng(); ret != 0) {
    LOG_E("Do init failed.");
    return ret;
  }

  marketCode_ = CONFIG["marketCode"].as<std::string>();
  symbolType_ = CONFIG["symbolType"].as<std::string>();

  marketCodeEnum_ = GetMarketCode(marketCode_);
  if (marketCodeEnum_ == MarketCode::Others) {
    LOG_W("Do init failed because of invalid marketcode {}.", marketCode_);
    return -1;
  }

  const auto symbolType = magic_enum::enum_cast<SymbolType>(symbolType_);
  if (!symbolType.has_value()) {
    LOG_W("Do init failed because of invalid symboltype {}.", symbolType_);
    return -1;
  }
  symbolTypeEnum_ = symbolType.value();

  initTBLMonitorOfSymbolInfo();
  flowCtrlSvc_ = std::make_shared<FlowCtrlSvc>(CONFIG);

  assert(wsCliOfExch_ != nullptr && "wsCliOfExch_ != nullptr");
  if (const auto ret = wsCliOfExch_->init(); ret != 0) {
    LOG_E("Do init failed.");
    return ret;
  }

  saveMarketData_ = CONFIG["saveMarketData"].as<bool>(false);
  if (saveMarketData()) {
    mdStorageSvc_ = std::make_shared<MDStorageSvc>(this);
    if (auto ret = mdStorageSvc_->init(); ret != 0) {
      LOG_E("Do init failed.");
      return ret;
    }
  }
  booksDepthLevelOfSave_ =
      CONFIG["booksDepthLevelOfSave"].as<std::uint32_t>(MAX_DEPTH_LEVEL);

  topicGroupMustSubMaint_ = std::make_shared<TopicGroupMustSubMaint>(this);

  assert(subAndUnSubSvc_ != nullptr && "subAndUnSubSvc_ != nullptr");
  if (auto ret = subAndUnSubSvc_->init(); ret != 0) {
    LOG_E("Do init failed.");
    return ret;
  }

  shmSrvMsgHandler_ = std::make_shared<SHMSrvMsgHandler>(this);
  addrOfSHMSrv_ = fmt::format(
      "{}-{}-{}{}{}{}{}{}{}", TOPIC_PREFIX_OF_MARKET_DATA, marketCode_,
      symbolType_, SEP_OF_SHM_SVC, TOPIC_PREFIX_OF_MARKET_DATA, SEP_OF_SHM_SVC,
      marketCode_, SEP_OF_SHM_SVC, symbolType_);
  shmSrv_ = std::make_shared<SHMSrv>(
      addrOfSHMSrv_, [this](const auto* shmBuf, std::size_t shmBufLen) {
        shmSrvMsgHandler_->handleReq(shmBuf, shmBufLen);
      });

  return 0;
}

int MDSvc::initDBEng() {
  const auto dbEngParam = SetParam(db::DEFAULT_DB_ENG_PARAM,
                                   CONFIG["dbEngParam"].as<std::string>());
  int retOfMakeDBEng = 0;
  std::tie(retOfMakeDBEng, dbEng_) = db::MakeDBEng(
      dbEngParam, [](db::DBTaskSPtr& dbTask, const StringSPtr& dbExecRet) {
        LOG_D("Exec sql finished. [{}] [exec result = {}]", dbTask->toStr(),
              *dbExecRet);
      });
  if (retOfMakeDBEng != 0) {
    LOG_E("Init dbeng failed. {}", dbEngParam);
    return retOfMakeDBEng;
  }

  if (auto retOfInit = getDBEng()->init(); retOfInit != 0) {
    LOG_E("Init dbeng failed. {}", dbEngParam);
    return retOfInit;
  }

  return 0;
}

void MDSvc::initTBLMonitorOfSymbolInfo() {
  const auto milliSecIntervalOfTBLMonitorOfSymbolInfo =
      CONFIG["milliSecIntervalOfTBLMonitorOfSymbolInfo"].as<std::uint32_t>();
  const auto sql = fmt::format(
      "SELECT * FROM {} WHERE `marketCode` = '{}' AND `symbolType` = '{}'",
      TBLSymbolInfo::TableName, getMarketCode(), getSymbolType());
  tblMonitorOfSymbolInfo_ = std::make_shared<db::TBLMonitorOfSymbolInfo>(
      getDBEng(), milliSecIntervalOfTBLMonitorOfSymbolInfo, sql);
}

int MDSvc::doRun() {
  getDBEng()->start();
  assert(symbolTableMaint_ != nullptr && "symbolTableMaint_ != nullptr");
  if (auto ret = symbolTableMaint_->start(); ret != 0) {
    LOG_E("Run failed.");
    return ret;
  }

  if (saveMarketData()) {
    mdStorageSvc_->start();
  }

  if (auto ret = tblMonitorOfSymbolInfo_->start(); ret != 0) {
    LOG_E("Run failed.");
    return ret;
  }

  shmSrv_->start();

  if (auto ret = wsCliOfExch_->start(); ret != 0) {
    LOG_E("Run failed.");
    return ret;
  }

  const auto addrOfWSPub = CONFIG["addrOfWSPub"].as<std::string>();
  if (const auto [ret, no] = wsCliOfExch_->getWSCli()->connect(addrOfWSPub);
      ret != 0) {
    LOG_E("Run failed.");
    return ret;
  }

  subAndUnSubSvc_->start();

  if (auto ret = topicGroupMustSubMaint_->start(); ret != 0) {
    LOG_E("Run failed.");
    return ret;
  }

  return 0;
}

void MDSvc::doExit(const boost::system::error_code* ec, int signalNum) {
  topicGroupMustSubMaint_->stop();
  subAndUnSubSvc_->stop();
  wsCliOfExch_->stop();
  shmSrv_->stop();
  tblMonitorOfSymbolInfo_->stop();
  if (saveMarketData()) {
    mdStorageSvc_->stop();
  }
  symbolTableMaint_->stop();
  getDBEng()->stop();
}

}  // namespace bq::md::svc
