/*!
 * \file TDSvcOfBinanceConst.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "util/Pch.hpp"

namespace bq::td::svc::binance {

const static std::string EXCH_SIDE_BID = "BUY";
const static std::string EXCH_SIDE_ASK = "SELL";

const static std::string pathnameOfListenKeySpot = "/api/v3/userDataStream";
const static std::string pathnameOfListenKeyUBasedContracts =
    "/fapi/v1/listenKey";
const static std::string pathnameOfListenKeyCBasedContracts =
    "/dapi/v1/listenKey";

const static std::string pathnameOfAssetInfoSpot = "/api/v3/account";
const static std::string pathnameOfAssetInfoUBasedContracts =
    "/fapi/v2/balance";
const static std::string pathnameOfAssetInfoCBasedContracts =
    "/dapi/v1/balance";

const static std::string pathnameOfQueryOrderInfoSpot = "/api/v3/order";
const static std::string pathnameOfQueryOrderInfoUBasedContracts =
    "/fapi/v1/order";
const static std::string pathnameOfQueryOrderInfoCBasedContracts =
    "/dapi/v1/order";

const static std::string pathnameOfOrderSpot = "/api/v3/order";
const static std::string pathnameOfOrderUBasedContracts = "/fapi/v1/order";
const static std::string pathnameOfOrderCBasedContracts = "/dapi/v1/order";

const static std::string pathnameOfCancelOrder = "/api/v3/order";

}  // namespace bq::td::svc::binance
