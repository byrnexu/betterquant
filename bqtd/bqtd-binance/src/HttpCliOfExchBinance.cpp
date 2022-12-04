/*!
 * \file HttpCliOfExchBinance.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "HttpCliOfExchBinance.hpp"

#include "Config.hpp"
#include "OrdMgr.hpp"
#include "TDSvc.hpp"
#include "TDSvcOfBinanceConst.hpp"
#include "TDSvcOfBinanceUtil.hpp"
#include "TDSvcUtil.hpp"
#include "db/TBLRecSetMaker.hpp"
#include "db/TBLTrdSymbol.hpp"
#include "def/AssetInfo.hpp"
#include "def/StatusCode.hpp"
#include "def/TDWSCliAsyncTaskArg.hpp"
#include "util/Datetime.hpp"
#include "util/ExternalStatusCodeCache.hpp"
#include "util/Float.hpp"
#include "util/TaskDispatcher.hpp"

namespace bq::td::svc::binance {

/*
{
   "symbol": "BTCUSDT",
   "orderId": 28,
   "orderListId": -1,
   "clientOrderId": "6gCrw2kRUAF9CvJDGP16IP",
   "transactTime": 1507725176595
}
*/
int HttpCliOfExchBinance::doOrder(const OrderInfoSPtr& orderInfo) {
  auto query = getQueryStrOfOrder(orderInfo);
  const auto pathnameOfOrder = getPathnameOfOrder(tdSvc_->getSymbolTypeEnum());
  const auto addrOfOrder =
      makeAddrWithSignature(tdSvc_, pathnameOfOrder, query);
  LOG_I("Send order. {}", addrOfOrder);
  const auto apiKey =
      std::any_cast<ApiInfoSPtr>(tdSvc_->getAcctData())->apiKey_;
  auto f = cpr::PostCallback(
      [this, orderInfo](cpr::Response rsp) {
        handleRspOfOrder(orderInfo, rsp);
      },
      cpr::Url{addrOfOrder}, cpr::Header{{"X-MBX-APIKEY", apiKey}});

  return 0;
}

std::tuple<bool, int, std::string> HttpCliOfExchBinance::rspOfOrderIsFailed(
    const std::string& text) {
  std::unique_ptr<yyjson_doc, AutoFreeYYDoc> doc(
      yyjson_read(text.data(), text.size(), 0));
  if (doc.get() == nullptr) {
    const auto statusMsg = fmt::format(
        "Handle rsp of order failed "
        "because of parse http rsp failed. {}",
        (text.empty() ? "empty" : text));
    LOG_W(statusMsg);
    const auto externalStatusCode = SCODE_TD_SVC_PARSE_HTTP_RSP_OF_ORDER_FAILED;
    const auto externalStatusMsg =
        GetStatusMsg(SCODE_TD_SVC_PARSE_HTTP_RSP_OF_ORDER_FAILED);
    return {true, externalStatusCode, externalStatusMsg};
  }

  yyjson_val* root = yyjson_doc_get_root(doc.get());
  const auto valCode = yyjson_obj_get(root, "code");
  const auto valMsg = yyjson_obj_get(root, "msg");
  if (valCode && valMsg) {
    const auto externalStatusCode = yyjson_get_int(valCode);
    const auto externalStatusMsg = yyjson_get_str(valMsg);
    return {true, externalStatusCode, externalStatusMsg};
  } else {
    return {false, 0, ""};
  }
}

int HttpCliOfExchBinance::doCancelOrder(const OrderInfoSPtr& orderInfo) {
  std::string exchSymbolCode = orderInfo->exchSymbolCode_;
  boost::to_upper(exchSymbolCode);
  const auto recvWindow = CONFIG["recvWindow"].as<std::uint32_t>();
  auto query = fmt::format("symbol={}&origClientOrderId={}&recvWindow={}",
                           exchSymbolCode, orderInfo->orderId_, recvWindow);

  const auto addrOfOrder =
      makeAddrWithSignature(tdSvc_, pathnameOfCancelOrder, query);
  LOG_I("Send cancel order. {}", addrOfOrder);
  const auto apiKey =
      std::any_cast<ApiInfoSPtr>(tdSvc_->getAcctData())->apiKey_;
  auto f = cpr::DeleteCallback(
      [this, orderInfo](cpr::Response rsp) {
        handleRspOfCancelOrder(orderInfo, rsp);
      },
      cpr::Url{addrOfOrder}, cpr::Header{{"X-MBX-APIKEY", apiKey}});

  return 0;
}

std::tuple<bool, int, std::string>
HttpCliOfExchBinance::rspOfCancelOrderIsFailed(const std::string& text) {
  std::unique_ptr<yyjson_doc, AutoFreeYYDoc> doc(
      yyjson_read(text.data(), text.size(), 0));
  if (doc.get() == nullptr) {
    const auto statusMsg = fmt::format(
        "Handle rsp of order failed "
        "because of parse http rsp failed. {}",
        (text.empty() ? "empty" : text));
    LOG_W(statusMsg);
    const auto externalStatusCode =
        SCODE_TD_SVC_PARSE_HTTP_RSP_OF_CANCEL_ORDER_FAILED;
    const auto externalStatusMsg =
        GetStatusMsg(SCODE_TD_SVC_PARSE_HTTP_RSP_OF_CANCEL_ORDER_FAILED);
    return {true, externalStatusCode, externalStatusMsg};
  }

  yyjson_val* root = yyjson_doc_get_root(doc.get());
  const auto valCode = yyjson_obj_get(root, "code");
  const auto valMsg = yyjson_obj_get(root, "msg");
  if (valCode && valMsg) {
    const auto externalStatusCode = yyjson_get_int(valCode);
    const auto externalStatusMsg = yyjson_get_str(valMsg);
    return {true, externalStatusCode, externalStatusMsg};
  } else {
    return {false, 0, ""};
  }
}

std::tuple<int, std::string> HttpCliOfExchBinance::doGetListenKey() {
  const auto addrOfHttp = CONFIG["addrOfHttp"].as<std::string>();
  const auto pathnameOfListenKey =
      getPathnameOfListenKey(tdSvc_->getSymbolTypeEnum());
  const auto addrOfListenKey =
      fmt::format("{}{}", addrOfHttp, pathnameOfListenKey);
  const auto timeoutOfGetListenKey =
      CONFIG["timeoutOfGetListenKey"].as<std::uint32_t>();
  const auto apiKey =
      std::any_cast<ApiInfoSPtr>(tdSvc_->getAcctData())->apiKey_;
  cpr::Response rsp = cpr::Post(cpr::Url{addrOfListenKey},
                                cpr::Header{{"X-MBX-APIKEY", apiKey}},
                                cpr::Timeout(timeoutOfGetListenKey));
  if (rsp.status_code != cpr::status::HTTP_OK) {
    const auto statusMsg =
        fmt::format("Get listen key failed. [{}:{}] {} {}", rsp.status_code,
                    rsp.reason, rsp.text, rsp.url.str());
    LOG_W(statusMsg);
    return {-1, ""};
  }
  LOG_D("Get listen key success. [text size = {}] {}", rsp.text.size(),
        rsp.url.str());

  std::unique_ptr<yyjson_doc, AutoFreeYYDoc> doc(
      yyjson_read(rsp.text.data(), rsp.text.size(), 0));
  if (doc.get() == nullptr) {
    LOG_W("Get listen key failed because of parse rsp failed. {}", rsp.text);
    return {-1, ""};
  }

  yyjson_val* root = yyjson_doc_get_root(doc.get());
  if (root == nullptr) {
    LOG_W("Get listen key failed because of parse rsp failed. {}", rsp.text);
    return {-1, ""};
  }

  const auto valListenKey = yyjson_obj_get(root, "listenKey");
  listenKey_ = yyjson_get_str(valListenKey);

  return {0, listenKey_};
}

void HttpCliOfExchBinance::doExtendConnLifecycle() {
  if (listenKey_.empty()) {
    LOG_D("Extend conn lifecycle failed because of listen key is empty.");
    return;
  }

  const auto addrOfHttp = CONFIG["addrOfHttp"].as<std::string>();
  const auto pathnameOfListenKey =
      getPathnameOfListenKey(tdSvc_->getSymbolTypeEnum());
  const auto addrOfListenKey =
      fmt::format("{}{}", addrOfHttp, pathnameOfListenKey);
  const auto apiKey =
      std::any_cast<ApiInfoSPtr>(tdSvc_->getAcctData())->apiKey_;
  auto f = cpr::PutCallback(
      [this](cpr::Response rsp) { handleRspOfExtendConnLifecycle(rsp); },
      cpr::Url{addrOfListenKey}, cpr::Header{{"X-MBX-APIKEY", apiKey}},
      cpr::Payload{{"listenKey", listenKey_}});
}

void HttpCliOfExchBinance::handleRspOfExtendConnLifecycle(cpr::Response rsp) {
  if (rsp.text == "{}") {
    LOG_D("Extend conn lifecycle valid period success. ");
  } else {
    LOG_W("Extend conn lifecycle valid period failed. {} {}", rsp.text,
          rsp.url.str());
  }
}

std::vector<AssetInfoSPtr> HttpCliOfExchBinance::doSyncAssetsSnapshot() {
  const auto jsonData = queryAssetInfoGroup();
  if (!jsonData) {
    LOG_W("Query exch asset info group failed.");
    return std::vector<AssetInfoSPtr>();
  }

  int ret = 0;
  std::vector<AssetInfoSPtr> assetInfoGroupFromExch;

  if (tdSvc_->getSymbolTypeEnum() == SymbolType::Spot) {
    std::tie(ret, assetInfoGroupFromExch) = makeAssetInfoGroupOfSpot(jsonData);

  } else if (tdSvc_->getSymbolTypeEnum() == SymbolType::Perp ||
             tdSvc_->getSymbolTypeEnum() == SymbolType::Futures) {
    std::tie(ret, assetInfoGroupFromExch) =
        makeAssetInfoGroupUBasedContracts(jsonData);

  } else if (tdSvc_->getSymbolTypeEnum() == SymbolType::CPerp ||
             tdSvc_->getSymbolTypeEnum() == SymbolType::CFutures) {
    std::tie(ret, assetInfoGroupFromExch) =
        makeAssetInfoGroupCBasedContracts(jsonData);

  } else {
    LOG_W(
        "Sync exch asset info group failed because of invalid symbol type {}.",
        tdSvc_->getSymbolType());
    return std::vector<AssetInfoSPtr>();
  }

  if (ret != 0) {
    LOG_W("Sync exch asset info group failed.");
    return std::vector<AssetInfoSPtr>();
  }

  return assetInfoGroupFromExch;
}

JsonDataSPtr HttpCliOfExchBinance::queryAssetInfoGroup() {
  const auto recvWindow = CONFIG["recvWindow"].as<std::uint32_t>();
  auto query = fmt::format("recvWindow={}", recvWindow);

  const auto pathnameOfAssetInfo =
      getPathnameOfAssetInfo(tdSvc_->getSymbolTypeEnum());
  const auto addrOfAssetInfoGroup =
      makeAddrWithSignature(tdSvc_, pathnameOfAssetInfo, query);
  const auto apiKey =
      std::any_cast<ApiInfoSPtr>(tdSvc_->getAcctData())->apiKey_;
  const auto timeoutOfQueryAssetInfoGroup =
      CONFIG["timeoutOfQueryAssetInfoGroup"].as<std::uint32_t>();
  cpr::Response rsp = cpr::Get(cpr::Url{addrOfAssetInfoGroup},
                               cpr::Header{{"X-MBX-APIKEY", apiKey}},
                               cpr::Timeout(timeoutOfQueryAssetInfoGroup));
  if (rsp.status_code != cpr::status::HTTP_OK) {
    const auto statusMsg =
        fmt::format("Query exch asset info group failed. [{}:{}] {} {} ",
                    rsp.status_code, rsp.reason, rsp.text, rsp.url.str());
    LOG_W(statusMsg);
    return nullptr;
  }
  LOG_D("Query exch asset info group success. [text size = {}] {} ",
        rsp.text.size(), rsp.url.str());

  const auto doc = yyjson_read(rsp.text.data(), rsp.text.size(), 0);
  if (doc == nullptr) {
    LOG_W("Query exch asset info failed because of parse josn failed. {}",
          rsp.text);
    return nullptr;
  }

  const auto root = yyjson_doc_get_root(doc);
  if (root == nullptr) {
    LOG_W("Query exch asset info failed because of parse josn failed. {}",
          rsp.text);
    return nullptr;
  }

  return std::make_shared<JsonData>(doc, root);
}

/*
{
  "makerCommission": 15,
  "takerCommission": 15,
  "buyerCommission": 0,
  "sellerCommission": 0,
  "canTrade": true,
  "canWithdraw": true,
  "canDeposit": true,
  "updateTime": 123456789,
  "accountType": "SPOT",
  "balances": [
  {
    "asset": "BTC",
    "free": "4723846.89208129",
    "locked": "0.00000000"
  },
  {
    "asset": "LTC",
    "free": "4763368.68006011",
    "locked": "0.00000000"
  }
  ],
  "permissions": [ "SPOT" ]
}
*/
std::tuple<int, std::vector<AssetInfoSPtr>>
HttpCliOfExchBinance::makeAssetInfoGroupOfSpot(const JsonDataSPtr& jsonData) {
  std::vector<AssetInfoSPtr> assetInfoGroup;

  const auto valUpdateTime = yyjson_obj_get(jsonData->root_, "updateTime");
  const std::uint64_t updateTime = yyjson_get_uint(valUpdateTime) * 1000;

  yyjson_val* arr = yyjson_obj_get(jsonData->root_, "balances");
  yyjson_val* valRec;
  yyjson_arr_iter iter;
  yyjson_arr_iter_init(arr, &iter);
  while ((valRec = yyjson_arr_iter_next(&iter))) {
    auto assetInfo = std::make_shared<AssetInfo>();
    yyjson_val *valFieldName, *valFieldValue;
    yyjson_obj_iter iter;
    yyjson_obj_iter_init(valRec, &iter);

    while ((valFieldName = yyjson_obj_iter_next(&iter))) {
      if (yyjson_equals_str(valFieldName, "asset")) {
        valFieldValue = yyjson_obj_iter_get_val(valFieldName);
        strncpy(assetInfo->assetName_, yyjson_get_str(valFieldValue),
                sizeof(assetInfo->assetName_) - 1);

      } else if (yyjson_equals_str(valFieldName, "free")) {
        valFieldValue = yyjson_obj_iter_get_val(valFieldName);
        const auto free = yyjson_get_str(valFieldValue);
        assetInfo->available_ = CONV(Decimal, free);

      } else if (yyjson_equals_str(valFieldName, "locked")) {
        valFieldValue = yyjson_obj_iter_get_val(valFieldName);
        const auto locked = yyjson_get_str(valFieldValue);
        assetInfo->frozen_ = CONV(Decimal, locked);
      }
    }

    assetInfo->vol_ = assetInfo->available_ + assetInfo->frozen_;
    if (!isApproximatelyZero(assetInfo->vol_)) {
      assetInfo->marketCode_ = tdSvc_->getMarketCodeEnum();
      assetInfo->symbolType_ = tdSvc_->getSymbolTypeEnum();
      assetInfo->acctId_ = tdSvc_->getAcctId();
      assetInfo->updateTime_ = updateTime;
      assetInfo->frozen_ = assetInfo->crossVol_ - assetInfo->maxWithdraw_;
      assetInfoGroup.emplace_back(assetInfo);
    }
  }

  return {0, assetInfoGroup};
}

/*
[
{
        "accountAlias": "SgXqFzFzXqoC",
        "asset": "ADA",
        "balance": "0.00000000",
        "crossWalletBalance": "0.00000000",
        "crossUnPnl": "0.00000000",
        "availableBalance": "0.00000000",
        "maxWithdrawAmount": "0.00000000",
        "marginAvailable": true,
        "updateTime": 0
}, {
        "accountAlias": "SgXqFzFzXqoC",
        "asset": "USDT",
        "balance": "314.73286672",
        "crossWalletBalance": "312.94843216",
        "crossUnPnl": "0.00000000",
        "availableBalance": "312.94843216",
        "maxWithdrawAmount": "312.94843216",
        "marginAvailable": true,
        "updateTime": 1657760993798
}, {
        "accountAlias": "SgXqFzFzXqoC",
        "asset": "USDC",
        "balance": "395.66932800",
        "crossWalletBalance": "395.66932800",
        "crossUnPnl": "0.00000000",
        "availableBalance": "395.66932800",
        "maxWithdrawAmount": "395.66932800",
        "marginAvailable": true,
        "updateTime": 1657373124721
}, {
        "accountAlias": "SgXqFzFzXqoC",
        "asset": "BUSD",
        "balance": "1.02076776",
        "crossWalletBalance": "1.02076776",
        "crossUnPnl": "0.00000000",
        "availableBalance": "1.02076776",
        "maxWithdrawAmount": "1.02076776",
        "marginAvailable": true,
        "updateTime": 1628065918243
}
]
*/
std::tuple<int, std::vector<AssetInfoSPtr>>
HttpCliOfExchBinance::makeAssetInfoGroupUBasedContracts(
    const JsonDataSPtr& jsonData) {
  std::vector<AssetInfoSPtr> assetInfoGroup;

  yyjson_val* arr = jsonData->root_;
  yyjson_val* valRec;
  yyjson_arr_iter iter;
  yyjson_arr_iter_init(arr, &iter);
  while ((valRec = yyjson_arr_iter_next(&iter))) {
    auto assetInfo = std::make_shared<AssetInfo>();
    yyjson_val *valFieldName, *valFieldValue;
    yyjson_obj_iter iter;
    yyjson_obj_iter_init(valRec, &iter);
    while ((valFieldName = yyjson_obj_iter_next(&iter))) {
      if (yyjson_equals_str(valFieldName, "asset")) {
        valFieldValue = yyjson_obj_iter_get_val(valFieldName);
        strncpy(assetInfo->assetName_, yyjson_get_str(valFieldValue),
                sizeof(assetInfo->assetName_) - 1);

      } else if (yyjson_equals_str(valFieldName, "balance")) {
        valFieldValue = yyjson_obj_iter_get_val(valFieldName);
        const auto balance = yyjson_get_str(valFieldValue);
        assetInfo->vol_ = CONV(Decimal, balance);

      } else if (yyjson_equals_str(valFieldName, "crossWalletBalance")) {
        valFieldValue = yyjson_obj_iter_get_val(valFieldName);
        const auto crossWalletBalance = yyjson_get_str(valFieldValue);
        assetInfo->crossVol_ = CONV(Decimal, crossWalletBalance);

      } else if (yyjson_equals_str(valFieldName, "crossUnPnl")) {
        valFieldValue = yyjson_obj_iter_get_val(valFieldName);
        const auto crossUnPnl = yyjson_get_str(valFieldValue);
        assetInfo->pnlUnreal_ = CONV(Decimal, crossUnPnl);

      } else if (yyjson_equals_str(valFieldName, "availableBalance")) {
        valFieldValue = yyjson_obj_iter_get_val(valFieldName);
        const auto availableBalance = yyjson_get_str(valFieldValue);
        assetInfo->available_ = CONV(Decimal, availableBalance);

      } else if (yyjson_equals_str(valFieldName, "maxWithdrawAmount")) {
        valFieldValue = yyjson_obj_iter_get_val(valFieldName);
        const auto maxWithdrawAmount = yyjson_get_str(valFieldValue);
        assetInfo->maxWithdraw_ = CONV(Decimal, maxWithdrawAmount);

      } else if (yyjson_equals_str(valFieldName, "updateTime")) {
        valFieldValue = yyjson_obj_iter_get_val(valFieldName);
        assetInfo->updateTime_ = yyjson_get_uint(valFieldValue);
        assetInfo->updateTime_ *= 1000;
      }
    }

    if (!isApproximatelyZero(assetInfo->vol_)) {
      assetInfo->marketCode_ = tdSvc_->getMarketCodeEnum();
      assetInfo->symbolType_ = tdSvc_->getSymbolTypeEnum();
      assetInfo->acctId_ = tdSvc_->getAcctId();
      assetInfoGroup.emplace_back(assetInfo);
    }
  }

  return {0, assetInfoGroup};
}

/*
[{
        "accountAlias": "FzAuAuTiFzuXmYXq",
        "asset": "BNB",
        "balance": "0.00000000",
        "withdrawAvailable": "0.00000000",
        "updateTime": 1657785708757,
        "crossWalletBalance": "0.00000000",
        "crossUnPnl": "0.00000000",
        "availableBalance": "0.00000000"
}, {
        "accountAlias": "FzAuAuTiFzuXmYXq",
        "asset": "DOT",
        "balance": "1.99645034",
        "withdrawAvailable": "1.99645034",
        "updateTime": 1657785708757,
        "crossWalletBalance": "1.99645034",
        "crossUnPnl": "0.00000000",
        "availableBalance": "1.99645034"
}, {
        "accountAlias": "FzAuAuTiFzuXmYXq",
        "asset": "CRV",
        "balance": "61.36640000",
        "withdrawAvailable": "61.36640000",
        "updateTime": 1657785708757,
        "crossWalletBalance": "61.36640000",
        "crossUnPnl": "0.00000000",
        "availableBalance": "61.36640000"
}]
*/
std::tuple<int, std::vector<AssetInfoSPtr>>
HttpCliOfExchBinance::makeAssetInfoGroupCBasedContracts(
    const JsonDataSPtr& jsonData) {
  std::vector<AssetInfoSPtr> assetInfoGroup;

  yyjson_val* arr = jsonData->root_;
  yyjson_val* valRec;
  yyjson_arr_iter iter;
  yyjson_arr_iter_init(arr, &iter);
  while ((valRec = yyjson_arr_iter_next(&iter))) {
    auto assetInfo = std::make_shared<AssetInfo>();
    yyjson_val *valFieldName, *valFieldValue;
    yyjson_obj_iter iter;
    yyjson_obj_iter_init(valRec, &iter);
    while ((valFieldName = yyjson_obj_iter_next(&iter))) {
      if (yyjson_equals_str(valFieldName, "asset")) {
        valFieldValue = yyjson_obj_iter_get_val(valFieldName);
        strncpy(assetInfo->assetName_, yyjson_get_str(valFieldValue),
                sizeof(assetInfo->assetName_) - 1);

      } else if (yyjson_equals_str(valFieldName, "balance")) {
        valFieldValue = yyjson_obj_iter_get_val(valFieldName);
        const auto balance = yyjson_get_str(valFieldValue);
        assetInfo->vol_ = CONV(Decimal, balance);

      } else if (yyjson_equals_str(valFieldName, "withdrawAvailable")) {
        valFieldValue = yyjson_obj_iter_get_val(valFieldName);
        const auto withdrawAvailable = yyjson_get_str(valFieldValue);
        assetInfo->maxWithdraw_ = CONV(Decimal, withdrawAvailable);

      } else if (yyjson_equals_str(valFieldName, "updateTime")) {
        valFieldValue = yyjson_obj_iter_get_val(valFieldName);
        assetInfo->updateTime_ = yyjson_get_uint(valFieldValue);
        assetInfo->updateTime_ *= 1000;

      } else if (yyjson_equals_str(valFieldName, "crossWalletBalance")) {
        valFieldValue = yyjson_obj_iter_get_val(valFieldName);
        const auto crossWalletBalance = yyjson_get_str(valFieldValue);
        assetInfo->crossVol_ = CONV(Decimal, crossWalletBalance);

      } else if (yyjson_equals_str(valFieldName, "crossUnPnl")) {
        valFieldValue = yyjson_obj_iter_get_val(valFieldName);
        const auto crossUnPnl = yyjson_get_str(valFieldValue);
        assetInfo->pnlUnreal_ = CONV(Decimal, crossUnPnl);
      }
    }

    if (!isApproximatelyZero(assetInfo->vol_)) {
      assetInfo->marketCode_ = tdSvc_->getMarketCodeEnum();
      assetInfo->symbolType_ = tdSvc_->getSymbolTypeEnum();
      assetInfo->acctId_ = tdSvc_->getAcctId();
      assetInfo->frozen_ = assetInfo->crossVol_ - assetInfo->maxWithdraw_;
      assetInfoGroup.emplace_back(assetInfo);
    }
  }

  return {0, assetInfoGroup};
}

OrderInfoSPtr HttpCliOfExchBinance::doSyncUnclosedOrderInfo(
    SHMIPCAsyncTaskSPtr& asyncTask) {
  const auto orderInfoInOrdMgr = std::any_cast<OrderInfoSPtr>(asyncTask->arg_);

  const auto jsonData = queryOrderInfo(orderInfoInOrdMgr);
  if (jsonData == nullptr) {
    const auto statusMsg = fmt::format("Sync unclosed order info failed. {} ",
                                       orderInfoInOrdMgr->toShortStr());
    LOG_W(statusMsg);
    return nullptr;
  }

  OrderInfoSPtr orderInfoFromExch;
  if (tdSvc_->getSymbolTypeEnum() == SymbolType::Spot) {
    orderInfoFromExch =
        makeOrderInfoFromExchOfSpot(orderInfoInOrdMgr, jsonData);

  } else if (tdSvc_->getSymbolTypeEnum() == SymbolType::Perp ||
             tdSvc_->getSymbolTypeEnum() == SymbolType::Futures) {
    orderInfoFromExch =
        makeOrderInfoFromExchOfUBasedContracts(orderInfoInOrdMgr, jsonData);

  } else if (tdSvc_->getSymbolTypeEnum() == SymbolType::CPerp ||
             tdSvc_->getSymbolTypeEnum() == SymbolType::CFutures) {
    orderInfoFromExch =
        makeOrderInfoFromExchOfCBasedContracts(orderInfoInOrdMgr, jsonData);

  } else {
    const auto statusMsg = fmt::format(
        "Sync unclosed order info group failed "
        "because of invalid symbol type {}.",
        tdSvc_->getSymbolType());
    LOG_W(statusMsg);
    return nullptr;
  }

  return orderInfoFromExch;
}

JsonDataSPtr HttpCliOfExchBinance::queryOrderInfo(
    const OrderInfoSPtr& orderInfoInOrdMgr) {
  const auto recvWindow = CONFIG["recvWindow"].as<std::uint32_t>();
  std::string exchSymbolCode = orderInfoInOrdMgr->exchSymbolCode_;
  boost::to_upper(exchSymbolCode);
  auto query =
      fmt::format("symbol={}&origClientOrderId={}&recvWindow={}",
                  exchSymbolCode, orderInfoInOrdMgr->orderId_, recvWindow);

  const auto pathnameOfQueryOrderInfo =
      getPathnameOfQueryOrder(tdSvc_->getSymbolTypeEnum());
  const auto addrOfQueryOrderInfo =
      makeAddrWithSignature(tdSvc_, pathnameOfQueryOrderInfo, query);
  const auto apiKey =
      std::any_cast<ApiInfoSPtr>(tdSvc_->getAcctData())->apiKey_;
  const auto timeoutOfQueryOrderInfo =
      CONFIG["timeoutOfQueryOrderInfo"].as<std::uint32_t>();
  cpr::Response rsp = cpr::Get(cpr::Url{addrOfQueryOrderInfo},
                               cpr::Header{{"X-MBX-APIKEY", apiKey}},
                               cpr::Timeout(timeoutOfQueryOrderInfo));
  if (rsp.status_code != cpr::status::HTTP_OK) {
    LOG_W("Query order info failed. [{}:{}] {} {} ", rsp.status_code,
          rsp.reason, rsp.text, rsp.url.str());
  } else {
    LOG_D("Query order info success. [text size = {}] {}", rsp.text.size(),
          rsp.url.str());
  }

  const auto doc = yyjson_read(rsp.text.data(), rsp.text.size(), 0);
  if (doc == nullptr) {
    LOG_W("Query orderinfo of {} failed because of parse josn failed. {}",
          orderInfoInOrdMgr->orderId_, rsp.text);
    return nullptr;
  }

  const auto root = yyjson_doc_get_root(doc);
  if (root == nullptr) {
    LOG_W("Query orderinfo of {} failed because of parse josn failed. {}",
          orderInfoInOrdMgr->orderId_, rsp.text);
    return nullptr;
  }

  return std::make_shared<JsonData>(doc, root);
}

/*
{"code":-2013,"msg":"Order does not exist."}

{
  "symbol": "CRVUSDT",
  "orderId": 827934906,
  "orderListId": -1,
  "clientOrderId": "FTonOs2C",
  "price": "0.72000000",
  "origQty": "15.00000000",
  "executedQty": "15.00000000",
  "cummulativeQuoteQty": "10.59000000",
  "status": "FILLED",
  "timeInForce": "GTC",
  "type": "LIMIT",
  "side": "BUY",
  "stopPrice": "0.00000000",
  "icebergQty": "0.00000000",
  "time": 1655101475416,
  "updateTime": 1655101475416,
  "isWorking": true,
  "origQuoteOrderQty": "0.00000000"
}
*/
OrderInfoSPtr HttpCliOfExchBinance::makeOrderInfoFromExchOfSpot(
    const OrderInfoSPtr& orderInfoInOrdMgr, const JsonDataSPtr& jsonData) {
  auto ret = std::make_shared<OrderInfo>();

  const auto valCode = yyjson_obj_get(jsonData->root_, "code");
  const auto valMsg = yyjson_obj_get(jsonData->root_, "msg");
  if (valCode && valMsg) {
    const auto code = yyjson_get_int(valCode);
    const auto msg = yyjson_get_str(valMsg);
    const auto externalStatusCode = fmt::format("{}", code);
    const auto externalStatusMsg = msg;

    const auto statusCode =
        tdSvc_->getExternalStatusCodeCache()->getAndSetStatusCodeIfNotExists(
            tdSvc_->getMarketCode(), tdSvc_->getSymbolType(),
            externalStatusCode, externalStatusMsg, -1);

    ret->marketCode_ = tdSvc_->getMarketCodeEnum();
    ret->orderId_ = orderInfoInOrdMgr->orderId_;
    ret->exchOrderId_ = orderInfoInOrdMgr->exchOrderId_;

    if (statusCode < 0) {
      ret->dealSize_ = 0;
      ret->avgDealPrice_ = 0;
      ret->orderStatus_ = OrderStatus::Failed;
      ret->statusCode_ = statusCode;
    }
  } else {
    const auto valExecuteQty = yyjson_obj_get(jsonData->root_, "executedQty");
    const auto valCummulativeQuoteQty =
        yyjson_obj_get(jsonData->root_, "cummulativeQuoteQty");

    const auto executedQty = yyjson_get_str(valExecuteQty);
    const auto cummulativeQuoteQty = yyjson_get_str(valCummulativeQuoteQty);

    const auto dealSize = CONV(Decimal, executedQty);
    const auto dealAmt = CONV(Decimal, cummulativeQuoteQty);

    Decimal avgDealPrice = 0;
    if (!isApproximatelyZero(dealSize)) {
      avgDealPrice = dealAmt / dealSize;
    }

    ret->marketCode_ = tdSvc_->getMarketCodeEnum();

    const auto valClientOrderId =
        yyjson_obj_get(jsonData->root_, "clientOrderId");
    const auto clientOrderId = yyjson_get_str(valClientOrderId);
    ret->orderId_ = CONV(std::uint64_t, clientOrderId);

    const auto valOrderId = yyjson_obj_get(jsonData->root_, "orderId");
    ret->exchOrderId_ = yyjson_get_sint(valOrderId);

    ret->dealSize_ = dealSize;
    ret->avgDealPrice_ = avgDealPrice;

    const auto valStatus = yyjson_obj_get(jsonData->root_, "status");
    const auto status = yyjson_get_str(valStatus);
    const auto orderStatus = getOrderStatus(ret, status);
    ret->orderStatus_ = orderStatus;
  }

  return ret;
}

/*
{
  "orderId": 8389765530121592356,
  "symbol": "ETHUSDT",
  "status": "FILLED",
  "clientOrderId": "7uvB7NBf",
  "price": "1150",
  "avgPrice": "1181.13000",
  "origQty": "0.010",
  "executedQty": "0.010",
  "cumQuote": "11.81130",
  "timeInForce": "GTC",
  "type": "LIMIT",
  "reduceOnly": false,
  "closePosition": false,
  "side": "SELL",
  "positionSide": "BOTH",
  "stopPrice": "0",
  "workingType": "CONTRACT_PRICE",
  "priceProtect": false,
  "origType": "LIMIT",
  "time": 1657835497570,
  "updateTime": 1657835497570
}
*/
OrderInfoSPtr HttpCliOfExchBinance::makeOrderInfoFromExchOfUBasedContracts(
    const OrderInfoSPtr& orderInfoInOrdMgr, const JsonDataSPtr& jsonData) {
  auto ret = std::make_shared<OrderInfo>();
  const auto valCode = yyjson_obj_get(jsonData->root_, "code");
  const auto valMsg = yyjson_obj_get(jsonData->root_, "msg");

  if (valCode && valMsg) {
    const auto code = yyjson_get_int(valCode);
    const auto msg = yyjson_get_str(valMsg);
    const auto externalStatusCode = fmt::format("{}", code);
    const auto externalStatusMsg = msg;

    const auto statusCode =
        tdSvc_->getExternalStatusCodeCache()->getAndSetStatusCodeIfNotExists(
            tdSvc_->getMarketCode(), tdSvc_->getSymbolType(),
            externalStatusCode, externalStatusMsg, -1);

    ret->marketCode_ = tdSvc_->getMarketCodeEnum();
    ret->orderId_ = orderInfoInOrdMgr->orderId_;
    ret->exchOrderId_ = orderInfoInOrdMgr->exchOrderId_;

    if (statusCode < 0) {
      ret->dealSize_ = 0;
      ret->avgDealPrice_ = 0;
      ret->orderStatus_ = OrderStatus::Failed;
    }
  } else {
    ret->marketCode_ = tdSvc_->getMarketCodeEnum();

    const auto valClientOrderId =
        yyjson_obj_get(jsonData->root_, "clientOrderId");
    const auto clientOrderId = yyjson_get_str(valClientOrderId);
    ret->orderId_ = CONV(std::uint64_t, clientOrderId);

    const auto valOrderId = yyjson_obj_get(jsonData->root_, "orderId");
    ret->exchOrderId_ = yyjson_get_sint(valOrderId);

    const auto valExecuteQty = yyjson_obj_get(jsonData->root_, "executedQty");
    const auto valAvgPrice = yyjson_obj_get(jsonData->root_, "avgPrice");
    const auto executedQty = yyjson_get_str(valExecuteQty);
    const auto avgPrice = yyjson_get_str(valAvgPrice);
    ret->dealSize_ = CONV(Decimal, executedQty);
    ret->avgDealPrice_ = CONV(Decimal, avgPrice);

    const auto valStatus = yyjson_obj_get(jsonData->root_, "status");
    const auto status = yyjson_get_str(valStatus);
    const auto orderStatus = getOrderStatus(ret, status);
    ret->orderStatus_ = orderStatus;
  }

  return ret;
}

/*
{
    "avgPrice": "0.0",                  // 平均成交价
    "clientOrderId": "abc",             // 用户自定义的订单号
    "cumBase": "0",                     // 成交金额(标的数量)
    "executedQty": "0",                 // 成交量(张数)
    "orderId": 1573346959,              // 系统订单号
    "origQty": "0.40",                  // 原始委托数量
    "origType": "TRAILING_STOP_MARKET", // 触发前订单类型
    "price": "0",                       // 委托价格
    "reduceOnly": false,                // 是否仅减仓
    "side": "BUY",                      // 买卖方向
    "positionSide": "SHORT",            // 持仓方向
    "status": "NEW",                    // 订单状态
    "stopPrice": "9300",                // 触发价
    "closePosition": false,             // 是否条件全平仓
    "symbol": "BTCUSD_200925",          // 交易对
    "pair": "BTCUSD",                   // 标的交易对
    "time": 1579276756075,              // 订单时间
    "timeInForce": "GTC",               // 有效方法
    "type": "TRAILING_STOP_MARKET",     // 订单类型
    "activatePrice": "9020",            // 跟踪止损激活价格
    "priceRate": "0.3",                 // 跟踪止损回调比例
    "updateTime": 1579276756075,        // 更新时间
    "workingType": "CONTRACT_PRICE",    // 条件价格触发类型
    "priceProtect": false               // 是否开启条件单触发保护
}
*/
OrderInfoSPtr HttpCliOfExchBinance::makeOrderInfoFromExchOfCBasedContracts(
    const OrderInfoSPtr& orderInfoInOrdMgr, const JsonDataSPtr& jsonData) {
  auto ret = std::make_shared<OrderInfo>();

  const auto valCode = yyjson_obj_get(jsonData->root_, "code");
  const auto valMsg = yyjson_obj_get(jsonData->root_, "msg");
  if (valCode && valMsg) {
    const auto code = yyjson_get_int(valCode);
    const auto msg = yyjson_get_str(valMsg);
    const auto externalStatusCode = fmt::format("{}", code);
    const auto externalStatusMsg = msg;

    const auto statusCode =
        tdSvc_->getExternalStatusCodeCache()->getAndSetStatusCodeIfNotExists(
            tdSvc_->getMarketCode(), tdSvc_->getSymbolType(),
            externalStatusCode, externalStatusMsg, -1);

    ret->marketCode_ = tdSvc_->getMarketCodeEnum();
    ret->orderId_ = orderInfoInOrdMgr->orderId_;
    ret->exchOrderId_ = orderInfoInOrdMgr->exchOrderId_;

    if (statusCode < 0) {
      ret->dealSize_ = 0;
      ret->avgDealPrice_ = 0;
      ret->orderStatus_ = OrderStatus::Failed;
      ret->statusCode_ = statusCode;
    }
  } else {
    ret->marketCode_ = tdSvc_->getMarketCodeEnum();

    const auto valClientOrderId =
        yyjson_obj_get(jsonData->root_, "clientOrderId");
    const auto clientOrderId = yyjson_get_str(valClientOrderId);
    ret->orderId_ = CONV(std::uint64_t, clientOrderId);

    const auto valOrderId = yyjson_obj_get(jsonData->root_, "orderId");
    ret->exchOrderId_ = yyjson_get_sint(valOrderId);

    const auto valExecuteQty = yyjson_obj_get(jsonData->root_, "executedQty");
    const auto valAvgPrice = yyjson_obj_get(jsonData->root_, "avgPrice");
    const auto executedQty = yyjson_get_str(valExecuteQty);
    const auto avgPrice = yyjson_get_str(valAvgPrice);
    ret->dealSize_ = CONV(Decimal, executedQty);
    ret->avgDealPrice_ = CONV(Decimal, avgPrice);

    const auto valStatus = yyjson_obj_get(jsonData->root_, "status");
    const auto status = yyjson_get_str(valStatus);
    const auto orderStatus = getOrderStatus(ret, status);
    ret->orderStatus_ = orderStatus;
  }

  return ret;
}

void HttpCliOfExchBinance::doTestOrder() {
  if (tdSvc_->getSymbolTypeEnum() == SymbolType::Spot) {
    doTestOrderOfSpot();

  } else if (tdSvc_->getSymbolTypeEnum() == SymbolType::Perp ||
             tdSvc_->getSymbolTypeEnum() == SymbolType::Futures) {
    doTestOrderOfUBasedContracts();

  } else if (tdSvc_->getSymbolTypeEnum() == SymbolType::CPerp ||
             tdSvc_->getSymbolTypeEnum() == SymbolType::CFutures) {
    doTestOrderOfCBasedContracts();

  } else {
  }
}

void HttpCliOfExchBinance::doTestOrderOfSpot() {
  auto orderInfo = std::make_shared<OrderInfo>();
  orderInfo->marketCode_ = tdSvc_->getMarketCodeEnum();
  orderInfo->symbolType_ = SymbolType::Spot;
  strncpy(orderInfo->exchSymbolCode_, "bnbusdt",
          sizeof(orderInfo->exchSymbolCode_) - 1);
  orderInfo->side_ = Side::Ask;
  orderInfo->orderSize_ = 0.04;
  orderInfo->orderPrice_ = 300;
  orderInfo->orderId_ = GET_RAND_INT();

  auto query = getQueryStrOfOrder(orderInfo);
  const auto pathnameOfOrder = getPathnameOfOrder(tdSvc_->getSymbolTypeEnum());
  const auto addrOfOrder =
      makeAddrWithSignature(tdSvc_, pathnameOfOrder, query);
  const auto apiKey =
      std::any_cast<ApiInfoSPtr>(tdSvc_->getAcctData())->apiKey_;
  auto f = cpr::PostCallback(
      [this, orderInfo](cpr::Response rsp) {
        handleRspOfOrder(orderInfo, rsp);
      },
      cpr::Url{addrOfOrder}, cpr::Header{{"X-MBX-APIKEY", apiKey}});
}

void HttpCliOfExchBinance::doTestOrderOfUBasedContracts() {
  auto orderInfo = std::make_shared<OrderInfo>();
  orderInfo->marketCode_ = tdSvc_->getMarketCodeEnum();
  orderInfo->symbolType_ = SymbolType::Perp;
  strncpy(orderInfo->exchSymbolCode_, "ethusdt",
          sizeof(orderInfo->exchSymbolCode_) - 1);
  orderInfo->side_ = Side::Bid;
  orderInfo->posSide_ = PosSide::Long;
  orderInfo->orderSize_ = 0.005;
  orderInfo->orderPrice_ = 1950;
  orderInfo->orderId_ = GET_RAND_INT();

  auto query = getQueryStrOfOrder(orderInfo);
  const auto pathnameOfOrder = getPathnameOfOrder(tdSvc_->getSymbolTypeEnum());
  const auto addrOfOrder =
      makeAddrWithSignature(tdSvc_, pathnameOfOrder, query);
  const auto apiKey =
      std::any_cast<ApiInfoSPtr>(tdSvc_->getAcctData())->apiKey_;
  auto f = cpr::PostCallback(
      [this, orderInfo](cpr::Response rsp) {
        handleRspOfOrder(orderInfo, rsp);
      },
      cpr::Url{addrOfOrder}, cpr::Header{{"X-MBX-APIKEY", apiKey}});
}

void HttpCliOfExchBinance::doTestOrderOfCBasedContracts() {
  auto orderInfo = std::make_shared<OrderInfo>();
  orderInfo->marketCode_ = tdSvc_->getMarketCodeEnum();
  orderInfo->symbolType_ = SymbolType::CPerp;
  strncpy(orderInfo->exchSymbolCode_, "dotusd_perp",
          sizeof(orderInfo->exchSymbolCode_) - 1);
  orderInfo->side_ = Side::Bid;
  orderInfo->posSide_ = PosSide::Long;
  orderInfo->orderSize_ = 1;
  orderInfo->orderPrice_ = 8.5;
  orderInfo->parValue_ = 10;
  orderInfo->orderId_ = GET_RAND_INT();

  auto query = getQueryStrOfOrder(orderInfo);
  const auto pathnameOfOrder = getPathnameOfOrder(tdSvc_->getSymbolTypeEnum());
  const auto addrOfOrder =
      makeAddrWithSignature(tdSvc_, pathnameOfOrder, query);
  const auto apiKey =
      std::any_cast<ApiInfoSPtr>(tdSvc_->getAcctData())->apiKey_;
  auto f = cpr::PostCallback(
      [this, orderInfo](cpr::Response rsp) {
        handleRspOfOrder(orderInfo, rsp);
      },
      cpr::Url{addrOfOrder}, cpr::Header{{"X-MBX-APIKEY", apiKey}});
}

void HttpCliOfExchBinance::doTestCancelOrder() {}

}  // namespace bq::td::svc::binance
