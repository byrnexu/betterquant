/*!
 * \file TDSvcOfBinanceUtil.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "def/BQConst.hpp"
#include "util/Pch.hpp"

namespace bq {
struct OrderInfo;
using OrderInfoSPtr = std::shared_ptr<OrderInfo>;
}  // namespace bq

namespace bq::td::svc {
class TDSvc;
}

namespace bq::td::svc::binance {

std::string makeAddrWithSignature(TDSvc* tdSvc, const std::string& pathname,
                                  const std::string& query);

std::string getExchSide(Side side);

OrderStatus getOrderStatus(const OrderInfoSPtr& orderInfo,
                           const std::string& exchOrderStatus);

std::string getPathnameOfListenKey(SymbolType symbolType);
std::string getPathnameOfAssetInfo(SymbolType symbolType);
std::string getPathnameOfOrder(SymbolType symbolType);
std::string getPathnameOfQueryOrder(SymbolType symbolType);

std::string getQueryStrOfOrder(const OrderInfoSPtr& orderInfo);

}  // namespace bq::td::svc::binance
