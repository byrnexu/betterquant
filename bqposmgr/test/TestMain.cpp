#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "PosMgr.hpp"
#include "def/DataStruOfAssets.hpp"
#include "def/DataStruOfTD.hpp"
#include "util/Datetime.hpp"

using namespace bq;

class GlobalEvt : public testing::Environment {
 public:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

OrderInfoSPtr MakeBaseOrderInfo() {
  auto baseOrderInfo = std::make_shared<OrderInfo>();
  baseOrderInfo->userId_ = 1;
  baseOrderInfo->acctId_ = 2;
  baseOrderInfo->stgId_ = 3;
  baseOrderInfo->stgInstId_ = 4;
  baseOrderInfo->marketCode_ = MarketCode::Binance;
  return baseOrderInfo;
}

OrderInfoSPtr Make_S_U_Bid_0_01_PRICE_19999() {
  auto ret = MakeBaseOrderInfo();
  ret->symbolType_ = SymbolType::Perp;
  strncpy(ret->symbolCode_, "BTC-USDT-PERP", sizeof(ret->symbolCode_));
  strncpy(ret->exchSymbolCode_, "btcusdt", sizeof(ret->exchSymbolCode_));
  ret->side_ = Side::Bid;
  ret->posSide_ = PosSide::Both;
  ret->orderPrice_ = 20000;
  ret->orderSize_ = 0.01;
  ret->parValue_ = 0;
  ret->orderType_ = OrderType::Limit;
  ret->orderTypeExtra_ = OrderTypeExtra::Normal;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 0.01;
  ret->avgDealPrice_ = 19999;
  strncpy(ret->lastTradeId_, "10000", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 19999;
  ret->lastDealSize_ = 0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_U_Bid_0_02_PRICE_20999(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->orderPrice_ = 21000;
  ret->orderSize_ = 0.02;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000002;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 0.02;
  ret->avgDealPrice_ = 20999;
  strncpy(ret->lastTradeId_, "10001", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20999;
  ret->lastDealSize_ = 0.02;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_U_Ask_0_01_PRICE_20555(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Ask;
  ret->orderPrice_ = 20500;
  ret->orderSize_ = 0.01;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -0.01;
  ret->avgDealPrice_ = 20555;
  strncpy(ret->lastTradeId_, "10002", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20555;
  ret->lastDealSize_ = -0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_U_Ask_0_03_PRICE_20888(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Ask;
  ret->orderPrice_ = 20788;
  ret->orderSize_ = 0.03;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000003;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -0.03;
  ret->avgDealPrice_ = 20888;
  strncpy(ret->lastTradeId_, "10003", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20888;
  ret->lastDealSize_ = -0.03;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_U_Bid_0_01_PRICE_21888(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Bid;
  ret->orderPrice_ = 20788;
  ret->orderSize_ = 0.01;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 0.01;
  ret->avgDealPrice_ = 21888;
  strncpy(ret->lastTradeId_, "10003", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 21888;
  ret->lastDealSize_ = 0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

TEST(PosMgr, PosMgrSingleSideULong) {
  auto posMgr = std::make_shared<PosMgr>();
  posMgr->setSyncToDB(SyncToDB::False);

  auto ordSUBid1 = Make_S_U_Bid_0_01_PRICE_19999();
  posMgr->updateByOrderInfoFromTDGW(ordSUBid1);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Ask/Both/0/BTC fee=0; pos=0; prePos=0; avgOpenPrice=0; pnlUnReal=0; pnlReal=0
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Bid/Both/0/BTC fee=1e-06; pos=0.01; prePos=0; avgOpenPrice=19999; pnlUnReal=0; pnlReal=0)");

  auto ordSUBid2 = Make_S_U_Bid_0_02_PRICE_20999(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid2);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Ask/Both/0/BTC fee=0; pos=0; prePos=0; avgOpenPrice=0; pnlUnReal=0; pnlReal=0
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Bid/Both/0/BTC fee=3e-06; pos=0.03; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=0)");

  auto ordSUBid3 = Make_S_U_Ask_0_01_PRICE_20555(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid3);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Ask/Both/0/BTC fee=0; pos=0; prePos=0; avgOpenPrice=0; pnlUnReal=0; pnlReal=0
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Bid/Both/0/BTC fee=4e-06; pos=0.019999999999999997; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=-1.106666666666679)");

  auto ordSUBid4 = Make_S_U_Ask_0_03_PRICE_20888(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid4);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Ask/Both/0/BTC fee=3e-06; pos=-0.010000000000000002; prePos=0; avgOpenPrice=20888; pnlUnReal=0; pnlReal=0
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Bid/Both/0/BTC fee=4e-06; pos=0; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=3.3399999999999626)");

  auto ordSUBid5 = Make_S_U_Bid_0_01_PRICE_21888(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid5);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Ask/Both/0/BTC fee=4e-06; pos=0; prePos=0; avgOpenPrice=20888; pnlUnReal=0; pnlReal=-10
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Bid/Both/0/BTC fee=4e-06; pos=0; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=3.3399999999999626)");
}

OrderInfoSPtr Make_S_U_Ask_0_01_PRICE_19999() {
  auto ret = MakeBaseOrderInfo();
  ret->symbolType_ = SymbolType::Perp;
  strncpy(ret->symbolCode_, "BTC-USDT-PERP", sizeof(ret->symbolCode_));
  strncpy(ret->exchSymbolCode_, "btcusdt", sizeof(ret->exchSymbolCode_));
  ret->side_ = Side::Ask;
  ret->posSide_ = PosSide::Both;
  ret->orderPrice_ = 19998;
  ret->orderSize_ = 0.01;
  ret->parValue_ = 0;
  ret->orderType_ = OrderType::Limit;
  ret->orderTypeExtra_ = OrderTypeExtra::Normal;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -0.01;
  ret->avgDealPrice_ = 19999;
  strncpy(ret->lastTradeId_, "10000", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 19999;
  ret->lastDealSize_ = -0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_U_Ask_0_02_PRICE_20999(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->orderPrice_ = 20998;
  ret->orderSize_ = 0.02;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000002;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -0.02;
  ret->avgDealPrice_ = 20999;
  strncpy(ret->lastTradeId_, "10001", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20999;
  ret->lastDealSize_ = -0.02;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_U_Bid_0_01_PRICE_20555(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Bid;
  ret->orderPrice_ = 20600;
  ret->orderSize_ = 0.01;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 0.01;
  ret->avgDealPrice_ = 20555;
  strncpy(ret->lastTradeId_, "10002", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20555;
  ret->lastDealSize_ = 0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_U_Bid_0_03_PRICE_20888(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Bid;
  ret->orderPrice_ = 20988;
  ret->orderSize_ = 0.03;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000003;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 0.03;
  ret->avgDealPrice_ = 20888;
  strncpy(ret->lastTradeId_, "10003", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20888;
  ret->lastDealSize_ = 0.03;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_U_Ask_0_01_PRICE_21888(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Ask;
  ret->orderPrice_ = 20988;
  ret->orderSize_ = 0.01;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -0.01;
  ret->avgDealPrice_ = 20188;
  strncpy(ret->lastTradeId_, "10003", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 21888;
  ret->lastDealSize_ = -0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

TEST(PosMgr, PosMgrSingleSideUShort) {
  auto posMgr = std::make_shared<PosMgr>();
  posMgr->setSyncToDB(SyncToDB::False);

  auto ordSUAsk1 = Make_S_U_Ask_0_01_PRICE_19999();
  posMgr->updateByOrderInfoFromTDGW(ordSUAsk1);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Ask/Both/0/BTC fee=1e-06; pos=-0.01; prePos=0; avgOpenPrice=19999; pnlUnReal=0; pnlReal=0
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Bid/Both/0/BTC fee=0; pos=0; prePos=0; avgOpenPrice=0; pnlUnReal=0; pnlReal=0)");

  auto ordSUAsk2 = Make_S_U_Ask_0_02_PRICE_20999(ordSUAsk1);
  posMgr->updateByOrderInfoFromTDGW(ordSUAsk2);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Ask/Both/0/BTC fee=3e-06; pos=-0.03; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=0
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Bid/Both/0/BTC fee=0; pos=0; prePos=0; avgOpenPrice=0; pnlUnReal=0; pnlReal=0)");

  auto ordSUAsk3 = Make_S_U_Bid_0_01_PRICE_20555(ordSUAsk1);
  posMgr->updateByOrderInfoFromTDGW(ordSUAsk3);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Ask/Both/0/BTC fee=4e-06; pos=-0.019999999999999997; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=1.106666666666679
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Bid/Both/0/BTC fee=0; pos=0; prePos=0; avgOpenPrice=0; pnlUnReal=0; pnlReal=0)");

  auto ordSUAsk4 = Make_S_U_Bid_0_03_PRICE_20888(ordSUAsk1);
  posMgr->updateByOrderInfoFromTDGW(ordSUAsk4);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Ask/Both/0/BTC fee=4e-06; pos=0; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=-3.3399999999999626
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Bid/Both/0/BTC fee=3e-06; pos=0.010000000000000002; prePos=0; avgOpenPrice=20888; pnlUnReal=0; pnlReal=0)");

  auto ordSUAsk5 = Make_S_U_Ask_0_01_PRICE_21888(ordSUAsk1);
  posMgr->updateByOrderInfoFromTDGW(ordSUAsk5);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Ask/Both/0/BTC fee=4e-06; pos=0; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=-3.3399999999999626
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Bid/Both/0/BTC fee=4e-06; pos=0; prePos=0; avgOpenPrice=20888; pnlUnReal=0; pnlReal=10)");
}

OrderInfoSPtr Make_S_C_Bid_1_PRICE_20000() {
  auto ret = MakeBaseOrderInfo();
  ret->symbolType_ = SymbolType::CPerp;
  strncpy(ret->symbolCode_, "BTC-USDT-CPERP", sizeof(ret->symbolCode_));
  strncpy(ret->exchSymbolCode_, "btcusd_perp", sizeof(ret->exchSymbolCode_));
  ret->side_ = Side::Bid;
  ret->posSide_ = PosSide::Both;
  ret->orderPrice_ = 21000;
  ret->orderSize_ = 1;
  ret->parValue_ = 100;
  ret->orderType_ = OrderType::Limit;
  ret->orderTypeExtra_ = OrderTypeExtra::Normal;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 1;
  ret->avgDealPrice_ = 20000;
  strncpy(ret->lastTradeId_, "10000", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20000;
  ret->lastDealSize_ = 1;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_C_Bid_2_PRICE_21000(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->orderPrice_ = 21001;
  ret->orderSize_ = 2;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000002;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 2;
  ret->avgDealPrice_ = 21000;
  strncpy(ret->lastTradeId_, "10001", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 21000;
  ret->lastDealSize_ = 2;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_C_Ask_1_PRICE_20500(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Ask;
  ret->orderPrice_ = 20400;
  ret->orderSize_ = 1;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -1;
  ret->avgDealPrice_ = 20500;
  strncpy(ret->lastTradeId_, "10002", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20500;
  ret->lastDealSize_ = -1;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_C_Ask_3_PRICE_22000(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Ask;
  ret->orderPrice_ = 21888;
  ret->orderSize_ = 3;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000003;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -3;
  ret->avgDealPrice_ = 22000;
  strncpy(ret->lastTradeId_, "10003", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 22000;
  ret->lastDealSize_ = -3;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_C_Bid_1_PRICE_23000(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Bid;
  ret->orderPrice_ = 21888;
  ret->orderSize_ = 1;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 1;
  ret->avgDealPrice_ = 23000;
  strncpy(ret->lastTradeId_, "10003", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 23000;
  ret->lastDealSize_ = 1;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

TEST(PosMgr, PosMgrSingleSideCLong) {
  auto posMgr = std::make_shared<PosMgr>();
  posMgr->setSyncToDB(SyncToDB::False);

  auto ordSUBid1 = Make_S_C_Bid_1_PRICE_20000();
  posMgr->updateByOrderInfoFromTDGW(ordSUBid1);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Ask/Both/100/BTC fee=0; pos=0; prePos=0; avgOpenPrice=0; pnlUnReal=0; pnlReal=0
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Bid/Both/100/BTC fee=1e-06; pos=1; prePos=0; avgOpenPrice=20000; pnlUnReal=0; pnlReal=0)");

  auto ordSUBid2 = Make_S_C_Bid_2_PRICE_21000(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid2);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Ask/Both/100/BTC fee=0; pos=0; prePos=0; avgOpenPrice=0; pnlUnReal=0; pnlReal=0
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Bid/Both/100/BTC fee=3e-06; pos=3; prePos=0; avgOpenPrice=20655.737704918032; pnlUnReal=0; pnlReal=0)");

  auto ordSUBid3 = Make_S_C_Ask_1_PRICE_20500(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid3);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Ask/Both/100/BTC fee=0; pos=0; prePos=0; avgOpenPrice=0; pnlUnReal=0; pnlReal=0
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Bid/Both/100/BTC fee=4e-06; pos=2; prePos=0; avgOpenPrice=20655.737704918032; pnlUnReal=0; pnlReal=-3.6778939217963933e-05)");

  auto ordSUBid4 = Make_S_C_Ask_3_PRICE_22000(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid4);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Ask/Both/100/BTC fee=3e-06; pos=-1; prePos=0; avgOpenPrice=22000; pnlUnReal=0; pnlReal=0
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Bid/Both/100/BTC fee=4e-06; pos=0; prePos=0; avgOpenPrice=20655.737704918032; pnlUnReal=0; pnlReal=0.0005548516524126281)");

  auto ordSUBid5 = Make_S_C_Bid_1_PRICE_23000(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid5);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Ask/Both/100/BTC fee=4e-06; pos=0; prePos=0; avgOpenPrice=22000; pnlUnReal=0; pnlReal=-0.00019762845849802314
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Bid/Both/100/BTC fee=4e-06; pos=0; prePos=0; avgOpenPrice=20655.737704918032; pnlUnReal=0; pnlReal=0.0005548516524126281)");
}

OrderInfoSPtr Make_S_C_Ask_1_PRICE_20000() {
  auto ret = MakeBaseOrderInfo();
  ret->symbolType_ = SymbolType::CPerp;
  strncpy(ret->symbolCode_, "BTC-USDT-CPERP", sizeof(ret->symbolCode_));
  strncpy(ret->exchSymbolCode_, "btcusd_perp", sizeof(ret->exchSymbolCode_));
  ret->side_ = Side::Ask;
  ret->posSide_ = PosSide::Both;
  ret->orderPrice_ = 20000;
  ret->orderSize_ = 1;
  ret->parValue_ = 100;
  ret->orderType_ = OrderType::Limit;
  ret->orderTypeExtra_ = OrderTypeExtra::Normal;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -1;
  ret->avgDealPrice_ = 20000;
  strncpy(ret->lastTradeId_, "10000", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20000;
  ret->lastDealSize_ = -1;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_C_Ask_2_PRICE_21000(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->orderPrice_ = 20998;
  ret->orderSize_ = 2;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000002;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -2;
  ret->avgDealPrice_ = 21000;
  strncpy(ret->lastTradeId_, "10001", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 21000;
  ret->lastDealSize_ = -2;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_C_Bid_1_PRICE_20500(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Bid;
  ret->orderPrice_ = 20500;
  ret->orderSize_ = 1;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 1;
  ret->avgDealPrice_ = 20500;
  strncpy(ret->lastTradeId_, "10002", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20500;
  ret->lastDealSize_ = 1;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_C_Bid_3_PRICE_22000(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Bid;
  ret->orderPrice_ = 22000;
  ret->orderSize_ = 3;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000003;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 3;
  ret->avgDealPrice_ = 22000;
  strncpy(ret->lastTradeId_, "10003", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 22000;
  ret->lastDealSize_ = 3;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_C_Ask_1_PRICE_23000(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Ask;
  ret->orderPrice_ = 22000;
  ret->orderSize_ = 1;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -1;
  ret->avgDealPrice_ = 23000;
  strncpy(ret->lastTradeId_, "10003", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 23000;
  ret->lastDealSize_ = -1;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

TEST(PosMgr, PosMgrSingleSideCShort) {
  auto posMgr = std::make_shared<PosMgr>();
  posMgr->setSyncToDB(SyncToDB::False);

  auto ordSUAsk1 = Make_S_C_Ask_1_PRICE_20000();
  posMgr->updateByOrderInfoFromTDGW(ordSUAsk1);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Ask/Both/100/BTC fee=1e-06; pos=-1; prePos=0; avgOpenPrice=20000; pnlUnReal=0; pnlReal=0
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Bid/Both/100/BTC fee=0; pos=0; prePos=0; avgOpenPrice=0; pnlUnReal=0; pnlReal=0)");

  auto ordSUAsk2 = Make_S_C_Ask_2_PRICE_21000(ordSUAsk1);
  posMgr->updateByOrderInfoFromTDGW(ordSUAsk2);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Ask/Both/100/BTC fee=3e-06; pos=-3; prePos=0; avgOpenPrice=20655.737704918032; pnlUnReal=0; pnlReal=0
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Bid/Both/100/BTC fee=0; pos=0; prePos=0; avgOpenPrice=0; pnlUnReal=0; pnlReal=0)");

  auto ordSUAsk3 = Make_S_C_Bid_1_PRICE_20500(ordSUAsk1);
  posMgr->updateByOrderInfoFromTDGW(ordSUAsk3);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Ask/Both/100/BTC fee=4e-06; pos=-2; prePos=0; avgOpenPrice=20655.737704918032; pnlUnReal=0; pnlReal=3.6778939217963933e-05
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Bid/Both/100/BTC fee=0; pos=0; prePos=0; avgOpenPrice=0; pnlUnReal=0; pnlReal=0)");

  auto ordSUAsk4 = Make_S_C_Bid_3_PRICE_22000(ordSUAsk1);
  posMgr->updateByOrderInfoFromTDGW(ordSUAsk4);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Ask/Both/100/BTC fee=4e-06; pos=0; prePos=0; avgOpenPrice=20655.737704918032; pnlUnReal=0; pnlReal=-0.0005548516524126281
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Bid/Both/100/BTC fee=3e-06; pos=1; prePos=0; avgOpenPrice=22000; pnlUnReal=0; pnlReal=0)");

  auto ordSUAsk5 = Make_S_C_Ask_1_PRICE_23000(ordSUAsk1);
  posMgr->updateByOrderInfoFromTDGW(ordSUAsk5);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Ask/Both/100/BTC fee=4e-06; pos=0; prePos=0; avgOpenPrice=20655.737704918032; pnlUnReal=0; pnlReal=-0.0005548516524126281
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Bid/Both/100/BTC fee=4e-06; pos=0; prePos=0; avgOpenPrice=22000; pnlUnReal=0; pnlReal=0.00019762845849802314)");
}

OrderInfoSPtr Make_D_U_Bid_L_0_01_PRICE_19999() {
  auto ret = MakeBaseOrderInfo();
  ret->symbolType_ = SymbolType::Perp;
  strncpy(ret->symbolCode_, "BTC-USDT-PERP", sizeof(ret->symbolCode_));
  strncpy(ret->exchSymbolCode_, "btcusdt", sizeof(ret->exchSymbolCode_));
  ret->side_ = Side::Bid;
  ret->posSide_ = PosSide::Long;
  ret->orderPrice_ = 20000;
  ret->orderSize_ = 0.01;
  ret->parValue_ = 0;
  ret->orderType_ = OrderType::Limit;
  ret->orderTypeExtra_ = OrderTypeExtra::Normal;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 0.01;
  ret->avgDealPrice_ = 19999;
  strncpy(ret->lastTradeId_, "10000", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 19999;
  ret->lastDealSize_ = 0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_D_U_Bid_L_0_02_PRICE_20999(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->orderPrice_ = 21000;
  ret->orderSize_ = 0.02;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000002;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 0.02;
  ret->avgDealPrice_ = 20999;
  strncpy(ret->lastTradeId_, "10001", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20999;
  ret->lastDealSize_ = 0.02;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_D_U_Ask_L_0_01_PRICE_20555(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Ask;
  ret->posSide_ = PosSide::Long;
  ret->orderPrice_ = 20500;
  ret->orderSize_ = 0.01;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -0.01;
  ret->avgDealPrice_ = 20555;
  strncpy(ret->lastTradeId_, "10002", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20555;
  ret->lastDealSize_ = -0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_D_U_Ask_L_0_02_PRICE_20555(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Ask;
  ret->posSide_ = PosSide::Long;
  ret->orderPrice_ = 20500;
  ret->orderSize_ = 0.02;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000002;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -0.02;
  ret->avgDealPrice_ = 20555;
  strncpy(ret->lastTradeId_, "10002", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20555;
  ret->lastDealSize_ = -0.02;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

TEST(PosMgr, PosMgrDoubleSideULong) {
  auto posMgr = std::make_shared<PosMgr>();
  posMgr->setSyncToDB(SyncToDB::False);

  auto ordSUBid1 = Make_D_U_Bid_L_0_01_PRICE_19999();
  posMgr->updateByOrderInfoFromTDGW(ordSUBid1);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Bid/Long/0/BTC fee=1e-06; pos=0.01; prePos=0; avgOpenPrice=19999; pnlUnReal=0; pnlReal=0)");

  auto ordSUBid2 = Make_D_U_Bid_L_0_02_PRICE_20999(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid2);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Bid/Long/0/BTC fee=3e-06; pos=0.03; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=0)");

  auto ordSUBid3 = Make_D_U_Ask_L_0_01_PRICE_20555(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid3);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Bid/Long/0/BTC fee=4e-06; pos=0.019999999999999997; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=-1.106666666666679)");

  auto ordSUBid4 = Make_D_U_Ask_L_0_02_PRICE_20555(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid4);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Bid/Long/0/BTC fee=6e-06; pos=0; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=-3.3200000000000367)");
}

OrderInfoSPtr Make_D_U_Ask_S_0_01_PRICE_19999() {
  auto ret = MakeBaseOrderInfo();
  ret->symbolType_ = SymbolType::Perp;
  strncpy(ret->symbolCode_, "BTC-USDT-PERP", sizeof(ret->symbolCode_));
  strncpy(ret->exchSymbolCode_, "btcusdt", sizeof(ret->exchSymbolCode_));
  ret->side_ = Side::Ask;
  ret->posSide_ = PosSide::Short;
  ret->orderPrice_ = 20000;
  ret->orderSize_ = 0.01;
  ret->parValue_ = 0;
  ret->orderType_ = OrderType::Limit;
  ret->orderTypeExtra_ = OrderTypeExtra::Normal;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -0.01;
  ret->avgDealPrice_ = 19999;
  strncpy(ret->lastTradeId_, "10000", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 19999;
  ret->lastDealSize_ = -0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_D_U_Ask_S_0_02_PRICE_20999(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->orderPrice_ = 21000;
  ret->orderSize_ = 0.02;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000002;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -0.02;
  ret->avgDealPrice_ = 20999;
  strncpy(ret->lastTradeId_, "10001", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20999;
  ret->lastDealSize_ = -0.02;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_D_U_Bid_S_0_01_PRICE_20555(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Bid;
  ret->posSide_ = PosSide::Short;
  ret->orderPrice_ = 20500;
  ret->orderSize_ = 0.01;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 0.01;
  ret->avgDealPrice_ = 20555;
  strncpy(ret->lastTradeId_, "10002", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20555;
  ret->lastDealSize_ = 0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_D_U_Bid_S_0_02_PRICE_20555(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Bid;
  ret->posSide_ = PosSide::Short;
  ret->orderPrice_ = 20500;
  ret->orderSize_ = 0.02;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000002;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 0.02;
  ret->avgDealPrice_ = 20555;
  strncpy(ret->lastTradeId_, "10002", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20555;
  ret->lastDealSize_ = 0.02;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

TEST(PosMgr, PosMgrDoubleSideUShort) {
  auto posMgr = std::make_shared<PosMgr>();
  posMgr->setSyncToDB(SyncToDB::False);

  auto ordSUBid1 = Make_D_U_Ask_S_0_01_PRICE_19999();
  posMgr->updateByOrderInfoFromTDGW(ordSUBid1);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Ask/Short/0/BTC fee=1e-06; pos=-0.01; prePos=0; avgOpenPrice=19999; pnlUnReal=0; pnlReal=0)");

  auto ordSUBid2 = Make_D_U_Ask_S_0_02_PRICE_20999(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid2);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Ask/Short/0/BTC fee=3e-06; pos=-0.03; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=0)");

  auto ordSUBid3 = Make_D_U_Bid_S_0_01_PRICE_20555(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid3);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Ask/Short/0/BTC fee=4e-06; pos=-0.019999999999999997; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=1.106666666666679)");

  auto ordSUBid4 = Make_D_U_Bid_S_0_02_PRICE_20555(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid4);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Perp/BTC-USDT-PERP/Ask/Short/0/BTC fee=6e-06; pos=0; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=3.3200000000000367)");
}

OrderInfoSPtr Make_D_C_Bid_L_1_PRICE_19999() {
  auto ret = MakeBaseOrderInfo();
  ret->symbolType_ = SymbolType::CPerp;
  strncpy(ret->symbolCode_, "BTC-USDT-CPERP", sizeof(ret->symbolCode_));
  strncpy(ret->exchSymbolCode_, "btcusd_perp", sizeof(ret->exchSymbolCode_));
  ret->side_ = Side::Bid;
  ret->posSide_ = PosSide::Long;
  ret->orderPrice_ = 20000;
  ret->orderSize_ = 1;
  ret->parValue_ = 100;
  ret->orderType_ = OrderType::Limit;
  ret->orderTypeExtra_ = OrderTypeExtra::Normal;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 1;
  ret->avgDealPrice_ = 19999;
  strncpy(ret->lastTradeId_, "10000", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 19999;
  ret->lastDealSize_ = 1;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_D_C_Bid_L_2_PRICE_20999(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->orderPrice_ = 21000;
  ret->orderSize_ = 2;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000002;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 2;
  ret->avgDealPrice_ = 20999;
  strncpy(ret->lastTradeId_, "10001", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20999;
  ret->lastDealSize_ = 2;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_D_C_Ask_L_1_PRICE_20555(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Ask;
  ret->posSide_ = PosSide::Long;
  ret->orderPrice_ = 20500;
  ret->orderSize_ = 1;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -1;
  ret->avgDealPrice_ = 20555;
  strncpy(ret->lastTradeId_, "10002", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20555;
  ret->lastDealSize_ = -1;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_D_C_Ask_L_2_PRICE_20555(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Ask;
  ret->posSide_ = PosSide::Long;
  ret->orderPrice_ = 20500;
  ret->orderSize_ = 2;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000002;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -2;
  ret->avgDealPrice_ = 20555;
  strncpy(ret->lastTradeId_, "10002", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20555;
  ret->lastDealSize_ = -2;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

TEST(PosMgr, PosMgrDoubleSideCLong) {
  auto posMgr = std::make_shared<PosMgr>();
  posMgr->setSyncToDB(SyncToDB::False);

  auto ordSUBid1 = Make_D_C_Bid_L_1_PRICE_19999();
  posMgr->updateByOrderInfoFromTDGW(ordSUBid1);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Bid/Long/100/BTC fee=1e-06; pos=1; prePos=0; avgOpenPrice=19999; pnlUnReal=0; pnlReal=0)");

  auto ordSUBid2 = Make_D_C_Bid_L_2_PRICE_20999(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid2);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Bid/Long/100/BTC fee=3e-06; pos=3; prePos=0; avgOpenPrice=20654.737167401676; pnlUnReal=0; pnlReal=0)");

  auto ordSUBid3 = Make_D_C_Ask_L_1_PRICE_20555(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid3);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Bid/Long/100/BTC fee=4e-06; pos=2; prePos=0; avgOpenPrice=20654.737167401676; pnlUnReal=0; pnlReal=-2.3491993703954783e-05)");

  auto ordSUBid4 = Make_D_C_Ask_L_2_PRICE_20555(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid4);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Bid/Long/100/BTC fee=6e-06; pos=0; prePos=0; avgOpenPrice=20654.737167401676; pnlUnReal=0; pnlReal=-7.047598111186435e-05)");
}

OrderInfoSPtr Make_D_C_Ask_S_1_PRICE_19999() {
  auto ret = MakeBaseOrderInfo();
  ret->symbolType_ = SymbolType::CPerp;
  strncpy(ret->symbolCode_, "BTC-USDT-CPERP", sizeof(ret->symbolCode_));
  strncpy(ret->exchSymbolCode_, "btcusd_perp", sizeof(ret->exchSymbolCode_));
  ret->side_ = Side::Ask;
  ret->posSide_ = PosSide::Short;
  ret->orderPrice_ = 20000;
  ret->orderSize_ = 1;
  ret->parValue_ = 100;
  ret->orderType_ = OrderType::Limit;
  ret->orderTypeExtra_ = OrderTypeExtra::Normal;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -1;
  ret->avgDealPrice_ = 19999;
  strncpy(ret->lastTradeId_, "10000", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 19999;
  ret->lastDealSize_ = -1;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_D_C_Ask_S_2_PRICE_20999(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->orderPrice_ = 21000;
  ret->orderSize_ = 2;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000002;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -2;
  ret->avgDealPrice_ = 20999;
  strncpy(ret->lastTradeId_, "10001", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20999;
  ret->lastDealSize_ = -2;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_D_C_Bid_S_1_PRICE_20555(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Bid;
  ret->posSide_ = PosSide::Short;
  ret->orderPrice_ = 20500;
  ret->orderSize_ = 1;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 1;
  ret->avgDealPrice_ = 20555;
  strncpy(ret->lastTradeId_, "10002", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20555;
  ret->lastDealSize_ = 1;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_D_C_Bid_S_2_PRICE_20555(const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Bid;
  ret->posSide_ = PosSide::Short;
  ret->orderPrice_ = 20500;
  ret->orderSize_ = 2;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000002;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 2;
  ret->avgDealPrice_ = 20555;
  strncpy(ret->lastTradeId_, "10002", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20555;
  ret->lastDealSize_ = 2;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

TEST(PosMgr, PosMgrDoubleSideCShort) {
  auto posMgr = std::make_shared<PosMgr>();
  posMgr->setSyncToDB(SyncToDB::False);

  auto ordSUBid1 = Make_D_C_Ask_S_1_PRICE_19999();
  posMgr->updateByOrderInfoFromTDGW(ordSUBid1);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Ask/Short/100/BTC fee=1e-06; pos=-1; prePos=0; avgOpenPrice=19999; pnlUnReal=0; pnlReal=0)");

  auto ordSUBid2 = Make_D_C_Ask_S_2_PRICE_20999(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid2);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Ask/Short/100/BTC fee=3e-06; pos=-3; prePos=0; avgOpenPrice=20654.737167401676; pnlUnReal=0; pnlReal=0)");

  auto ordSUBid3 = Make_D_C_Bid_S_1_PRICE_20555(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid3);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Ask/Short/100/BTC fee=4e-06; pos=-2; prePos=0; avgOpenPrice=20654.737167401676; pnlUnReal=0; pnlReal=2.3491993703954783e-05)");

  auto ordSUBid4 = Make_D_C_Bid_S_2_PRICE_20555(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid4);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/CPerp/BTC-USDT-CPERP/Ask/Short/100/BTC fee=6e-06; pos=0; prePos=0; avgOpenPrice=20654.737167401676; pnlUnReal=0; pnlReal=7.047598111186435e-05)");
}

OrderInfoSPtr Make_S_U_Bid_0_01_SPOT_PRICE_19999() {
  auto ret = MakeBaseOrderInfo();
  ret->symbolType_ = SymbolType::Spot;
  strncpy(ret->symbolCode_, "BTC-USDT", sizeof(ret->symbolCode_));
  strncpy(ret->exchSymbolCode_, "btcusdt", sizeof(ret->exchSymbolCode_));
  ret->side_ = Side::Bid;
  ret->posSide_ = PosSide::Both;
  ret->orderPrice_ = 20000;
  ret->orderSize_ = 0.01;
  ret->parValue_ = 0;
  ret->orderType_ = OrderType::Limit;
  ret->orderTypeExtra_ = OrderTypeExtra::Normal;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 0.01;
  ret->avgDealPrice_ = 19999;
  strncpy(ret->lastTradeId_, "10000", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 19999;
  ret->lastDealSize_ = 0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_U_Bid_0_02_SPOT_PRICE_20999(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->orderPrice_ = 21000;
  ret->orderSize_ = 0.02;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000002;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 0.02;
  ret->avgDealPrice_ = 20999;
  strncpy(ret->lastTradeId_, "10001", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20999;
  ret->lastDealSize_ = 0.02;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_U_Ask_0_01_SPOT_PRICE_20555(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Ask;
  ret->orderPrice_ = 20500;
  ret->orderSize_ = 0.01;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -0.01;
  ret->avgDealPrice_ = 20555;
  strncpy(ret->lastTradeId_, "10002", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20555;
  ret->lastDealSize_ = -0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_U_Ask_0_03_SPOT_PRICE_20888(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Ask;
  ret->orderPrice_ = 20788;
  ret->orderSize_ = 0.03;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000003;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -0.03;
  ret->avgDealPrice_ = 20888;
  strncpy(ret->lastTradeId_, "10003", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20888;
  ret->lastDealSize_ = -0.03;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_U_Bid_0_01_SPOT_PRICE_21888(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Bid;
  ret->orderPrice_ = 20788;
  ret->orderSize_ = 0.01;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 0.01;
  ret->avgDealPrice_ = 21888;
  strncpy(ret->lastTradeId_, "10003", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 21888;
  ret->lastDealSize_ = 0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

TEST(PosMgr, PosMgrSingleSideULongSpot) {
  auto posMgr = std::make_shared<PosMgr>();
  posMgr->setSyncToDB(SyncToDB::False);

  auto ordSUBid1 = Make_S_U_Bid_0_01_SPOT_PRICE_19999();
  posMgr->updateByOrderInfoFromTDGW(ordSUBid1);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Spot/BTC-USDT/Ask/Both/0/BTC fee=0; pos=0; prePos=0; avgOpenPrice=0; pnlUnReal=0; pnlReal=0
1/2/3/4/Binance/Spot/BTC-USDT/Bid/Both/0/BTC fee=1e-06; pos=0.01; prePos=0; avgOpenPrice=19999; pnlUnReal=0; pnlReal=0)");

  auto ordSUBid2 = Make_S_U_Bid_0_02_SPOT_PRICE_20999(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid2);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Spot/BTC-USDT/Ask/Both/0/BTC fee=0; pos=0; prePos=0; avgOpenPrice=0; pnlUnReal=0; pnlReal=0
1/2/3/4/Binance/Spot/BTC-USDT/Bid/Both/0/BTC fee=3e-06; pos=0.03; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=0)");

  auto ordSUBid3 = Make_S_U_Ask_0_01_SPOT_PRICE_20555(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid3);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Spot/BTC-USDT/Ask/Both/0/BTC fee=0; pos=0; prePos=0; avgOpenPrice=0; pnlUnReal=0; pnlReal=0
1/2/3/4/Binance/Spot/BTC-USDT/Bid/Both/0/BTC fee=4e-06; pos=0.019999999999999997; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=-1.106666666666679)");

  auto ordSUBid4 = Make_S_U_Ask_0_03_SPOT_PRICE_20888(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid4);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Spot/BTC-USDT/Ask/Both/0/BTC fee=3e-06; pos=-0.010000000000000002; prePos=0; avgOpenPrice=20888; pnlUnReal=0; pnlReal=0
1/2/3/4/Binance/Spot/BTC-USDT/Bid/Both/0/BTC fee=4e-06; pos=0; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=3.3399999999999626)");

  auto ordSUBid5 = Make_S_U_Bid_0_01_SPOT_PRICE_21888(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid5);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Spot/BTC-USDT/Ask/Both/0/BTC fee=4e-06; pos=0; prePos=0; avgOpenPrice=20888; pnlUnReal=0; pnlReal=-10
1/2/3/4/Binance/Spot/BTC-USDT/Bid/Both/0/BTC fee=4e-06; pos=0; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=3.3399999999999626)");
}

OrderInfoSPtr Make_S_U_Ask_0_01_SPOT_PRICE_19999() {
  auto ret = MakeBaseOrderInfo();
  ret->symbolType_ = SymbolType::Spot;
  strncpy(ret->symbolCode_, "BTC-USDT", sizeof(ret->symbolCode_));
  strncpy(ret->exchSymbolCode_, "btcusdt", sizeof(ret->exchSymbolCode_));
  ret->side_ = Side::Ask;
  ret->posSide_ = PosSide::Both;
  ret->orderPrice_ = 19998;
  ret->orderSize_ = 0.01;
  ret->parValue_ = 0;
  ret->orderType_ = OrderType::Limit;
  ret->orderTypeExtra_ = OrderTypeExtra::Normal;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -0.01;
  ret->avgDealPrice_ = 19999;
  strncpy(ret->lastTradeId_, "10000", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 19999;
  ret->lastDealSize_ = -0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_U_Ask_0_02_SPOT_PRICE_20999(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->orderPrice_ = 20998;
  ret->orderSize_ = 0.02;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000002;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -0.02;
  ret->avgDealPrice_ = 20999;
  strncpy(ret->lastTradeId_, "10001", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20999;
  ret->lastDealSize_ = -0.02;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_U_Bid_0_01_SPOT_PRICE_20555(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Bid;
  ret->orderPrice_ = 20600;
  ret->orderSize_ = 0.01;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 0.01;
  ret->avgDealPrice_ = 20555;
  strncpy(ret->lastTradeId_, "10002", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20555;
  ret->lastDealSize_ = 0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_U_Bid_0_03_SPOT_PRICE_20888(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Bid;
  ret->orderPrice_ = 20988;
  ret->orderSize_ = 0.03;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000003;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 0.03;
  ret->avgDealPrice_ = 20888;
  strncpy(ret->lastTradeId_, "10003", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20888;
  ret->lastDealSize_ = 0.03;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_S_U_Ask_0_01_SPOT_PRICE_21888(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Ask;
  ret->orderPrice_ = 20988;
  ret->orderSize_ = 0.01;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -0.01;
  ret->avgDealPrice_ = 20188;
  strncpy(ret->lastTradeId_, "10003", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 21888;
  ret->lastDealSize_ = -0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

TEST(PosMgr, PosMgrSingleSideUShortSpot) {
  auto posMgr = std::make_shared<PosMgr>();
  posMgr->setSyncToDB(SyncToDB::False);

  auto ordSUAsk1 = Make_S_U_Ask_0_01_SPOT_PRICE_19999();
  posMgr->updateByOrderInfoFromTDGW(ordSUAsk1);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Spot/BTC-USDT/Ask/Both/0/BTC fee=1e-06; pos=-0.01; prePos=0; avgOpenPrice=19999; pnlUnReal=0; pnlReal=0
1/2/3/4/Binance/Spot/BTC-USDT/Bid/Both/0/BTC fee=0; pos=0; prePos=0; avgOpenPrice=0; pnlUnReal=0; pnlReal=0)");

  auto ordSUAsk2 = Make_S_U_Ask_0_02_SPOT_PRICE_20999(ordSUAsk1);
  posMgr->updateByOrderInfoFromTDGW(ordSUAsk2);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Spot/BTC-USDT/Ask/Both/0/BTC fee=3e-06; pos=-0.03; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=0
1/2/3/4/Binance/Spot/BTC-USDT/Bid/Both/0/BTC fee=0; pos=0; prePos=0; avgOpenPrice=0; pnlUnReal=0; pnlReal=0)");

  auto ordSUAsk3 = Make_S_U_Bid_0_01_SPOT_PRICE_20555(ordSUAsk1);
  posMgr->updateByOrderInfoFromTDGW(ordSUAsk3);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Spot/BTC-USDT/Ask/Both/0/BTC fee=4e-06; pos=-0.019999999999999997; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=1.106666666666679
1/2/3/4/Binance/Spot/BTC-USDT/Bid/Both/0/BTC fee=0; pos=0; prePos=0; avgOpenPrice=0; pnlUnReal=0; pnlReal=0)");

  auto ordSUAsk4 = Make_S_U_Bid_0_03_SPOT_PRICE_20888(ordSUAsk1);
  posMgr->updateByOrderInfoFromTDGW(ordSUAsk4);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Spot/BTC-USDT/Ask/Both/0/BTC fee=4e-06; pos=0; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=-3.3399999999999626
1/2/3/4/Binance/Spot/BTC-USDT/Bid/Both/0/BTC fee=3e-06; pos=0.010000000000000002; prePos=0; avgOpenPrice=20888; pnlUnReal=0; pnlReal=0)");

  auto ordSUAsk5 = Make_S_U_Ask_0_01_SPOT_PRICE_21888(ordSUAsk1);
  posMgr->updateByOrderInfoFromTDGW(ordSUAsk5);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Spot/BTC-USDT/Ask/Both/0/BTC fee=4e-06; pos=0; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=-3.3399999999999626
1/2/3/4/Binance/Spot/BTC-USDT/Bid/Both/0/BTC fee=4e-06; pos=0; prePos=0; avgOpenPrice=20888; pnlUnReal=0; pnlReal=10)");
}

OrderInfoSPtr Make_D_U_Bid_L_0_01_SPOT_PRICE_19999() {
  auto ret = MakeBaseOrderInfo();
  ret->symbolType_ = SymbolType::Spot;
  strncpy(ret->symbolCode_, "BTC-USDT", sizeof(ret->symbolCode_));
  strncpy(ret->exchSymbolCode_, "btcusdt", sizeof(ret->exchSymbolCode_));
  ret->side_ = Side::Bid;
  ret->posSide_ = PosSide::Long;
  ret->orderPrice_ = 20000;
  ret->orderSize_ = 0.01;
  ret->parValue_ = 0;
  ret->orderType_ = OrderType::Limit;
  ret->orderTypeExtra_ = OrderTypeExtra::Normal;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 0.01;
  ret->avgDealPrice_ = 19999;
  strncpy(ret->lastTradeId_, "10000", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 19999;
  ret->lastDealSize_ = 0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_D_U_Bid_L_0_02_SPOT_PRICE_20999(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->orderPrice_ = 21000;
  ret->orderSize_ = 0.02;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000002;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 0.02;
  ret->avgDealPrice_ = 20999;
  strncpy(ret->lastTradeId_, "10001", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20999;
  ret->lastDealSize_ = 0.02;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_D_U_Ask_L_0_01_SPOT_PRICE_20555(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Ask;
  ret->posSide_ = PosSide::Long;
  ret->orderPrice_ = 20500;
  ret->orderSize_ = 0.01;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -0.01;
  ret->avgDealPrice_ = 20555;
  strncpy(ret->lastTradeId_, "10002", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20555;
  ret->lastDealSize_ = -0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_D_U_Ask_L_0_02_SPOT_PRICE_20555(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Ask;
  ret->posSide_ = PosSide::Long;
  ret->orderPrice_ = 20500;
  ret->orderSize_ = 0.02;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000002;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -0.02;
  ret->avgDealPrice_ = 20555;
  strncpy(ret->lastTradeId_, "10002", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20555;
  ret->lastDealSize_ = -0.02;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

TEST(PosMgr, PosMgrDoubleSideULongSpot) {
  auto posMgr = std::make_shared<PosMgr>();
  posMgr->setSyncToDB(SyncToDB::False);

  auto ordSUBid1 = Make_D_U_Bid_L_0_01_SPOT_PRICE_19999();
  posMgr->updateByOrderInfoFromTDGW(ordSUBid1);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Spot/BTC-USDT/Bid/Long/0/BTC fee=1e-06; pos=0.01; prePos=0; avgOpenPrice=19999; pnlUnReal=0; pnlReal=0)");

  auto ordSUBid2 = Make_D_U_Bid_L_0_02_SPOT_PRICE_20999(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid2);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Spot/BTC-USDT/Bid/Long/0/BTC fee=3e-06; pos=0.03; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=0)");

  auto ordSUBid3 = Make_D_U_Ask_L_0_01_SPOT_PRICE_20555(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid3);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Spot/BTC-USDT/Bid/Long/0/BTC fee=4e-06; pos=0.019999999999999997; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=-1.106666666666679)");

  auto ordSUBid4 = Make_D_U_Ask_L_0_02_SPOT_PRICE_20555(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid4);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Spot/BTC-USDT/Bid/Long/0/BTC fee=6e-06; pos=0; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=-3.3200000000000367)");
}

OrderInfoSPtr Make_D_U_Ask_S_0_01_SPOT_PRICE_19999() {
  auto ret = MakeBaseOrderInfo();
  ret->symbolType_ = SymbolType::Spot;
  strncpy(ret->symbolCode_, "BTC-USDT", sizeof(ret->symbolCode_));
  strncpy(ret->exchSymbolCode_, "btcusdt", sizeof(ret->exchSymbolCode_));
  ret->side_ = Side::Ask;
  ret->posSide_ = PosSide::Short;
  ret->orderPrice_ = 20000;
  ret->orderSize_ = 0.01;
  ret->parValue_ = 0;
  ret->orderType_ = OrderType::Limit;
  ret->orderTypeExtra_ = OrderTypeExtra::Normal;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -0.01;
  ret->avgDealPrice_ = 19999;
  strncpy(ret->lastTradeId_, "10000", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 19999;
  ret->lastDealSize_ = -0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_D_U_Ask_S_0_02_SPOT_PRICE_20999(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->orderPrice_ = 21000;
  ret->orderSize_ = 0.02;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000002;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = -0.02;
  ret->avgDealPrice_ = 20999;
  strncpy(ret->lastTradeId_, "10001", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20999;
  ret->lastDealSize_ = -0.02;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_D_U_Bid_S_0_01_SPOT_PRICE_20555(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Bid;
  ret->posSide_ = PosSide::Short;
  ret->orderPrice_ = 20500;
  ret->orderSize_ = 0.01;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000001;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 0.01;
  ret->avgDealPrice_ = 20555;
  strncpy(ret->lastTradeId_, "10002", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20555;
  ret->lastDealSize_ = 0.01;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

OrderInfoSPtr Make_D_U_Bid_S_0_02_SPOT_PRICE_20555(
    const OrderInfoSPtr& orderInfo) {
  auto ret = std::make_shared<OrderInfo>(*orderInfo);
  ret->side_ = Side::Bid;
  ret->posSide_ = PosSide::Short;
  ret->orderPrice_ = 20500;
  ret->orderSize_ = 0.02;
  ret->orderTime_ = GetTotalUSSince1970() - 10000000;
  ret->fee_ = 0.000002;
  strncpy(ret->feeCurrency_, "BTC", sizeof(ret->feeCurrency_));
  ret->dealSize_ = 0.02;
  ret->avgDealPrice_ = 20555;
  strncpy(ret->lastTradeId_, "10002", sizeof(ret->lastTradeId_));
  ret->lastDealPrice_ = 20555;
  ret->lastDealSize_ = 0.02;
  ret->lastDealTime_ = GetTotalUSSince1970();
  ret->orderStatus_ = OrderStatus::Filled;
  return ret;
}

TEST(PosMgr, PosMgrDoubleSideUShortSpot) {
  auto posMgr = std::make_shared<PosMgr>();
  posMgr->setSyncToDB(SyncToDB::False);

  auto ordSUBid1 = Make_D_U_Ask_S_0_01_SPOT_PRICE_19999();
  posMgr->updateByOrderInfoFromTDGW(ordSUBid1);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Spot/BTC-USDT/Ask/Short/0/BTC fee=1e-06; pos=-0.01; prePos=0; avgOpenPrice=19999; pnlUnReal=0; pnlReal=0)");

  auto ordSUBid2 = Make_D_U_Ask_S_0_02_SPOT_PRICE_20999(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid2);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Spot/BTC-USDT/Ask/Short/0/BTC fee=3e-06; pos=-0.03; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=0)");

  auto ordSUBid3 = Make_D_U_Bid_S_0_01_SPOT_PRICE_20555(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid3);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Spot/BTC-USDT/Ask/Short/0/BTC fee=4e-06; pos=-0.019999999999999997; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=1.106666666666679)");

  auto ordSUBid4 = Make_D_U_Bid_S_0_02_SPOT_PRICE_20555(ordSUBid1);
  posMgr->updateByOrderInfoFromTDGW(ordSUBid4);
  EXPECT_TRUE(posMgr->toStr() == R"(
1/2/3/4/Binance/Spot/BTC-USDT/Ask/Short/0/BTC fee=6e-06; pos=0; prePos=0; avgOpenPrice=20665.666666666668; pnlUnReal=0; pnlReal=3.3200000000000367)");
}

int main(int argc, char** argv) {
  testing::AddGlobalTestEnvironment(new GlobalEvt);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
