#include "TDSvcOfBinanceUtil.hpp"

#include "Config.hpp"
#include "TDSvc.hpp"
#include "TDSvcOfBinanceConst.hpp"
#include "TDSvcUtil.hpp"
#include "def/BQConst.hpp"
#include "def/DataStruOfTD.hpp"
#include "def/Def.hpp"
#include "util/Datetime.hpp"
#include "util/Float.hpp"
#include "util/Logger.hpp"
#include "util/String.hpp"

namespace bq::td::svc::binance {

std::string makeAddrWithSignature(TDSvc* tdSvc, const std::string& pathname,
                                  const std::string& query) {
  std::string ret;
  const auto secKey = std::any_cast<ApiInfoSPtr>(tdSvc->getAcctData())->secKey_;
  const auto now = GetTotalMSSince1970();
  if (query.empty()) {
    ret = fmt::format("timestamp={}", now);
  } else {
    ret = fmt::format("{}&timestamp={}", query, now);
  }
  const auto signature = HMACSHA256(ret, secKey, 32);
  ret = fmt::format("{}&signature={}", ret, signature);
  const auto addrOfHttp = CONFIG["addrOfHttp"].as<std::string>();
  ret = fmt::format("{}{}?{}", addrOfHttp, pathname, ret);

  return ret;
}

std::string getExchSide(Side side) {
  if (side == Side::Bid) {
    return EXCH_SIDE_BID;
  } else {
    return EXCH_SIDE_ASK;
  }
}

OrderStatus getOrderStatus(const OrderInfoSPtr& orderInfo,
                           const std::string& exchOrderStatus) {
  if (exchOrderStatus == "NEW") {
    return OrderStatus::ConfirmedByExch;

  } else if (exchOrderStatus == "FILLED") {
    return OrderStatus::Filled;

  } else if (exchOrderStatus == "PARTIALLY_FILLED") {
    return OrderStatus::PartialFilled;

  } else if (exchOrderStatus == "CANCELED") {
    if (isApproximatelyZero(orderInfo->dealSize_)) {
      return OrderStatus::Canceled;
    } else {
      return OrderStatus::PartialFilledCanceled;
    }

  } else if (exchOrderStatus == "REJECTED") {
    return OrderStatus::Failed;

  } else if (exchOrderStatus == "EXPIRED") {
    return OrderStatus::Failed;

  } else {
    return OrderStatus::Others;
    LOG_W("Recv unknown order status {}. {}", exchOrderStatus,
          orderInfo->toShortStr());
  }

  return OrderStatus::Others;
}

std::string getPathnameOfListenKey(SymbolType symbolType) {
  if (symbolType == SymbolType::Spot) {
    return pathnameOfListenKeySpot;
  } else if (symbolType == SymbolType::Perp ||
             symbolType == SymbolType::Futures) {
    return pathnameOfListenKeyUBasedContracts;
  } else if (symbolType == SymbolType::CPerp ||
             symbolType == SymbolType::CFutures) {
    return pathnameOfListenKeyCBasedContracts;
  } else {
    LOG_W("Get pathname of listenkey failed because of invalid symboltype {}.",
          magic_enum::enum_name(symbolType));
    return "";
  }
  return "";
}

std::string getPathnameOfAssetInfo(SymbolType symbolType) {
  if (symbolType == SymbolType::Spot) {
    return pathnameOfAssetInfoSpot;
  } else if (symbolType == SymbolType::Perp ||
             symbolType == SymbolType::Futures) {
    return pathnameOfAssetInfoUBasedContracts;
  } else if (symbolType == SymbolType::CPerp ||
             symbolType == SymbolType::CFutures) {
    return pathnameOfAssetInfoCBasedContracts;
  } else {
    LOG_W("Get pathname of assetinfo failed because of invalid symboltype {}.",
          magic_enum::enum_name(symbolType));
    return "";
  }
  return "";
}

std::string getPathnameOfOrder(SymbolType symbolType) {
  if (symbolType == SymbolType::Spot) {
    return pathnameOfOrderSpot;
  } else if (symbolType == SymbolType::Perp ||
             symbolType == SymbolType::Futures) {
    return pathnameOfOrderUBasedContracts;
  } else if (symbolType == SymbolType::CPerp ||
             symbolType == SymbolType::CFutures) {
    return pathnameOfOrderCBasedContracts;
  } else {
    LOG_W("Get pathname of order failed because of invalid symboltype {}.",
          magic_enum::enum_name(symbolType));
    return "";
  }
  return "";
}

std::string getPathnameOfQueryOrder(SymbolType symbolType) {
  if (symbolType == SymbolType::Spot) {
    return pathnameOfQueryOrderInfoSpot;
  } else if (symbolType == SymbolType::Perp ||
             symbolType == SymbolType::Futures) {
    return pathnameOfQueryOrderInfoUBasedContracts;
  } else if (symbolType == SymbolType::CPerp ||
             symbolType == SymbolType::CFutures) {
    return pathnameOfQueryOrderInfoCBasedContracts;
  } else {
    LOG_W(
        "Get pathname of query order failed because of invalid symboltype {}.",
        magic_enum::enum_name(symbolType));
    return "";
  }
  return "";
}

std::string getQueryStrOfOrder(const OrderInfoSPtr& orderInfo) {
  const auto symbolType = orderInfo->symbolType_;

  auto exchSymbolCode = orderInfo->exchSymbolCode_;
  boost::to_upper(exchSymbolCode);
  const auto exchSide = getExchSide(orderInfo->side_);
  const auto recvWindow = CONFIG["recvWindow"].as<std::uint32_t>();

  const auto orderSize = RemoveTrailingZero(
      fmt::format("{:.{}f}", orderInfo->orderSize_, DBL_PREC));
  const auto orderPrice = RemoveTrailingZero(
      fmt::format("{:.{}f}", orderInfo->orderPrice_, DBL_PREC));

  if (symbolType == SymbolType::Spot) {
    std::string query;
    query = fmt::format(
        "symbol={}&side={}&type={}&timeInForce={}&"
        "quantity={}&price={}&newClientOrderId={}&recvWindow={}",
        exchSymbolCode, exchSide, "LIMIT", "GTC", orderSize, orderPrice,
        orderInfo->orderId_, recvWindow);
    return query;

  } else if (symbolType == SymbolType::Perp ||
             symbolType == SymbolType::Futures ||
             symbolType == SymbolType::CPerp ||
             symbolType == SymbolType::CFutures) {
    const auto exchPosSide = magic_enum::enum_name(orderInfo->posSide_);
    auto query = fmt::format(
        "symbol={}&side={}&positionSide={}&type={}&timeInForce={}&"
        "quantity={}&price={}&newClientOrderId={}&recvWindow={}",
        exchSymbolCode, exchSide, exchPosSide, "LIMIT", "GTC", orderSize,
        orderPrice, orderInfo->orderId_, recvWindow);
    return query;

  } else {
    LOG_W("Get pathname of order failed because of invalid symboltype {}.",
          magic_enum::enum_name(symbolType));
    return "";
  }
  return "";
}

}  // namespace bq::td::svc::binance
