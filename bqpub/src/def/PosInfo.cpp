/*!
 * \file PosInfo.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "def/PosInfo.hpp"

#include "db/TBLPosInfo.hpp"
#include "def/DataStruOfMD.hpp"
#include "def/DataStruOfTD.hpp"
#include "def/PnlIF.hpp"
#include "def/SymbolInfoIF.hpp"
#include "util/Datetime.hpp"
#include "util/Float.hpp"
#include "util/Json.hpp"
#include "util/Logger.hpp"
#include "util/MarketDataCache.hpp"
#include "util/String.hpp"

namespace bq {

PosInfo::PosInfo() {}

std::vector<SymbolInfoSPtr> PosInfo::getSymbolGroupUsedToCalcPnl(
    const std::string& quoteCurrencyForCalc,
    const std::string& quoteCurrencyForConv) const {
  std::set<std::string> symbolGroup;
  if (symbolType_ == SymbolType::Spot) {
    const auto [ret, baseCurrency, quoteCurrency] =
        SplitStrIntoTwoParts(std::string(symbolCode_), SEP_OF_SYMBOL_SPOT);

    if (quoteCurrencyForCalc != quoteCurrency) {
      symbolGroup.emplace(fmt::format("{}{}{}", quoteCurrencyForCalc,
                                      SEP_OF_SYMBOL_SPOT, quoteCurrency));
      symbolGroup.emplace(fmt::format(
          "{}{}{}", quoteCurrency, SEP_OF_SYMBOL_SPOT, quoteCurrencyForCalc));
    }

    if (quoteCurrencyForCalc != quoteCurrencyForConv) {
      symbolGroup.emplace(fmt::format("{}{}{}", quoteCurrencyForCalc,
                                      SEP_OF_SYMBOL_SPOT,
                                      quoteCurrencyForConv));
    }

    if (quoteCurrency != quoteCurrencyForConv) {
      symbolGroup.emplace(fmt::format(
          "{}{}{}", quoteCurrency, SEP_OF_SYMBOL_SPOT, quoteCurrencyForConv));
    }

    if (quoteCurrencyForCalc != feeCurrency_) {
      symbolGroup.emplace(fmt::format("{}{}{}", quoteCurrencyForCalc,
                                      SEP_OF_SYMBOL_SPOT, feeCurrency_));
      symbolGroup.emplace(fmt::format(
          "{}{}{}", feeCurrency_, SEP_OF_SYMBOL_SPOT, quoteCurrencyForCalc));
    }

    if (feeCurrency_ != quoteCurrencyForConv) {
      symbolGroup.emplace(fmt::format(
          "{}{}{}", feeCurrency_, SEP_OF_SYMBOL_SPOT, quoteCurrencyForConv));
    }

  } else if (symbolType_ == SymbolType::Perp ||
             symbolType_ == SymbolType::Futures) {
    const std::string quoteCurrencyOfUSD = "USD";
    const std::string quoteCurrencyOfUSDT = "USDT";

    // quoteCurrencyOfUSD
    {
      if (quoteCurrencyForCalc != quoteCurrencyOfUSD) {
        symbolGroup.emplace(fmt::format("{}{}{}", quoteCurrencyForCalc,
                                        SEP_OF_SYMBOL_SPOT,
                                        quoteCurrencyOfUSD));
        symbolGroup.emplace(fmt::format("{}{}{}", quoteCurrencyOfUSD,
                                        SEP_OF_SYMBOL_SPOT,
                                        quoteCurrencyForCalc));
      }

      if (quoteCurrencyForCalc != quoteCurrencyForConv) {
        symbolGroup.emplace(fmt::format("{}{}{}", quoteCurrencyForCalc,
                                        SEP_OF_SYMBOL_SPOT,
                                        quoteCurrencyForConv));
      }

      if (quoteCurrencyOfUSD != quoteCurrencyForConv) {
        symbolGroup.emplace(fmt::format("{}{}{}", quoteCurrencyOfUSD,
                                        SEP_OF_SYMBOL_SPOT,
                                        quoteCurrencyForConv));
      }
    }

    // quoteCurrencyOfUSDT
    {
      if (quoteCurrencyForCalc != quoteCurrencyOfUSDT) {
        symbolGroup.emplace(fmt::format("{}{}{}", quoteCurrencyForCalc,
                                        SEP_OF_SYMBOL_SPOT,
                                        quoteCurrencyOfUSDT));
        symbolGroup.emplace(fmt::format("{}{}{}", quoteCurrencyOfUSDT,
                                        SEP_OF_SYMBOL_SPOT,
                                        quoteCurrencyForCalc));
      }

      if (quoteCurrencyForCalc != quoteCurrencyForConv) {
        symbolGroup.emplace(fmt::format("{}{}{}", quoteCurrencyForCalc,
                                        SEP_OF_SYMBOL_SPOT,
                                        quoteCurrencyForConv));
      }

      if (quoteCurrencyOfUSDT != quoteCurrencyForConv) {
        symbolGroup.emplace(fmt::format("{}{}{}", quoteCurrencyOfUSDT,
                                        SEP_OF_SYMBOL_SPOT,
                                        quoteCurrencyForConv));
      }
    }

    if (quoteCurrencyForCalc != feeCurrency_) {
      symbolGroup.emplace(fmt::format("{}{}{}", quoteCurrencyForCalc,
                                      SEP_OF_SYMBOL_SPOT, feeCurrency_));
      symbolGroup.emplace(fmt::format(
          "{}{}{}", feeCurrency_, SEP_OF_SYMBOL_SPOT, quoteCurrencyForCalc));
    }

    if (feeCurrency_ != quoteCurrencyForConv) {
      symbolGroup.emplace(fmt::format(
          "{}{}{}", feeCurrency_, SEP_OF_SYMBOL_SPOT, quoteCurrencyForConv));
    }

  } else if (symbolType_ == SymbolType::CPerp ||
             symbolType_ == SymbolType::CFutures) {
    const auto sep = symbolType_ == SymbolType::Perp ? SEP_OF_SYMBOL_PERP
                                                     : SEP_OF_SYMBOL_FUTURES;
    const auto baseCurrency =
        std::string(symbolCode_, strstr(symbolCode_, sep.c_str()));
    const auto quoteCurrency = baseCurrency;

    if (quoteCurrencyForCalc != quoteCurrency) {
      symbolGroup.emplace(fmt::format("{}{}{}", quoteCurrencyForCalc,
                                      SEP_OF_SYMBOL_SPOT, quoteCurrency));
      symbolGroup.emplace(fmt::format(
          "{}{}{}", quoteCurrency, SEP_OF_SYMBOL_SPOT, quoteCurrencyForCalc));
    }

    if (quoteCurrencyForCalc != quoteCurrencyForConv) {
      symbolGroup.emplace(fmt::format("{}{}{}", quoteCurrencyForCalc,
                                      SEP_OF_SYMBOL_SPOT,
                                      quoteCurrencyForConv));
    }

    if (quoteCurrency != quoteCurrencyForConv) {
      symbolGroup.emplace(fmt::format(
          "{}{}{}", quoteCurrency, SEP_OF_SYMBOL_SPOT, quoteCurrencyForConv));
    }

    if (quoteCurrencyForCalc != feeCurrency_) {
      symbolGroup.emplace(fmt::format("{}{}{}", quoteCurrencyForCalc,
                                      SEP_OF_SYMBOL_SPOT, feeCurrency_));
      symbolGroup.emplace(fmt::format(
          "{}{}{}", feeCurrency_, SEP_OF_SYMBOL_SPOT, quoteCurrencyForCalc));
    }

    if (feeCurrency_ != quoteCurrencyForConv) {
      symbolGroup.emplace(fmt::format(
          "{}{}{}", feeCurrency_, SEP_OF_SYMBOL_SPOT, quoteCurrencyForConv));
    }

  } else {
    LOG_W("Unhandled symbolType {}.", magic_enum::enum_name(symbolType_));
  }

  std::vector<SymbolInfoSPtr> ret;
  for (const auto& symbolCode : symbolGroup) {
    ret.emplace_back(
        std::make_shared<SymbolInfo>(marketCode_, symbolType_, symbolCode));
  }
  return ret;
}

PnlSPtr PosInfo::calcPnl(
    const MarketDataCacheSPtr& marketDataCache,
    const std::string& quoteCurrencyForCalc,
    const std::string& quoteCurrencyForConv,
    const std::string& origQuoteCurrencyOfUBasedContract) const {
  auto pnl = std::make_shared<Pnl>();
  pnl->quoteCurrencyForCalc_ = quoteCurrencyForCalc;

  if (symbolType_ == SymbolType::Spot) {
    const auto [ret, baseCurrency, quoteCurrency] =
        SplitStrIntoTwoParts(std::string(symbolCode_), SEP_OF_SYMBOL_SPOT);
    {
      const auto [statusCode, symbolGroupForCalc, updateTime, lastPrice] =
          CalcPrice(marketDataCache, marketCode_, baseCurrency, quoteCurrency,
                    quoteCurrencyForCalc, quoteCurrencyForConv);
      if (statusCode == 0) {
        LOG_T("Calc price of {}-{} success. [price: {}; delay: {}s]",
              quoteCurrency, quoteCurrencyForCalc, lastPrice,
              (GetTotalSecSince1970() - updateTime / 1000000));
        pnl->pnlUnReal_ = pnlUnReal_ / lastPrice;
        pnl->pnlReal_ = pnlReal_ / lastPrice;
        pnl->updateTime_ = updateTime_;
      } else {
        pnl->statusCode_ = statusCode;
        pnl->symbolGroupForCalc_ = std::move(symbolGroupForCalc);
        LOG_W("Calc pnl failed because of calc price of {}{}{} failed. {}",
              quoteCurrency, SEP_OF_SYMBOL_SPOT, quoteCurrencyForCalc, toStr());
        return pnl;
      }
    }

    {
      const auto [statusCode, symbolGroupForCalc, updateTime, lastPrice] =
          CalcPrice(marketDataCache, marketCode_, feeCurrency_, feeCurrency_,
                    quoteCurrencyForCalc, quoteCurrencyForConv);
      if (statusCode == 0) {
        LOG_T("Calc price of {}-{} success. [price: {}; delay: {}s]",
              feeCurrency_, quoteCurrencyForCalc, lastPrice,
              (GetTotalSecSince1970() - updateTime / 1000000));
        pnl->fee_ = fee_ / lastPrice;
        if (updateTime < pnl->updateTime_) pnl->updateTime_ = updateTime;
      } else {
        pnl->statusCode_ = statusCode;
        pnl->symbolGroupForCalc_ = std::move(symbolGroupForCalc);
        LOG_W("Calc pnl failed because of calc price of {}{}{} failed. {}",
              feeCurrency_, SEP_OF_SYMBOL_SPOT, quoteCurrencyForCalc, toStr());
        return pnl;
      }
    }

  } else if (symbolType_ == SymbolType::Perp ||
             symbolType_ == SymbolType::Futures) {
    // calc pnl
    {
      const auto sep = symbolType_ == SymbolType::Perp ? SEP_OF_SYMBOL_PERP
                                                       : SEP_OF_SYMBOL_FUTURES;
      const auto baseCurrency =
          std::string(symbolCode_, strstr(symbolCode_, sep.c_str()));
      const auto [statusCode, symbolGroupForCalc, updateTime, lastPrice] =
          CalcPrice(marketDataCache, marketCode_, baseCurrency,
                    origQuoteCurrencyOfUBasedContract, quoteCurrencyForCalc,
                    quoteCurrencyForConv);
      if (statusCode == 0) {
        pnl->pnlUnReal_ = pnlUnReal_ / lastPrice;
        pnl->pnlReal_ = pnlReal_ / lastPrice;
        pnl->updateTime_ = updateTime_;
      } else {
        pnl->statusCode_ = statusCode;
        pnl->symbolGroupForCalc_ = std::move(symbolGroupForCalc);
        LOG_W("Calc pnl failed because of calc price of {}{}{} failed. {}",
              origQuoteCurrencyOfUBasedContract, SEP_OF_SYMBOL_SPOT,
              quoteCurrencyForCalc, toStr());
        return pnl;
      }
    }

    {
      const auto [statusCode, symbolGroupForCalc, updateTime, lastPrice] =
          CalcPrice(marketDataCache, marketCode_, feeCurrency_, feeCurrency_,
                    quoteCurrencyForCalc, quoteCurrencyForConv);
      if (statusCode == 0) {
        pnl->fee_ = fee_ / lastPrice;
        if (updateTime < pnl->updateTime_) pnl->updateTime_ = updateTime;
      } else {
        pnl->statusCode_ = statusCode;
        pnl->symbolGroupForCalc_ = std::move(symbolGroupForCalc);
        LOG_W("Calc pnl failed because of calc price of {}{}{} failed. {}",
              feeCurrency_, SEP_OF_SYMBOL_SPOT, quoteCurrencyForCalc, toStr());
        return pnl;
      }
    }

  } else if (symbolType_ == SymbolType::CPerp ||
             symbolType_ == SymbolType::CFutures) {
    // calc pnl
    {
      const auto sep = symbolType_ == SymbolType::Perp ? SEP_OF_SYMBOL_PERP
                                                       : SEP_OF_SYMBOL_FUTURES;
      const auto baseCurrency =
          std::string(symbolCode_, strstr(symbolCode_, sep.c_str()));
      const auto [statusCode, symbolGroupForCalc, updateTime, lastPrice] =
          CalcPrice(marketDataCache, marketCode_, baseCurrency, baseCurrency,
                    quoteCurrencyForCalc, quoteCurrencyForConv);
      if (statusCode == 0) {
        pnl->pnlUnReal_ = pnlUnReal_ / lastPrice;
        pnl->pnlReal_ = pnlReal_ / lastPrice;
        pnl->updateTime_ = updateTime_;
      } else {
        pnl->statusCode_ = statusCode;
        pnl->symbolGroupForCalc_ = std::move(symbolGroupForCalc);
        LOG_W("Calc pnl failed because of calc price of {}{}{} failed. {}",
              baseCurrency, SEP_OF_SYMBOL_SPOT, quoteCurrencyForCalc, toStr());
        return pnl;
      }
    }

    {
      const auto [statusCode, symbolGroupForCalc, updateTime, lastPrice] =
          CalcPrice(marketDataCache, marketCode_, feeCurrency_, feeCurrency_,
                    quoteCurrencyForCalc, quoteCurrencyForConv);
      if (statusCode == 0) {
        pnl->fee_ = fee_ / lastPrice;
        if (updateTime < pnl->updateTime_) pnl->updateTime_ = updateTime;
      } else {
        pnl->statusCode_ = statusCode;
        pnl->symbolGroupForCalc_ = std::move(symbolGroupForCalc);
        LOG_W("Calc pnl failed because of calc price of {}{}{} failed. {}",
              feeCurrency_, SEP_OF_SYMBOL_SPOT, quoteCurrencyForCalc, toStr());
        return pnl;
      }
    }

  } else {
    LOG_W("Unhandled symbolType {}.", magic_enum::enum_name(symbolType_));
  }

  return pnl;
}

std::string PosInfo::toStr() const {
  std::string ret;
  ret = fmt::format(
      "{} fee={}; pos={}; prePos={}; avgOpenPrice={}; pnlUnReal={}; pnlReal={}",
      getKey(), fee_, pos_, prePos_, avgOpenPrice_, pnlUnReal_, pnlReal_);
  return ret;
}

std::string PosInfo::getKey() const {
  const auto ret = fmt::format(
      "{}/{}/{}/{}/{}/{}/{}/{}/{}/{}/{}/{}/{}", productId_, userId_, acctId_,
      stgId_, stgInstId_, algoId_, GetMarketName(marketCode_),
      magic_enum::enum_name(symbolType_), symbolCode_,
      magic_enum::enum_name(side_), magic_enum::enum_name(posSide_), parValue_,
      feeCurrency_);
  return ret;
}

std::string PosInfo::getTopicPrefix() const {
  const auto ret = fmt::format("{}{}{}{}{}{}{}{}", TOPIC_PREFIX_OF_MARKET_DATA,
                               SEP_OF_TOPIC, GetMarketName(marketCode_),
                               SEP_OF_TOPIC, magic_enum::enum_name(symbolType_),
                               SEP_OF_TOPIC, symbolCode_, SEP_OF_TOPIC);
  return ret;
}

bool PosInfo::oneMoreFeeCurrencyThanInput(const PosInfoSPtr& posInfo) const {
  bool keyWithoutFeeCurrencyNotEqual =
      (productId_ != posInfo->productId_ || userId_ != posInfo->userId_ ||
       acctId_ != posInfo->acctId_ || stgId_ != posInfo->stgId_ ||
       stgInstId_ != posInfo->stgInstId_ || algoId_ != posInfo->algoId_ ||
       marketCode_ != posInfo->marketCode_ ||
       symbolType_ != posInfo->symbolType_ ||
       strcmp(symbolCode_, posInfo->symbolCode_) != 0 ||
       side_ != posInfo->side_ || posSide_ != posInfo->posSide_ ||
       parValue_ != posInfo->parValue_);

  if (keyWithoutFeeCurrencyNotEqual || feeCurrency_[0] == '\0') {
    return false;
  } else {
    return true;
  }
}

bool PosInfo::isEqual(const PosInfoSPtr& posInfo) {
  if (pnlUnReal_ != posInfo->pnlUnReal_) return false;
  if (pnlReal_ != posInfo->pnlReal_) return false;
  if (totalBidSize_ != posInfo->totalBidSize_) return false;
  if (totalAskSize_ != posInfo->totalAskSize_) return false;
  if (fee_ != posInfo->fee_) return false;
  if (pos_ != posInfo->pos_) return false;
  if (prePos_ != posInfo->prePos_) return false;
  if (avgOpenPrice_ != posInfo->avgOpenPrice_) return false;
  return true;
}

// clang-format off
std::string PosInfo::getSqlOfReplace() const {
const auto sql = fmt::format(
"REPLACE INTO `BetterQuant`.`posInfo` ("
  "`productId`,"
  "`userId`,"
  "`acctId`,"
  "`stgId`,"
  "`stgInstId`,"
  "`algoId`,"
  "`marketCode`,"
  "`symbolType`,"
  "`symbolCode`,"
  "`side`,"
  "`posSide`,"
  "`parValue`,"
  "`feeCurrency`,"
  "`fee`,"
  "`pos`,"
  "`prePos`,"
  "`avgOpenPrice`,"
  "`pnlUnReal`,"
  "`pnlReal`,"
  "`totalBidSize`,"
  "`totalAskSize`,"
  "`lastNoUsedToCalcPos`,"
  "`updateTime`"
")"
"VALUES"
"("
  " {} ,"  // productId
  " {} ,"  // userId
  " {} ,"  // acctId
  " {} ,"  // stgId
  " {} ,"  // stgInstId
  " {} ,"  // algoId
  "'{}',"  // marketCode
  "'{}',"  // symbolType
  "'{}',"  // symbolCode
  "'{}',"  // side
  "'{}',"  // posSide
  " {} ,"  // parValue
  "'{}',"  // feeCurrency
  "'{}',"  // fee
  "'{}',"  // pos
  "'{}',"  // prePos
  "'{}',"  // avgOpenPrice
  "'{}',"  // pnlUnReal
  "'{}',"  // pnlReal
  "'{}',"  // totalBidSize
  "'{}',"  // totalAskSize
  " {} ,"  // lastNoUsedToCalcPos
  "'{}' "  // updateTime
"); ",
  productId_,
  userId_,
  acctId_,
  stgId_,
  stgInstId_,
  algoId_,
  GetMarketName(marketCode_),
  magic_enum::enum_name(symbolType_),
  symbolCode_,
  magic_enum::enum_name(side_),
  magic_enum::enum_name(posSide_),
  parValue_,
  feeCurrency_,
  fee_,
  pos_,
  prePos_,
  avgOpenPrice_,
  pnlUnReal_,
  pnlReal_,
  totalBidSize_,
  totalAskSize_,
  lastNoUsedToCalcPos_,
  ConvertTsToDBTime(updateTime_)
);
return sql;
} ;

std::string PosInfo::getSqlOfInsert() const {
const auto sql = fmt::format(
"INSERT INTO `BetterQuant`.`posInfo` ("
  "`productId`,"
  "`userId`,"
  "`acctId`,"
  "`stgId`,"
  "`stgInstId`,"
  "`algoId`,"
  "`marketCode`,"
  "`symbolType`,"
  "`symbolCode`,"
  "`side`,"
  "`posSide`,"
  "`parValue`,"
  "`feeCurrency`,"
  "`fee`,"
  "`pos`,"
  "`prePos`,"
  "`avgOpenPrice`,"
  "`pnlUnReal`,"
  "`pnlReal`,"
  "`totalBidSize`,"
  "`totalAskSize`,"
  "`lastNoUsedToCalcPos`,"
  "`updateTime`"
")"
"VALUES"
"("
  " {} ,"  // productId
  " {} ,"  // userId
  " {} ,"  // acctId
  " {} ,"  // stgId
  " {} ,"  // stgInstId
  " {} ,"  // algoId
  "'{}',"  // marketCode
  "'{}',"  // symbolType
  "'{}',"  // symbolCode
  "'{}',"  // side
  "'{}',"  // posSide
  " {} ,"  // parValue
  "'{}',"  // feeCurrency
  "'{}',"  // fee
  "'{}',"  // pos
  "'{}',"  // prePos
  "'{}',"  // avgOpenPrice
  "'{}',"  // pnlUnReal
  "'{}',"  // pnlReal
  "'{}',"  // totalBidSize
  "'{}',"  // totalAskSize
  " {} ,"  // lastNoUsedToCalcPos
  "'{}' "  // updateTime
"); ",
  productId_,
  userId_,
  acctId_,
  stgId_,
  stgInstId_,
  algoId_,
  GetMarketName(marketCode_),
  magic_enum::enum_name(symbolType_),
  symbolCode_,
  magic_enum::enum_name(side_),
  magic_enum::enum_name(posSide_),
  parValue_,
  feeCurrency_,
  fee_,
  pos_,
  prePos_,
  avgOpenPrice_,
  pnlUnReal_,
  pnlReal_,
  totalBidSize_,
  totalAskSize_,
  lastNoUsedToCalcPos_,
  ConvertTsToDBTime(updateTime_)
);
return sql;
} ;

std::string PosInfo::getSqlOfUpdate() const {
const auto sql = fmt::format(
"UPDATE `BetterQuant`.`posInfo` SET "
  "`fee`                = {}, "
  "`pos`                = {}, "
  "`prePos`             = {}, "
  "`avgOpenPrice`       = {}, "
  "`pnlUnreal`          = {}, "
  "`pnlReal`            = {}, "
  "`totalBidSize`       = {}, "
  "`totalAskSize`       = {}, "
  "`lastNoUsedToCalcPos`= {},"
  "`updateTime`         ='{}' "
"WHERE `productId`  = {}  "
  "AND `userId`     = {}  "
  "AND `acctId`     = {}  "
  "AND `stgId`      = {}  "
  "AND `stgInstId`  = {}  "
  "AND `algoId`     = {}  "
  "AND `marketCode` ='{}' "
  "AND `symbolType` ='{}' "
  "AND `symbolCode` ='{}' "
  "AND `side`       ='{}' "
  "AND `posSide`    ='{}' "
  "AND `parValue`   = {}  "
  "AND `feeCurrency`='{}';",
  fee_,
  pos_,
  prePos_,
  avgOpenPrice_,
  pnlUnReal_,
  pnlReal_,
  totalBidSize_,
  totalAskSize_,
  lastNoUsedToCalcPos_,
  ConvertTsToDBTime(updateTime_),
  productId_,
  userId_,
  acctId_,
  stgId_,
  stgInstId_,
  algoId_,
  GetMarketName(marketCode_),
  magic_enum::enum_name(symbolType_),
  symbolCode_,
  magic_enum::enum_name(side_),
  magic_enum::enum_name(posSide_),
  parValue_,
  feeCurrency_
);
return sql;
}

std::string PosInfo::getSqlOfDelete() const{
const auto sql = fmt::format(
  "DELETE FROM `BetterQuant`.`posInfo` "
  "WHERE `productId`  = {}  "
  "  AND `userId`     = {}  "
  "  AND `acctId`     = {}  "
  "  AND `stgId`      = {}  "
  "  AND `stgInstId`  = {}  "
  "  AND `algoId`     = {}  "
  "  AND `marketCode` ='{}' "
  "  AND `symbolType` ='{}' "
  "  AND `symbolCode` ='{}' "
  "  AND `side`       ='{}' "
  "  AND `posSide`    ='{}' "
  "  AND `parValue`   ='{}' "
  "  AND `feeCurrency`='{}';",
  productId_,
  userId_,
  acctId_,
  stgId_,
  stgInstId_,
  algoId_,
  GetMarketName(marketCode_),
  magic_enum::enum_name(symbolType_),
  symbolCode_,
  magic_enum::enum_name(side_),
  magic_enum::enum_name(posSide_),
  parValue_,
  feeCurrency_
);
return sql;
} ;
// clang-format on

PosInfoSPtr MakePosInfo(const db::posInfo::RecordSPtr& recPosInfo) {
  auto posInfo = std::make_shared<PosInfo>();

  posInfo->productId_ = recPosInfo->productId;
  posInfo->userId_ = recPosInfo->userId;
  posInfo->acctId_ = recPosInfo->acctId;
  posInfo->stgId_ = recPosInfo->stgId;
  posInfo->stgInstId_ = recPosInfo->stgInstId;
  posInfo->algoId_ = recPosInfo->algoId;

  posInfo->marketCode_ =
      magic_enum::enum_cast<MarketCode>(recPosInfo->marketCode).value();
  posInfo->symbolType_ =
      magic_enum::enum_cast<SymbolType>(recPosInfo->symbolType).value();
  strncpy(posInfo->symbolCode_, recPosInfo->symbolCode.c_str(),
          sizeof(posInfo->symbolCode_) - 1);

  posInfo->side_ = magic_enum::enum_cast<Side>(recPosInfo->side).value();
  posInfo->posSide_ =
      magic_enum::enum_cast<PosSide>(recPosInfo->posSide).value();

  posInfo->parValue_ = recPosInfo->parValue;
  strncpy(posInfo->feeCurrency_, recPosInfo->feeCurrency.c_str(),
          sizeof(posInfo->feeCurrency_) - 1);
  posInfo->fee_ = CONV(Decimal, recPosInfo->fee);
  posInfo->pos_ = CONV(Decimal, recPosInfo->pos);
  posInfo->prePos_ = CONV(Decimal, recPosInfo->prePos);
  posInfo->avgOpenPrice_ = CONV(Decimal, recPosInfo->avgOpenPrice);
  posInfo->pnlUnReal_ = CONV(Decimal, recPosInfo->pnlUnReal);
  posInfo->pnlReal_ = CONV(Decimal, recPosInfo->pnlReal);
  posInfo->totalBidSize_ = CONV(Decimal, recPosInfo->totalBidSize);
  posInfo->totalAskSize_ = CONV(Decimal, recPosInfo->totalAskSize);
  posInfo->updateTime_ = ConvertDBTimeToTS(recPosInfo->updateTime);

  const auto key = posInfo->getKey();
  posInfo->keyHash_ = XXH3_64bits(key.data(), key.size());

  return posInfo;
}

PosInfoSPtr MakePosInfoOfContract(const OrderInfoSPtr& orderInfo) {
  auto posInfo = std::make_shared<PosInfo>();

  posInfo->productId_ = orderInfo->productId_;
  posInfo->userId_ = orderInfo->userId_;
  posInfo->acctId_ = orderInfo->acctId_;
  posInfo->stgId_ = orderInfo->stgId_;
  posInfo->stgInstId_ = orderInfo->stgInstId_;
  posInfo->algoId_ = orderInfo->algoId_;

  posInfo->marketCode_ = orderInfo->marketCode_;
  posInfo->symbolType_ = orderInfo->symbolType_;
  strncpy(posInfo->symbolCode_, orderInfo->symbolCode_,
          sizeof(posInfo->symbolCode_) - 1);

  posInfo->side_ = orderInfo->side_;
  posInfo->posSide_ = orderInfo->posSide_;

  posInfo->parValue_ = orderInfo->parValue_;

  strncpy(posInfo->feeCurrency_, orderInfo->feeCurrency_,
          sizeof(posInfo->feeCurrency_) - 1);
  posInfo->fee_ = orderInfo->getFeeOfLastTrade();

  posInfo->pos_ = orderInfo->lastDealSize_;
  posInfo->prePos_ = 0;

  posInfo->avgOpenPrice_ = orderInfo->lastDealPrice_;

  posInfo->pnlUnReal_ = 0;
  posInfo->pnlReal_ = 0;
  posInfo->totalBidSize_ = 0;
  posInfo->totalAskSize_ = 0;

  posInfo->updateTime_ = GetTotalUSSince1970();

  const auto key = posInfo->getKey();
  posInfo->keyHash_ = XXH3_64bits(key.data(), key.size());

  return posInfo;
}

PosInfoSPtr MakePosInfoOfContractWithKeyFields(const OrderInfoSPtr& orderInfo,
                                               Side side) {
  auto posInfo = std::make_shared<PosInfo>();

  posInfo->productId_ = orderInfo->productId_;
  posInfo->userId_ = orderInfo->userId_;
  posInfo->acctId_ = orderInfo->acctId_;
  posInfo->stgId_ = orderInfo->stgId_;
  posInfo->stgInstId_ = orderInfo->stgInstId_;
  posInfo->algoId_ = orderInfo->algoId_;

  posInfo->marketCode_ = orderInfo->marketCode_;
  posInfo->symbolType_ = orderInfo->symbolType_;
  strncpy(posInfo->symbolCode_, orderInfo->symbolCode_,
          sizeof(posInfo->symbolCode_) - 1);

  if (side == Side::Others) {
    posInfo->side_ = orderInfo->side_;
  } else {
    posInfo->side_ = side;
  }
  posInfo->posSide_ = orderInfo->posSide_;

  posInfo->parValue_ = orderInfo->parValue_;
  strncpy(posInfo->feeCurrency_, orderInfo->feeCurrency_,
          sizeof(posInfo->feeCurrency_) - 1);

  posInfo->updateTime_ = GetTotalUSSince1970();

  const auto key = posInfo->getKey();
  posInfo->keyHash_ = XXH3_64bits(key.data(), key.size());

  return posInfo;
}

bool isEqual(const Key2PosInfoGroupSPtr& lhs, const Key2PosInfoGroupSPtr& rhs) {
  if (lhs->size() != rhs->size()) {
    return false;
  }
  auto lhsIter = std::begin(*lhs);
  auto rhsIter = std::begin(*rhs);
  while (true) {
    if (lhsIter == std::end(*lhs)) break;
    if (lhsIter->first != rhsIter->first) return false;
    ++lhsIter;
    ++rhsIter;
  }

  for (const auto& lhsRec : *lhs) {
    const auto& lhsPosInfo = lhsRec.second;
    const auto& rhsPosInfo = (*rhs)[lhsRec.first];
    if (lhsPosInfo->isEqual(rhsPosInfo) == false) {
      return false;
    }
  }

  return true;
}

Key2PosInfoGroup MakePosUpdateOfAcctId(
    const PosUpdateOfAcctIdForPubSPtr& posUpdateOfAcctIdForPub) {
  Key2PosInfoGroup ret;
  auto posInfoAddr = posUpdateOfAcctIdForPub->posInfoGroup_;
  for (std::uint16_t i = 0; i < posUpdateOfAcctIdForPub->num_; ++i) {
    const auto posInfoRecv = reinterpret_cast<const PosInfo*>(posInfoAddr);
    const auto posInfo = std::make_shared<PosInfo>(*posInfoRecv);
    ret.emplace(posInfo->getKey(), posInfo);
    posInfoAddr += sizeof(PosInfo);
  }
  return ret;
}

Key2PosInfoGroup MakePosUpdateOfStgId(
    const PosUpdateOfStgIdForPubSPtr& posUpdateOfStgIdForPub) {
  Key2PosInfoGroup ret;
  auto posInfoAddr = posUpdateOfStgIdForPub->posInfoGroup_;
  for (std::uint16_t i = 0; i < posUpdateOfStgIdForPub->num_; ++i) {
    const auto posInfoRecv = reinterpret_cast<const PosInfo*>(posInfoAddr);
    const auto posInfo = std::make_shared<PosInfo>(*posInfoRecv);
    ret.emplace(posInfo->getKey(), posInfo);
    posInfoAddr += sizeof(PosInfo);
  }
  return ret;
}

Key2PosInfoGroup MakePosUpdateOfStgInstId(
    const PosUpdateOfStgInstIdForPubSPtr& posUpdateOfStgInstIdForPub) {
  Key2PosInfoGroup ret;
  auto posInfoAddr = posUpdateOfStgInstIdForPub->posInfoGroup_;
  for (std::uint16_t i = 0; i < posUpdateOfStgInstIdForPub->num_; ++i) {
    const auto posInfoRecv = reinterpret_cast<const PosInfo*>(posInfoAddr);
    const auto posInfo = std::make_shared<PosInfo>(*posInfoRecv);
    ret.emplace(posInfo->getKey(), posInfo);
    posInfoAddr += sizeof(PosInfo);
  }
  return ret;
}

Decimal CalcAvgOpenPrice(const PosInfoSPtr& lhs, const PosInfoSPtr& rhs) {
  assert(lhs->symbolType_ == rhs->symbolType_ &&
         "lhs->symbolType_ == rhs->symbolType_");

  Decimal avgOpenPrice = 0;

  if (lhs->symbolType_ == SymbolType::Spot ||
      lhs->symbolType_ == SymbolType::Perp ||
      lhs->symbolType_ == SymbolType::Futures) {
    const auto totalPos = lhs->pos_ + rhs->pos_;
    if (!isApproximatelyZero(totalPos)) {
      const auto totalAmt =
          lhs->pos_ * lhs->avgOpenPrice_ + rhs->pos_ * rhs->avgOpenPrice_;
      avgOpenPrice = totalAmt / totalPos;
    }

  } else if (lhs->symbolType_ == SymbolType::CPerp ||
             lhs->symbolType_ == SymbolType::CFutures) {
    const auto totalPos = lhs->pos_ + rhs->pos_;
    if (isApproximatelyZero(lhs->avgOpenPrice_) &&
        isApproximatelyZero(rhs->avgOpenPrice_)) {
      avgOpenPrice = 0;
    } else {
      if (isApproximatelyZero(lhs->avgOpenPrice_)) {
        avgOpenPrice = totalPos / (rhs->pos_ / rhs->avgOpenPrice_);

      } else if (isApproximatelyZero(rhs->avgOpenPrice_)) {
        avgOpenPrice = totalPos / (lhs->pos_ / lhs->avgOpenPrice_);

      } else {
        avgOpenPrice = totalPos / (lhs->pos_ / lhs->avgOpenPrice_ +
                                   rhs->pos_ / rhs->avgOpenPrice_);
      }
    }

  } else {
    LOG_W("Unhandled symbolType {}.", magic_enum::enum_name(lhs->symbolType_));
  }

  return avgOpenPrice;
}

void MergePosInfoHasNoFeeCurrency(PosInfoGroup& posInfoGroup) {
  auto getPosInfoUsedToMerge = [](const auto& posInfoGroup,
                                  const auto& posInfoHasNoFeeCurrency) {
    PosInfoSPtr ret = nullptr;
    for (const auto& posInfo : posInfoGroup) {
      if (posInfo->oneMoreFeeCurrencyThanInput(posInfoHasNoFeeCurrency)) {
        if (ret == nullptr || posInfo->updateTime_ > ret->updateTime_) {
          ret = posInfo;
        }
      }
    }
    return ret;
  };

  auto calcFeeOfEstimate = [](const auto& posInfo,
                              const auto& posInfoHasNoFeeCurrency) {
    Decimal feeOfEstimated = 0;
    const auto totalDealSize = posInfo->totalBidSize_ - posInfo->totalAskSize_;
    const auto totalDealSizeOfPosInfoHasNoFeeCurrency =
        posInfoHasNoFeeCurrency->totalBidSize_ -
        posInfoHasNoFeeCurrency->totalAskSize_;
    if (!isApproximatelyZero(totalDealSize)) {
      feeOfEstimated = totalDealSizeOfPosInfoHasNoFeeCurrency / totalDealSize *
                       posInfo->fee_;
    }
    return feeOfEstimated;
  };

  auto mergePosInfo = [&](auto& posInfo, const auto& posInfoHasNoFeeCurrency) {
    const auto feeOfEstimated =
        calcFeeOfEstimate(posInfo, posInfoHasNoFeeCurrency);
    posInfo->fee_ += feeOfEstimated;
    posInfo->avgOpenPrice_ = CalcAvgOpenPrice(posInfo, posInfoHasNoFeeCurrency);
    posInfo->pos_ += posInfoHasNoFeeCurrency->pos_;
    posInfo->prePos_ += posInfoHasNoFeeCurrency->prePos_;
    posInfo->pnlUnReal_ += posInfoHasNoFeeCurrency->pnlUnReal_;
    posInfo->pnlReal_ += posInfoHasNoFeeCurrency->pnlReal_;
    posInfo->totalBidSize_ += posInfoHasNoFeeCurrency->totalBidSize_;
    posInfo->totalAskSize_ += posInfoHasNoFeeCurrency->totalAskSize_;
  };

  PosInfoGroup posInfoHasNoFeeCurrencyGroup;
  std::ext::erase_if(posInfoGroup, [&](const auto& posInfo) {
    if (posInfo->feeCurrency_[0] == '\0') {
      posInfoHasNoFeeCurrencyGroup.emplace_back(posInfo);
      return true;
    } else {
      return false;
    }
  });

  for (const auto& posInfoHasNoFeeCurrency : posInfoHasNoFeeCurrencyGroup) {
    auto posInfo = getPosInfoUsedToMerge(posInfoGroup, posInfoHasNoFeeCurrency);
    if (posInfo != nullptr) {
      mergePosInfo(posInfo, posInfoHasNoFeeCurrency);
    } else {
      posInfoGroup.emplace_back(posInfo);
    }
  }
}

}  // namespace bq
