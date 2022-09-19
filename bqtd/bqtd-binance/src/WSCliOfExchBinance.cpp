#include "WSCliOfExchBinance.hpp"

#include "AssetsMgr.hpp"
#include "Config.hpp"
#include "OrdMgr.hpp"
#include "TDSvc.hpp"
#include "TDSvcDef.hpp"
#include "TDSvcOfBinanceConst.hpp"
#include "TDSvcOfBinanceUtil.hpp"
#include "WSTask.hpp"
#include "db/TBLMonitorOfSymbolInfo.hpp"
#include "def/AssetInfo.hpp"
#include "def/Def.hpp"
#include "def/TDWSCliAsyncTaskArg.hpp"
#include "util/Float.hpp"
#include "util/Json.hpp"
#include "util/String.hpp"
#include "util/TaskDispatcher.hpp"

namespace bq::td::svc::binance {

void WSCliOfExchBinance::onBeforeOpen(
    web::WSCli* wsCli, const web::ConnMetadataSPtr& connMetadata) {}

WSCliAsyncTaskArgSPtr WSCliOfExchBinance::MakeWSCliAsyncTaskArg(
    const web::TaskFromSrvSPtr& task) const {
  const auto& payload = task->msg_->get_payload();
  const auto jsonData = std::make_shared<JsonData>(payload);
  const auto valE = yyjson_obj_get(jsonData->root_, "e");
  if (!valE) {
    LOG_W("Recv {}", payload);
    return nullptr;
  }

  WSCliAsyncTaskArgSPtr ret;
  const auto e = yyjson_get_str(valE);
  if (yyjson_equals_str(valE, "outboundAccountPosition")) {
    ret = std::make_shared<WSCliAsyncTaskArg>(WSMsgType::SyncAssetsUpdate,
                                              jsonData);
  } else if (yyjson_equals_str(valE, "executionReport") ||
             yyjson_equals_str(valE, "ORDER_TRADE_UPDATE")) {
    LOG_D("Recv {}", payload);
    ret = std::make_shared<WSCliAsyncTaskArg>(WSMsgType::Order, jsonData);
  } else {
    LOG_D("Unhandled event. {}", payload);
  }
  return ret;
}

/*
 {
  "e": "outboundAccountPosition",
  "E": 1657063273450,
  "u": 1657063273449,
  "B": [
   {
     "a": "BNB",
     "f": "0.00001061",
     "l": "0.00000000"
   }, {
     "a": "USDT",
     "f": "0.39080000",
     "l": "300.03000000"
   }, {
     "a": "USDC",
     "f": "395.66932800",
     "l": "0.00000000"
   }
  ]
 }
*/
std::vector<AssetInfoSPtr> WSCliOfExchBinance::makeAssetsUpdate(
    WSCliAsyncTaskSPtr& asyncTask) {
  const auto asyncTaskArg =
      std::any_cast<WSCliAsyncTaskArgSPtr>(asyncTask->arg_);
  const auto jsonData = std::any_cast<JsonDataSPtr>(asyncTaskArg->extData_);

  yyjson_val* valExchTs = yyjson_obj_get(jsonData->root_, "E");
  const auto exchTs = yyjson_get_uint(valExchTs);

  std::vector<AssetInfoSPtr> assetInfoGroup;
  const auto valB = yyjson_obj_get(jsonData->root_, "B");
  if (!valB) {
    LOG_W("Recv invalid event of asset chg. {}",
          asyncTask->task_->msg_->get_payload());
    return assetInfoGroup;
  }

  yyjson_val* valRec;
  yyjson_arr_iter iter;
  yyjson_arr_iter_init(valB, &iter);
  while ((valRec = yyjson_arr_iter_next(&iter))) {
    auto assetInfo = std::make_shared<AssetInfo>();
    assetInfo->marketCode_ = tdSvc_->getMarketCodeEnum();
    assetInfo->symbolType_ = tdSvc_->getSymbolTypeEnum();
    assetInfo->acctId_ = tdSvc_->getAcctId();
    assetInfo->updateTime_ = exchTs * 1000;

    yyjson_val *valFieldName, *valFieldValue;
    yyjson_obj_iter iter;
    yyjson_obj_iter_init(valRec, &iter);
    while ((valFieldName = yyjson_obj_iter_next(&iter))) {
      if (yyjson_equals_str(valFieldName, "a")) {
        valFieldValue = yyjson_obj_iter_get_val(valFieldName);
        const auto fieldValue = yyjson_get_str(valFieldValue);
        strncpy(assetInfo->assetName_, fieldValue,
                sizeof(assetInfo->assetName_) - 1);

      } else if (yyjson_equals_str(valFieldName, "f")) {
        valFieldValue = yyjson_obj_iter_get_val(valFieldName);
        const auto fieldValue = yyjson_get_str(valFieldValue);
        assetInfo->available_ = CONV(Decimal, fieldValue);

      } else if (yyjson_equals_str(valFieldName, "l")) {
        valFieldValue = yyjson_obj_iter_get_val(valFieldName);
        const auto fieldValue = yyjson_get_str(valFieldValue);
        assetInfo->frozen_ = CONV(Decimal, fieldValue);
      }
      assetInfo->vol_ = assetInfo->available_ + assetInfo->frozen_;
    }
    assetInfoGroup.emplace_back(assetInfo);
  }

  return assetInfoGroup;
}

OrderInfoSPtr WSCliOfExchBinance::makeOrderInfoFromExch(
    WSCliAsyncTaskSPtr& asyncTask) {
  if (tdSvc_->getSymbolTypeEnum() == SymbolType::Spot) {
    return makeOrderInfoFromExchOfSpot(asyncTask);

  } else if (tdSvc_->getSymbolTypeEnum() == SymbolType::Perp ||
             tdSvc_->getSymbolTypeEnum() == SymbolType::Futures ||
             tdSvc_->getSymbolTypeEnum() == SymbolType::CPerp ||
             tdSvc_->getSymbolTypeEnum() == SymbolType::CFutures) {
    return makeOrderInfoFromExchOfContracts(asyncTask);

  } else {
  }
  return nullptr;
}

/*
{
  "e": "executionReport",
  "E": 1660540068746,
  "s": "BNBUSDT",
  "c": "electron_98eddb640e09453589d774ed8f8",
  "S": "BUY",
  "o": "LIMIT",
  "f": "GTC",
  "q": "0.04000000",
  "p": "311.00000000",
  "P": "0.00000000",
  "F": "0.00000000",
  "g": -1,
  "C": "electron_521895f489c540c99529757ccd8",
  "x": "CANCELED",
  "X": "CANCELED",
  "r": "NONE",
  "i": 4265733670,
  "l": "0.00000000",
  "z": "0.00000000",
  "L": "0.00000000",
  "n": "0",
  "N": null,
  "T": 1660540068745,
  "t": -1,
  "I": 9091895832,
  "w": false,
  "m": false,
  "M": false,
  "O": 1660539946192,
  "Z": "0.00000000",
  "Y": "0.00000000",
  "Q": "0.00000000"
}

{
  "e": "executionReport",
  "E": 1660532705691,
  "s": "ADAUSDT",
  "c": "7041994658805312512",
  "S": "BUY",
  "o": "LIMIT",
  "f": "GTC",
  "q": "20.00000000",
  "p": "0.55000000",
  "P": "0.00000000",
  "F": "0.00000000",
  "g": -1,
  "C": "",
  "x": "NEW",
  "X": "NEW",
  "r": "NONE",
  "i": 3567958093,
  "l": "0.00000000",
  "z": "0.00000000",
  "L": "0.00000000",
  "n": "0",
  "N": null,
  "T": 1660532705690,
  "t": -1,
  "I": 7521672397,
  "w": true,
  "m": false,
  "M": false,
  "O": 1660532705690,
  "Z": "0.00000000",
  "Y": "0.00000000",
  "Q": "0.00000000"
}
*/
OrderInfoSPtr WSCliOfExchBinance::makeOrderInfoFromExchOfSpot(
    WSCliAsyncTaskSPtr& asyncTask) {
  const auto asyncTaskArg =
      std::any_cast<WSCliAsyncTaskArgSPtr>(asyncTask->arg_);
  const auto jsonData = std::any_cast<JsonDataSPtr>(asyncTaskArg->extData_);

  auto orderInfoFromExch = std::make_shared<OrderInfo>();

  yyjson_val* valC = nullptr;
  yyjson_val* valc = nullptr;
  yyjson_val* valX = nullptr;
  yyjson_val* vali = nullptr;
  yyjson_val* vall = nullptr;
  yyjson_val* valr = nullptr;
  yyjson_val* valz = nullptr;
  yyjson_val* valL = nullptr;
  yyjson_val* valn = nullptr;
  yyjson_val* valN = nullptr;
  yyjson_val* valT = nullptr;
  yyjson_val* valt = nullptr;
  yyjson_val* valZ = nullptr;

  yyjson_val *valFieldName, *valFieldValue;
  yyjson_obj_iter iter;
  yyjson_obj_iter_init(jsonData->root_, &iter);
  while ((valFieldName = yyjson_obj_iter_next(&iter))) {
    if (yyjson_equals_str(valFieldName, "X")) {
      valX = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "C")) {
      valC = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "c")) {
      valc = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "i")) {
      vali = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "n")) {
      valn = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "N")) {
      valN = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "z")) {
      valz = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "T")) {
      valT = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "t")) {
      valt = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "L")) {
      valL = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "l")) {
      vall = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "Z")) {
      valZ = yyjson_obj_iter_get_val(valFieldName);
    }
  }

  const auto totalDealAmt = CONV(Decimal, yyjson_get_str(valZ));
  const auto totalDealSize = CONV(Decimal, yyjson_get_str(valz));
  Decimal avgDealPrice = 0;
  if (!isApproximatelyZero(totalDealSize)) {
    avgDealPrice = totalDealAmt / totalDealSize;
  }

  const auto exchOrderStatus = yyjson_get_str(valX);
  if (strcmp(exchOrderStatus, "CANCELED") != 0) {
    orderInfoFromExch->orderId_ = getHashIfIsNotNum(yyjson_get_str(valc));
  } else {
    orderInfoFromExch->orderId_ = getHashIfIsNotNum(yyjson_get_str(valC));
  }

  orderInfoFromExch->exchOrderId_ = yyjson_get_sint(vali);
  if (valn) {
    orderInfoFromExch->fee_ = CONV(Decimal, yyjson_get_str(valn));
  }
  if (valN) {
    if (!yyjson_is_null(valN)) {
      strncpy(orderInfoFromExch->feeCurrency_, yyjson_get_str(valN),
              sizeof(orderInfoFromExch->feeCurrency_) - 1);
    }
  }
  orderInfoFromExch->dealSize_ = CONV(Decimal, yyjson_get_str(valz));
  orderInfoFromExch->avgDealPrice_ = avgDealPrice;

  const auto t = CONV(std::string, yyjson_get_sint(valt));
  strncpy(orderInfoFromExch->lastTradeId_, t.c_str(),
          sizeof(orderInfoFromExch->lastTradeId_) - 1);

  orderInfoFromExch->lastDealPrice_ = CONV(Decimal, yyjson_get_str(valL));
  orderInfoFromExch->lastDealSize_ = CONV(Decimal, yyjson_get_str(vall));
  orderInfoFromExch->lastDealTime_ = yyjson_get_uint(valT) * 1000;

  const auto orderStatus = getOrderStatus(orderInfoFromExch, exchOrderStatus);
  orderInfoFromExch->orderStatus_ = orderStatus;

  return orderInfoFromExch;
}

// ethusdt ask 0.01 price 1150
/*
"{\"e\":\"ORDER_TRADE_UPDATE\",\"T\":1657835497570,\"E\":1657835497583,\"o\":{\"s\":\"ETHUSDT\",\"c\":\"7uvB7NBf\",\"S\":\"SELL\",\"o\":\"LIMIT\",\"f\":\"GTC\",\"q\":\"0.010\",\"p\":\"1150\",\"ap\":\"0\",\"sp\":\"0\",\"x\":\"NEW\",\"X\":\"NEW\",\"i\":8389765530121592356,\"l\":\"0\",\"z\":\"0\",\"L\":\"0\",\"T\":1657835497570,\"t\":0,\"b\":\"0\",\"a\":\"0\",\"m\":false,\"R\":false,\"wt\":\"CONTRACT_PRICE\",\"ot\":\"LIMIT\",\"ps\":\"BOTH\",\"cp\":false,\"rp\":\"0\",\"pP\":false,\"si\":0,\"ss\":0}}"
"{\"e\":\"ORDER_TRADE_UPDATE\",\"T\":1657835685808,\"E\":1657835685816,\"o\":{\"s\":\"ETHUSDT\",\"c\":\"Ps5qsPU1\",\"S\":\"SELL\",\"o\":\"LIMIT\",\"f\":\"GTC\",\"q\":\"0.010\",\"p\":\"1150\",\"ap\":\"0\",\"sp\":\"0\",\"x\":\"NEW\",\"X\":\"NEW\",\"i\":8389765530121809606,\"l\":\"0\",\"z\":\"0\",\"L\":\"0\",\"T\":1657835685808,\"t\":0,\"b\":\"0\",\"a\":\"0\",\"m\":false,\"R\":false,\"wt\":\"CONTRACT_PRICE\",\"ot\":\"LIMIT\",\"ps\":\"BOTH\",\"cp\":false,\"rp\":\"0\",\"pP\":false,\"si\":0,\"ss\":0}}"
"{\"e\":\"ORDER_TRADE_UPDATE\",\"T\":1657836278448,\"E\":1657836278453,\"o\":{\"s\":\"ETHUSDT\",\"c\":\"4y0aB9FQ\",\"S\":\"SELL\",\"o\":\"LIMIT\",\"f\":\"GTC\",\"q\":\"0.010\",\"p\":\"1150\",\"ap\":\"0\",\"sp\":\"0\",\"x\":\"NEW\",\"X\":\"NEW\",\"i\":8389765530122369544,\"l\":\"0\",\"z\":\"0\",\"L\":\"0\",\"T\":1657836278448,\"t\":0,\"b\":\"0\",\"a\":\"0\",\"m\":false,\"R\":false,\"wt\":\"CONTRACT_PRICE\",\"ot\":\"LIMIT\",\"ps\":\"SHORT\",\"cp\":false,\"rp\":\"0\",\"pP\":false,\"si\":0,\"ss\":0}}"

"{\"e\":\"ORDER_TRADE_UPDATE\",\"T\":1657835497570,\"E\":1657835497583,\"o\":{\"s\":\"ETHUSDT\",\"c\":\"7uvB7NBf\",\"S\":\"SELL\",\"o\":\"LIMIT\",\"f\":\"GTC\",\"q\":\"0.010\",\"p\":\"1150\",\"ap\":\"1181.13000\",\"sp\":\"0\",\"x\":\"TRADE\",\"X\":\"FILLED\",\"i\":8389765530121592356,\"l\":\"0.010\",\"z\":\"0.010\",\"L\":\"1181.13\",\"n\":\"0.00472452\",\"N\":\"USDT\",\"T\":1657835497570,\"t\":1895780941,\"b\":\"0\",\"a\":\"0\",\"m\":false,\"R\":false,\"wt\":\"CONTRACT_PRICE\",\"ot\":\"LIMIT\",\"ps\":\"BOTH\",\"cp\":false,\"rp\":\"0\",\"pP\":false,\"si\":0,\"ss\":0}}"
"{\"e\":\"ORDER_TRADE_UPDATE\",\"T\":1657835685808,\"E\":1657835685816,\"o\":{\"s\":\"ETHUSDT\",\"c\":\"Ps5qsPU1\",\"S\":\"SELL\",\"o\":\"LIMIT\",\"f\":\"GTC\",\"q\":\"0.010\",\"p\":\"1150\",\"ap\":\"1182.49000\",\"sp\":\"0\",\"x\":\"TRADE\",\"X\":\"FILLED\",\"i\":8389765530121809606,\"l\":\"0.010\",\"z\":\"0.010\",\"L\":\"1182.49\",\"n\":\"0.00472996\",\"N\":\"USDT\",\"T\":1657835685808,\"t\":1895792659,\"b\":\"0\",\"a\":\"0\",\"m\":false,\"R\":false,\"wt\":\"CONTRACT_PRICE\",\"ot\":\"LIMIT\",\"ps\":\"BOTH\",\"cp\":false,\"rp\":\"0\",\"pP\":false,\"si\":0,\"ss\":0}}"
"{\"e\":\"ORDER_TRADE_UPDATE\",\"T\":1657836278448,\"E\":1657836278453,\"o\":{\"s\":\"ETHUSDT\",\"c\":\"4y0aB9FQ\",\"S\":\"SELL\",\"o\":\"LIMIT\",\"f\":\"GTC\",\"q\":\"0.010\",\"p\":\"1150\",\"ap\":\"1184.72000\",\"sp\":\"0\",\"x\":\"TRADE\",\"X\":\"FILLED\",\"i\":8389765530122369544,\"l\":\"0.010\",\"z\":\"0.010\",\"L\":\"1184.72\",\"n\":\"0.00473888\",\"N\":\"USDT\",\"T\":1657836278448,\"t\":1895815081,\"b\":\"0\",\"a\":\"0\",\"m\":false,\"R\":false,\"wt\":\"CONTRACT_PRICE\",\"ot\":\"LIMIT\",\"ps\":\"SHORT\",\"cp\":false,\"rp\":\"0\",\"pP\":false,\"si\":0,\"ss\":0}}"
 */

// dotusd_perp ask 1 price 6.6
/*
"{\"e\":\"ORDER_TRADE_UPDATE\",\"T\":1657848230656,\"E\":1657848230664,\"i\":\"FzAuAuTiFzuXmYXq\",\"o\":{\"s\":\"DOTUSD_PERP\",\"c\":\"9LhpeSIz\",\"S\":\"SELL\",\"o\":\"LIMIT\",\"f\":\"GTC\",\"q\":\"1\",\"p\":\"6.600\",\"ap\":\"0\",\"sp\":\"0\",\"x\":\"NEW\",\"X\":\"NEW\",\"i\":5593570453,\"l\":\"0\",\"z\":\"0\",\"L\":\"0\",\"T\":1657848230656,\"t\":0,\"b\":\"0\",\"a\":\"0\",\"m\":false,\"R\":false,\"wt\":\"CONTRACT_PRICE\",\"ot\":\"LIMIT\",\"ps\":\"SHORT\",\"cp\":false,\"ma\":\"DOT\",\"rp\":\"0\",\"pP\":false,\"si\":0,\"ss\":0}}"
"{\"e\":\"ORDER_TRADE_UPDATE\",\"T\":1657849024542,\"E\":1657849024550,\"i\":\"FzAuAuTiFzuXmYXq\",\"o\":{\"s\":\"DOTUSD_PERP\",\"c\":\"TIZBdNXi\",\"S\":\"SELL\",\"o\":\"LIMIT\",\"f\":\"GTC\",\"q\":\"1\",\"p\":\"6.600\",\"ap\":\"6.684\",\"sp\":\"0\",\"x\":\"TRADE\",\"X\":\"FILLED\",\"i\":5593597101,\"l\":\"1\",\"z\":\"1\",\"L\":\"6.684\",\"n\":\"0.00074805\",\"N\":\"DOT\",\"T\":1657849024542,\"t\":119274183,\"b\":\"0\",\"a\":\"0\",\"m\":false,\"R\":false,\"wt\":\"CONTRACT_PRICE\",\"ot\":\"LIMIT\",\"ps\":\"SHORT\",\"cp\":false,\"ma\":\"DOT\",\"rp\":\"0\",\"pP\":false,\"si\":0,\"ss\":0}}"
*/
OrderInfoSPtr WSCliOfExchBinance::makeOrderInfoFromExchOfContracts(
    WSCliAsyncTaskSPtr& asyncTask) {
  const auto asyncTaskArg =
      std::any_cast<WSCliAsyncTaskArgSPtr>(asyncTask->arg_);
  const auto jsonData = std::any_cast<JsonDataSPtr>(asyncTaskArg->extData_);

  auto orderInfoFromExch = std::make_shared<OrderInfo>();

  const auto valT = yyjson_obj_get(jsonData->root_, "T");
  orderInfoFromExch->lastDealTime_ = yyjson_get_uint(valT) * 1000;

  yyjson_val* valX = nullptr;
  yyjson_val* valC = nullptr;
  yyjson_val* valc = nullptr;
  yyjson_val* vali = nullptr;
  yyjson_val* valn = nullptr;
  yyjson_val* valN = nullptr;
  yyjson_val* valz = nullptr;
  yyjson_val* valap = nullptr;
  yyjson_val* valt = nullptr;
  yyjson_val* valL = nullptr;
  yyjson_val* vall = nullptr;
  yyjson_val* valS = nullptr;

  const auto valO = yyjson_obj_get(jsonData->root_, "o");

  yyjson_val *valFieldName, *valFieldValue;
  yyjson_obj_iter iter;
  yyjson_obj_iter_init(valO, &iter);
  while ((valFieldName = yyjson_obj_iter_next(&iter))) {
    if (yyjson_equals_str(valFieldName, "X")) {
      valX = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "c")) {
      valc = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "i")) {
      vali = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "n")) {
      valn = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "N")) {
      valN = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "z")) {
      valz = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "ap")) {
      valap = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "t")) {
      valt = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "L")) {
      valL = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "l")) {
      vall = yyjson_obj_iter_get_val(valFieldName);
    } else if (yyjson_equals_str(valFieldName, "S")) {
      valS = yyjson_obj_iter_get_val(valFieldName);
    }
  }

  const auto exchOrderStatus = yyjson_get_str(valX);
  orderInfoFromExch->orderId_ = getHashIfIsNotNum(yyjson_get_str(valc));

  orderInfoFromExch->exchOrderId_ = yyjson_get_uint(vali);
  if (valn) {
    orderInfoFromExch->fee_ = CONV(Decimal, yyjson_get_str(valn));
  }
  if (valN) {
    if (!yyjson_is_null(valN)) {
      strncpy(orderInfoFromExch->feeCurrency_, yyjson_get_str(valN),
              sizeof(orderInfoFromExch->feeCurrency_) - 1);
    }
  }
  orderInfoFromExch->dealSize_ = CONV(Decimal, yyjson_get_str(valz));
  orderInfoFromExch->avgDealPrice_ = CONV(Decimal, yyjson_get_str(valap));

  const auto t = CONV(std::string, yyjson_get_uint(valt));
  strncpy(orderInfoFromExch->lastTradeId_, t.c_str(),
          sizeof(orderInfoFromExch->lastTradeId_) - 1);

  orderInfoFromExch->lastDealPrice_ = CONV(Decimal, yyjson_get_str(valL));
  orderInfoFromExch->lastDealSize_ = CONV(Decimal, yyjson_get_str(vall));

  const auto orderStatus = getOrderStatus(orderInfoFromExch, exchOrderStatus);
  orderInfoFromExch->orderStatus_ = orderStatus;

  if (orderStatus == OrderStatus::Failed) {
    orderInfoFromExch->statusCode_ = -1;
    LOG_W("Order failed. {}", orderInfoFromExch->toShortStr());
  }

  return orderInfoFromExch;
}

}  // namespace bq::td::svc::binance
