#include "SymbolTableMaintOfBinance.hpp"

#include "Config.hpp"
#include "MDSvcOfBinance.hpp"
#include "MDSvcOfBinanceConst.hpp"
#include "db/TBLRecSetMaker.hpp"
#include "db/TBLSymbolInfo.hpp"
#include "def/BQConst.hpp"
#include "def/Const.hpp"
#include "util/Datetime.hpp"
#include "util/Json.hpp"
#include "util/String.hpp"

namespace bq::md::svc::binance {

/* spot
 {
  "timezone": "UTC",
  "serverTime": 1565246363776,
  "rateLimits": [{}],
  "exchangeFilters": [],
  "symbols": [{
    "symbol": "ETHBTC",
    "status": "TRADING",
    "baseAsset": "ETH",
    "baseAssetPrecision": 8,
    "quoteAsset": "BTC",
    "quotePrecision": 8,
    "quoteAssetPrecision": 8,
    "orderTypes": [
      "LIMIT",
      "LIMIT_MAKER",
      "MARKET",
      "STOP_LOSS",
      "STOP_LOSS_LIMIT",
      "TAKE_PROFIT",
      "TAKE_PROFIT_LIMIT"
    ],
    "icebergAllowed": true,
    "ocoAllowed": true,
    "quoteOrderQtyMarketAllowed": false,
    "allowTrailingStop": false,
    "isSpotTradingAllowed": true,
    "isMarginTradingAllowed": true,
    "allowTrailingStop": false,
    "filters": [],
    "permissions": [
      "SPOT",
      "MARGIN"
    ]
  }]
 }
 */

/* usdt base futures
 {
  "symbols": [{
    "symbol": "BTCUSDT",
    "pair": "BTCUSDT",
    "contractType": "PERPETUAL",
    "deliveryDate": 4133404800000,
    "onboardDate": 1569398400000,
    "status": "TRADING",
    "maintMarginPercent": "2.5000",
    "requiredMarginPercent": "5.0000",
    "baseAsset": "BTC",
    "quoteAsset": "USDT",
    "marginAsset": "USDT",
    "pricePrecision": 2,
    "quantityPrecision": 3,
    "baseAssetPrecision": 8,
    "quotePrecision": 8,
    "underlyingType": "COIN",
    "underlyingSubType": ["PoW"],
    "settlePlan": 0,
    "triggerProtect": "0.0500",
    "liquidationFee": "0.012500",
    "marketTakeBound": "0.05",
    "filters": [{
      "minPrice": "556.80",
      "maxPrice": "4529764",
      "filterType": "PRICE_FILTER",
      "tickSize": "0.10"
    }, {
      "stepSize": "0.001",
      "filterType": "LOT_SIZE",
      "maxQty": "1000",
      "minQty": "0.001"
    }, {
      "stepSize": "0.001",
      "filterType": "MARKET_LOT_SIZE",
      "maxQty": "120",
      "minQty": "0.001"
    }, {
      "limit": 200,
      "filterType": "MAX_NUM_ORDERS"
    }, {
      "limit": 10,
      "filterType": "MAX_NUM_ALGO_ORDERS"
    }, {
      "notional": "5",
      "filterType": "MIN_NOTIONAL"
    }, {
      "multiplierDown": "0.9500",
      "multiplierUp": "1.0500",
      "multiplierDecimal": "4",
      "filterType": "PERCENT_PRICE"
    }]
  }, {
    "symbol": "BTCUSDT_220624",
    "pair": "BTCUSDT",
    "contractType": "CURRENT_QUARTER",
    "deliveryDate": 1656057600000,
    "onboardDate": 1647849600000,
    "status": "TRADING",
    "maintMarginPercent": "2.5000",
    "requiredMarginPercent": "5.0000",
    "baseAsset": "BTC",
    "quoteAsset": "USDT",
    "marginAsset": "USDT",
    "pricePrecision": 1,
    "quantityPrecision": 3,
    "baseAssetPrecision": 8,
    "quotePrecision": 8,
    "underlyingType": "COIN",
    "underlyingSubType": [],
    "settlePlan": 0,
    "triggerProtect": "0.0500",
    "liquidationFee": "0.010000",
    "marketTakeBound": "0.05",
    "filters": [{
      "minPrice": "576.3",
      "maxPrice": "1000000",
      "filterType": "PRICE_FILTER",
      "tickSize": "0.1"
    }, {
      "stepSize": "0.001",
      "filterType": "LOT_SIZE",
      "maxQty": "500",
      "minQty": "0.001"
    }, {
      "stepSize": "0.001",
      "filterType": "MARKET_LOT_SIZE",
      "maxQty": "3",
      "minQty": "0.001"
    }, {
      "limit": 200,
      "filterType": "MAX_NUM_ORDERS"
    }, {
      "limit": 10,
      "filterType": "MAX_NUM_ALGO_ORDERS"
    }, {
      "notional": "5",
      "filterType": "MIN_NOTIONAL"
    }, {
      "multiplierDown": "0.9500",
      "multiplierUp": "1.0500",
      "multiplierDecimal": "4",
      "filterType": "PERCENT_PRICE"
    }]
 }]
 }
 */

/* coin based futures
 {
  "symbols": [{
    "symbol": "BTCUSD_PERP",
    "pair": "BTCUSD",
    "contractType": "PERPETUAL",
    "deliveryDate": 4133404800000,
    "onboardDate": 1597042800000,
    "contractStatus": "TRADING",
    "contractSize": 100,
    "marginAsset": "BTC",
    "maintMarginPercent": "2.5000",
    "requiredMarginPercent": "5.0000",
    "baseAsset": "BTC",
    "quoteAsset": "USD",
    "pricePrecision": 1,
    "quantityPrecision": 0,
    "baseAssetPrecision": 8,
    "quotePrecision": 8,
    "equalQtyPrecision": 4,
    "maxMoveOrderLimit": 10000,
    "triggerProtect": "0.0500",
    "underlyingType": "COIN",
    "underlyingSubType": ["PoW"],
    "filters": [{
      "minPrice": "1000",
      "maxPrice": "4520958",
      "filterType": "PRICE_FILTER",
      "tickSize": "0.1"
    }, {
      "stepSize": "1",
      "filterType": "LOT_SIZE",
      "maxQty": "1000000",
      "minQty": "1"
    }, {
      "stepSize": "1",
      "filterType": "MARKET_LOT_SIZE",
      "maxQty": "60000",
      "minQty": "1"
    }, {
      "limit": 200,
      "filterType": "MAX_NUM_ORDERS"
    }, {
      "limit": 100,
      "filterType": "MAX_NUM_ALGO_ORDERS"
    }, {
      "multiplierDown": "0.9500",
      "multiplierUp": "1.0500",
      "multiplierDecimal": "4",
      "filterType": "PERCENT_PRICE"
    }],
    "liquidationFee": "0.020000",
    "marketTakeBound": "0.05"
  }, {
    "symbol": "BTCUSD_220624",
    "pair": "BTCUSD",
    "contractType": "CURRENT_QUARTER",
    "deliveryDate": 1656057600000,
    "onboardDate": 1640937600000,
    "contractStatus": "TRADING",
    "contractSize": 100,
    "marginAsset": "BTC",
    "maintMarginPercent": "2.5000",
    "requiredMarginPercent": "5.0000",
    "baseAsset": "BTC",
    "quoteAsset": "USD",
    "pricePrecision": 1,
    "quantityPrecision": 0,
    "baseAssetPrecision": 8,
    "quotePrecision": 8,
    "equalQtyPrecision": 4,
    "maxMoveOrderLimit": 10000,
    "triggerProtect": "0.0500",
    "underlyingType": "COIN",
    "underlyingSubType": [],
    "filters": [{
      "minPrice": "1000",
      "maxPrice": "4671848",
      "filterType": "PRICE_FILTER",
      "tickSize": "0.1"
    }, {
      "stepSize": "1",
      "filterType": "LOT_SIZE",
      "maxQty": "1000000",
      "minQty": "1"
    }, {
      "stepSize": "1",
      "filterType": "MARKET_LOT_SIZE",
      "maxQty": "250",
      "minQty": "1"
    }, {
      "limit": 200,
      "filterType": "MAX_NUM_ORDERS"
    }, {
      "limit": 100,
      "filterType": "MAX_NUM_ALGO_ORDERS"
    }, {
      "multiplierDown": "0.9500",
      "multiplierUp": "1.0500",
      "multiplierDecimal": "4",
      "filterType": "PERCENT_PRICE"
    }]
  }]
 }
 */
std::tuple<int, db::TBLRecSetSPtr<TBLSymbolInfo>>
SymbolTableMaintOfBinance::convertSymbolTableFromExch(
    const std::string& symbolTableFromExch) {
  auto doc = std::make_shared<Doc>();
  if (doc->Parse(symbolTableFromExch.data()).HasParseError()) {
    const auto statusMsg = fmt::format(
        "Convert symbol table from exch failed because of parse http rsp "
        "failed. {0} [offset {1}] {2}",
        GetParseError_En(doc->GetParseError()), doc->GetErrorOffset(),
        symbolTableFromExch);
    LOG_W(statusMsg);
    return std::make_tuple(-1, nullptr);
  }

  if (!doc->HasMember("symbols") || !(*doc)["symbols"].IsArray()) {
    const auto statusMsg = fmt::format(
        "Convert symbol table from exch failed because of "
        "invalid format of http rsp. {}",
        symbolTableFromExch);
    LOG_W(statusMsg);
    return std::make_tuple(-1, nullptr);
  }

  auto tblRecSet = std::make_shared<db::TBLRecSet<TBLSymbolInfo>>();
  for (std::size_t i = 0; i < (*doc)["symbols"].Size(); ++i) {
    auto [retOfMakeSym, symbolInfo] = makeSymbolInfo((*doc)["symbols"][i]);
    if (retOfMakeSym != 0) {
      const auto statusMsg =
          fmt::format("Convert symbol table from exch failed.");
      LOG_W(statusMsg);
      return std::make_tuple(-1, nullptr);
    }

    if (symbolInfo->symbolType != mdSvc_->getSymbolType()) {
      continue;
    }

    const auto tblRec = std::make_shared<db::TBLRec<TBLSymbolInfo>>(symbolInfo);
    tblRecSet->emplace(tblRec->getJsonStrOfKeyFields(), tblRec);
  }

  return {0, tblRecSet};
}

std::tuple<int, db::symbolInfo::RecordSPtr>
SymbolTableMaintOfBinance::doMakeSymbolInfo(const Val& v) const {
  auto convertAlias = [](const std::string& alias) {
    std::string ret;
    if (alias == "CURRENT_WEEK") {
      ret = "ThisWeek";
    } else if (alias == "NEXT_WEEK") {
      ret = "NextWeek";
    } else if (alias == "CURRENT_MONTH") {
      ret = "ThisMonth";
    } else if (alias == "NEXT_MONTH") {
      ret = "NextMonth";
    } else if (alias == "CURRENT_QUARTER") {
      ret = "ThisQuarter";
    } else if (alias == "NEXT_QUARTER") {
      ret = "NextQuarter";
    } else if (alias == "PERPETUAL") {
      ret = "PERPETUAL";
    } else {
      ret = alias;
    }
    return ret;
  };

  auto convertSymbolType = [](const auto& symbolInfo) {
    std::string ret;
    if (symbolInfo->alias == "") {
      ret = ENUM_VALUE_TO_STR(SymbolType::Spot);
    } else {
      if (symbolInfo->alias == "PERPETUAL") {
        if (symbolInfo->settlementCurrency == "USD") {
          ret = ENUM_VALUE_TO_STR(SymbolType::CPerp);
        } else {
          ret = ENUM_VALUE_TO_STR(SymbolType::Perp);
        }
      } else {
        if (symbolInfo->settlementCurrency == "USD") {
          ret = ENUM_VALUE_TO_STR(SymbolType::CFutures);
        } else {
          ret = ENUM_VALUE_TO_STR(SymbolType::Futures);
        }
      }
    }
    return ret;
  };

  auto convertSymbolCode = [](const auto& symbolInfo) {
    std::string ret = "";
    if (symbolInfo->symbolType == ENUM_VALUE_TO_STR(SymbolType::Spot)) {
      ret = fmt::format("{}{}{}", symbolInfo->baseCurrency, SEP_OF_SYMBOL_SPOT,
                        symbolInfo->quoteCurrency);

    } else if (symbolInfo->symbolType ==
               ENUM_VALUE_TO_STR(SymbolType::Futures)) {
      ret = fmt::format("{}{}{}{}{}", symbolInfo->baseCurrency,
                        SEP_OF_SYMBOL_FUTURES, symbolInfo->quoteCurrency,
                        SEP_OF_SYMBOL_FUTURES, symbolInfo->alias);

    } else if (symbolInfo->symbolType == ENUM_VALUE_TO_STR(SymbolType::Perp)) {
      ret =
          fmt::format("{}{}{}{}{}", symbolInfo->baseCurrency,
                      SEP_OF_SYMBOL_PERP, symbolInfo->quoteCurrency,
                      SEP_OF_SYMBOL_PERP, ENUM_VALUE_TO_STR(SymbolType::Perp));

    } else if (symbolInfo->symbolType ==
               ENUM_VALUE_TO_STR(SymbolType::CFutures)) {
      ret = fmt::format("{}{}{}{}{}", symbolInfo->baseCurrency,
                        SEP_OF_SYMBOL_FUTURES, symbolInfo->quoteCurrency,
                        SEP_OF_SYMBOL_FUTURES, symbolInfo->alias);

    } else if (symbolInfo->symbolType == ENUM_VALUE_TO_STR(SymbolType::CPerp)) {
      ret =
          fmt::format("{}{}{}{}{}", symbolInfo->baseCurrency,
                      SEP_OF_SYMBOL_PERP, symbolInfo->quoteCurrency,
                      SEP_OF_SYMBOL_PERP, ENUM_VALUE_TO_STR(SymbolType::CPerp));
    }
    return ret;
  };

  auto convertSymbolState = [](const auto& symbolInfo) {
    std::string ret;
    if (symbolInfo->symbolState == "PRE_TRADING") {
      ret = SYMBOL_STATE_PEROPEN;
    } else if (symbolInfo->symbolState == "TRADING") {
      ret = SYMBOL_STATE_ONLINE;
    } else {
      ret = SYMBOL_STATE_SUSPEND;
    }
    return ret;
  };

  Doc d;
  d.CopyFrom(v, d.GetAllocator());
  auto rec = std::make_shared<db::symbolInfo::Record>();
  GET_STR(rec->exchSymbolCode, d, "symbol", std::make_tuple(-1, nullptr));
  GET_STR(rec->baseCurrency, d, "baseAsset", std::make_tuple(-1, nullptr));
  GET_STR(rec->quoteCurrency, d, "quoteAsset", std::make_tuple(-1, nullptr));
  GET_STR(rec->settlementCurrency, d, "quoteAsset",
          std::make_tuple(-1, nullptr));

  boost::to_lower(rec->exchSymbolCode);

  if (d.HasMember("contractType") && d["contractType"].IsString()) {
    const std::string alias = d["contractType"].GetString();
    rec->alias = convertAlias(alias);
  } else {
    rec->alias = "";
  }

  rec->marketCode = mdSvc_->getMarketCode();
  rec->symbolType = convertSymbolType(rec);

  if (rec->symbolType == ENUM_VALUE_TO_STR(SymbolType::CFutures) ||
      rec->symbolType == ENUM_VALUE_TO_STR(SymbolType::CPerp)) {
    if (d.HasMember("contractSize") && d["contractSize"].IsUint()) {
      rec->parValue = d["contractSize"].GetUint();
    } else {
      rec->parValue = 0;
    }
    GET_STR(rec->symbolState, d, "contractStatus",
            std::make_tuple(-1, nullptr));
  } else {
    GET_STR(rec->symbolState, d, "status", std::make_tuple(-1, nullptr));
  }
  rec->symbolState = convertSymbolState(rec);

  rec->symbolCode = convertSymbolCode(rec);
  rec->symbolName = rec->symbolCode;

  for (std::size_t i = 0; i < d["filters"].Size(); ++i) {
    const auto filterType = d["filters"][i]["filterType"].GetString();
    if (std::strcmp(filterType, "LOT_SIZE") == 0) {
      rec->precOfOrderVol =
          RemoveTrailingZero(d["filters"][i]["stepSize"].GetString());
      rec->minOrderVol =
          RemoveTrailingZero(d["filters"][i]["minQty"].GetString());
      rec->maxOrderVol =
          RemoveTrailingZero(d["filters"][i]["maxQty"].GetString());
    }
    if (std::strcmp(filterType, "PRICE_FILTER") == 0) {
      rec->precOfOrderPrice =
          RemoveTrailingZero(d["filters"][i]["tickSize"].GetString());
    }
  }

  return {0, rec};
}

}  // namespace bq::md::svc::binance
