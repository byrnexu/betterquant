/*!
 * \file TestMain.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "def/BQConst.hpp"
#include "def/BQDef.hpp"
#include "def/PosInfo.hpp"
#include "def/SymbolInfo.hpp"
#include "util/PosSnapshotImpl.hpp"

using namespace bq;

class global_event : public testing::Environment {
 public:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

std::string PosGroup2Str(const PosInfoGroup& posInfoGroup) {
  std::string ret;
  for (const auto& rec : posInfoGroup) {
    ret = ret + rec->toStr() + "\n";
  }
  if (!ret.empty()) ret.pop_back();
  return ret;
}

TEST(testPosInfo, testMergePosInfoOfSpot) {
  auto posInfo1 = std::make_shared<PosInfo>();
  posInfo1->productId_ = 0;
  posInfo1->userId_ = 1;
  posInfo1->acctId_ = 2;
  posInfo1->stgId_ = 3;
  posInfo1->stgInstId_ = 4;
  posInfo1->algoId_ = 5;
  posInfo1->marketCode_ = MarketCode::Binance;
  posInfo1->symbolType_ = SymbolType::Spot;
  strncpy(posInfo1->symbolCode_, "BTC-USDT", sizeof(posInfo1->symbolCode_) - 1);
  posInfo1->side_ = Side::Bid;
  posInfo1->posSide_ = PosSide::Both;
  posInfo1->parValue_ = 100;
  strncpy(posInfo1->feeCurrency_, "USDT", sizeof(posInfo1->feeCurrency_) - 1);

  posInfo1->fee_ = 10;
  posInfo1->pos_ = 100;
  posInfo1->prePos_ = 0;
  posInfo1->avgOpenPrice_ = 20000;
  posInfo1->pnlUnReal_ = 30;
  posInfo1->pnlReal_ = 50;
  posInfo1->totalBidSize_ = 50;
  posInfo1->totalAskSize_ = -50;

  auto posInfo2 = std::make_shared<PosInfo>(*posInfo1);
  posInfo2->avgOpenPrice_ = 40000;
  posInfo2->feeCurrency_[0] = '\0';
  posInfo2->totalBidSize_ = 100;
  posInfo2->totalAskSize_ = -100;

  auto posInfo3 = std::make_shared<PosInfo>();
  posInfo3->productId_ = 0;
  posInfo3->userId_ = 1;
  posInfo3->acctId_ = 2;
  posInfo3->stgId_ = 3;
  posInfo3->stgInstId_ = 4;
  posInfo3->algoId_ = 5;
  posInfo3->marketCode_ = MarketCode::Binance;
  posInfo3->symbolType_ = SymbolType::Spot;
  strncpy(posInfo3->symbolCode_, "BTC-USDT", sizeof(posInfo3->symbolCode_) - 1);
  posInfo3->side_ = Side::Ask;
  posInfo3->posSide_ = PosSide::Both;
  posInfo3->parValue_ = 100;
  strncpy(posInfo3->feeCurrency_, "USDT", sizeof(posInfo3->feeCurrency_) - 1);

  posInfo3->fee_ = 10;
  posInfo3->pos_ = -100;
  posInfo3->prePos_ = 0;
  posInfo3->avgOpenPrice_ = 20000;
  posInfo3->pnlUnReal_ = 30;
  posInfo3->pnlReal_ = 50;
  posInfo3->totalBidSize_ = 50;
  posInfo3->totalAskSize_ = -50;

  auto posInfo4 = std::make_shared<PosInfo>(*posInfo3);
  posInfo4->avgOpenPrice_ = 40000;
  posInfo4->feeCurrency_[0] = '\0';
  posInfo4->totalBidSize_ = 100;
  posInfo4->totalAskSize_ = -100;

  PosInfoGroup posInfoGroup;
  posInfoGroup.emplace_back(posInfo1);
  posInfoGroup.emplace_back(posInfo2);
  posInfoGroup.emplace_back(posInfo3);
  posInfoGroup.emplace_back(posInfo4);

  MergePosInfoHasNoFeeCurrency(posInfoGroup);

  EXPECT_TRUE(posInfoGroup.size() == 2);
  EXPECT_TRUE(
      posInfoGroup[0]->toStr() ==
      R"(0/1/2/3/4/5/Binance/Spot/BTC-USDT/Bid/Both/100/USDT fee=30; pos=200; prePos=0; avgOpenPrice=30000; pnlUnReal=60; pnlReal=100)");
  EXPECT_TRUE(
      posInfoGroup[1]->toStr() ==
      R"(0/1/2/3/4/5/Binance/Spot/BTC-USDT/Ask/Both/100/USDT fee=30; pos=-200; prePos=0; avgOpenPrice=30000; pnlUnReal=60; pnlReal=100)");
}

TEST(testPosInfo, testMergePosInfoOfUBaseContract) {
  auto posInfo1 = std::make_shared<PosInfo>();
  posInfo1->productId_ = 0;
  posInfo1->userId_ = 1;
  posInfo1->acctId_ = 2;
  posInfo1->stgId_ = 3;
  posInfo1->stgInstId_ = 4;
  posInfo1->algoId_ = 5;
  posInfo1->marketCode_ = MarketCode::Binance;
  posInfo1->symbolType_ = SymbolType::Perp;
  strncpy(posInfo1->symbolCode_, "BTC-USDT", sizeof(posInfo1->symbolCode_) - 1);
  posInfo1->side_ = Side::Bid;
  posInfo1->posSide_ = PosSide::Both;
  posInfo1->parValue_ = 100;
  strncpy(posInfo1->feeCurrency_, "USDT", sizeof(posInfo1->feeCurrency_) - 1);

  posInfo1->fee_ = 10;
  posInfo1->pos_ = 100;
  posInfo1->prePos_ = 0;
  posInfo1->avgOpenPrice_ = 20000;
  posInfo1->pnlUnReal_ = 30;
  posInfo1->pnlReal_ = 50;
  posInfo1->totalBidSize_ = 50;
  posInfo1->totalAskSize_ = -50;

  auto posInfo2 = std::make_shared<PosInfo>(*posInfo1);
  posInfo2->avgOpenPrice_ = 40000;
  posInfo2->feeCurrency_[0] = '\0';
  posInfo2->totalBidSize_ = 100;
  posInfo2->totalAskSize_ = -100;

  auto posInfo3 = std::make_shared<PosInfo>();
  posInfo3->productId_ = 0;
  posInfo3->userId_ = 1;
  posInfo3->acctId_ = 2;
  posInfo3->stgId_ = 3;
  posInfo3->stgInstId_ = 4;
  posInfo3->algoId_ = 5;
  posInfo3->marketCode_ = MarketCode::Binance;
  posInfo3->symbolType_ = SymbolType::Perp;
  strncpy(posInfo3->symbolCode_, "BTC-USDT", sizeof(posInfo3->symbolCode_) - 1);
  posInfo3->side_ = Side::Ask;
  posInfo3->posSide_ = PosSide::Both;
  posInfo3->parValue_ = 100;
  strncpy(posInfo3->feeCurrency_, "USDT", sizeof(posInfo3->feeCurrency_) - 1);

  posInfo3->fee_ = 10;
  posInfo3->pos_ = -100;
  posInfo3->prePos_ = 0;
  posInfo3->avgOpenPrice_ = 20000;
  posInfo3->pnlUnReal_ = 30;
  posInfo3->pnlReal_ = 50;
  posInfo3->totalBidSize_ = 50;
  posInfo3->totalAskSize_ = -50;

  auto posInfo4 = std::make_shared<PosInfo>(*posInfo3);
  posInfo4->avgOpenPrice_ = 40000;
  posInfo4->feeCurrency_[0] = '\0';
  posInfo4->totalBidSize_ = 100;
  posInfo4->totalAskSize_ = -100;

  PosInfoGroup posInfoGroup;
  posInfoGroup.emplace_back(posInfo1);
  posInfoGroup.emplace_back(posInfo2);
  posInfoGroup.emplace_back(posInfo3);
  posInfoGroup.emplace_back(posInfo4);

  MergePosInfoHasNoFeeCurrency(posInfoGroup);

  EXPECT_TRUE(posInfoGroup.size() == 2);
  EXPECT_TRUE(
      posInfoGroup[0]->toStr() ==
      R"(0/1/2/3/4/5/Binance/Perp/BTC-USDT/Bid/Both/100/USDT fee=30; pos=200; prePos=0; avgOpenPrice=30000; pnlUnReal=60; pnlReal=100)");
  EXPECT_TRUE(
      posInfoGroup[1]->toStr() ==
      R"(0/1/2/3/4/5/Binance/Perp/BTC-USDT/Ask/Both/100/USDT fee=30; pos=-200; prePos=0; avgOpenPrice=30000; pnlUnReal=60; pnlReal=100)");
}

TEST(testPosInfo, testMergePosInfoOfCBaseContract) {
  auto posInfo1 = std::make_shared<PosInfo>();
  posInfo1->productId_ = 0;
  posInfo1->userId_ = 1;
  posInfo1->acctId_ = 2;
  posInfo1->stgId_ = 3;
  posInfo1->stgInstId_ = 4;
  posInfo1->algoId_ = 5;
  posInfo1->marketCode_ = MarketCode::Binance;
  posInfo1->symbolType_ = SymbolType::CPerp;
  strncpy(posInfo1->symbolCode_, "BTC-USDT", sizeof(posInfo1->symbolCode_) - 1);
  posInfo1->side_ = Side::Bid;
  posInfo1->posSide_ = PosSide::Both;
  posInfo1->parValue_ = 100;
  strncpy(posInfo1->feeCurrency_, "USDT", sizeof(posInfo1->feeCurrency_) - 1);

  posInfo1->fee_ = 10;
  posInfo1->pos_ = 100;
  posInfo1->prePos_ = 0;
  posInfo1->avgOpenPrice_ = 20000;
  posInfo1->pnlUnReal_ = 30;
  posInfo1->pnlReal_ = 50;
  posInfo1->totalBidSize_ = 50;
  posInfo1->totalAskSize_ = -50;

  auto posInfo2 = std::make_shared<PosInfo>(*posInfo1);
  posInfo2->avgOpenPrice_ = 40000;
  posInfo2->feeCurrency_[0] = '\0';
  posInfo2->totalBidSize_ = 100;
  posInfo2->totalAskSize_ = -100;

  auto posInfo3 = std::make_shared<PosInfo>();
  posInfo3->productId_ = 0;
  posInfo3->userId_ = 1;
  posInfo3->acctId_ = 2;
  posInfo3->stgId_ = 3;
  posInfo3->stgInstId_ = 4;
  posInfo3->algoId_ = 5;
  posInfo3->marketCode_ = MarketCode::Binance;
  posInfo3->symbolType_ = SymbolType::CPerp;
  strncpy(posInfo3->symbolCode_, "BTC-USDT", sizeof(posInfo3->symbolCode_) - 1);
  posInfo3->side_ = Side::Ask;
  posInfo3->posSide_ = PosSide::Both;
  posInfo3->parValue_ = 100;
  strncpy(posInfo3->feeCurrency_, "USDT", sizeof(posInfo3->feeCurrency_) - 1);

  posInfo3->fee_ = 10;
  posInfo3->pos_ = -100;
  posInfo3->prePos_ = 0;
  posInfo3->avgOpenPrice_ = 20000;
  posInfo3->pnlUnReal_ = 30;
  posInfo3->pnlReal_ = 50;
  posInfo3->totalBidSize_ = 50;
  posInfo3->totalAskSize_ = -50;

  auto posInfo4 = std::make_shared<PosInfo>(*posInfo3);
  posInfo4->avgOpenPrice_ = 40000;
  posInfo4->feeCurrency_[0] = '\0';
  posInfo4->totalBidSize_ = 100;
  posInfo4->totalAskSize_ = -100;

  PosInfoGroup posInfoGroup;
  posInfoGroup.emplace_back(posInfo1);
  posInfoGroup.emplace_back(posInfo2);
  posInfoGroup.emplace_back(posInfo3);
  posInfoGroup.emplace_back(posInfo4);

  MergePosInfoHasNoFeeCurrency(posInfoGroup);

  EXPECT_TRUE(posInfoGroup.size() == 2);
  EXPECT_TRUE(
      posInfoGroup[0]->toStr() ==
      R"(0/1/2/3/4/5/Binance/CPerp/BTC-USDT/Bid/Both/100/USDT fee=30; pos=200; prePos=0; avgOpenPrice=26666.666666666668; pnlUnReal=60; pnlReal=100)");
  EXPECT_TRUE(
      posInfoGroup[1]->toStr() ==
      R"(0/1/2/3/4/5/Binance/CPerp/BTC-USDT/Ask/Both/100/USDT fee=30; pos=-200; prePos=0; avgOpenPrice=26666.666666666668; pnlUnReal=60; pnlReal=100)");
}

TEST(testPosSnapshotImpl, testPosSnapshotImplGroupBy) {
  auto posInfo1 = std::make_shared<PosInfo>();
  posInfo1->userId_ = 1;
  posInfo1->acctId_ = 2;
  posInfo1->stgId_ = 3;
  posInfo1->stgInstId_ = 1211;

  auto posInfo2 = std::make_shared<PosInfo>(*posInfo1);
  posInfo2->userId_ = 2;
  posInfo2->acctId_ = 1;
  posInfo1->stgInstId_ = 2122;

  auto posInfo3 = std::make_shared<PosInfo>(*posInfo1);
  posInfo3->userId_ = 1;
  posInfo3->acctId_ = 2;
  posInfo1->stgInstId_ = 1213;

  auto posInfo4 = std::make_shared<PosInfo>(*posInfo1);
  posInfo4->userId_ = 2;
  posInfo4->acctId_ = 1;
  posInfo1->stgInstId_ = 2124;

  std::map<std::string, PosInfoSPtr> posInfoDetail;
  posInfoDetail.emplace(posInfo1->getKey(), posInfo1);
  posInfoDetail.emplace(posInfo2->getKey(), posInfo2);
  posInfoDetail.emplace(posInfo3->getKey(), posInfo3);
  posInfoDetail.emplace(posInfo4->getKey(), posInfo4);

  auto posSnapshotImpl =
      std::make_shared<PosSnapshotImpl>(posInfoDetail, nullptr);
  auto [ret, posInfoGroup] =
      posSnapshotImpl->queryPosInfoGroupBy("acctId&userId");
  EXPECT_TRUE((*posInfoGroup)["acctId=1&userId=2"]->size() == 2);
  EXPECT_TRUE((*posInfoGroup)["acctId=2&userId=1"]->size() == 2);
}

TEST(testPosInfo, testGetSymbolGroupUsedToCalcPnlOfSpot) {
  auto posInfo = std::make_shared<PosInfo>();
  posInfo->userId_ = 1;
  posInfo->acctId_ = 2;
  posInfo->stgId_ = 3;
  posInfo->stgInstId_ = 4;
  posInfo->marketCode_ = MarketCode::Binance;
  posInfo->symbolType_ = SymbolType::Spot;
  strncpy(posInfo->symbolCode_, "DOT-ETH", sizeof(posInfo->symbolCode_) - 1);
  posInfo->side_ = Side::Bid;
  posInfo->posSide_ = PosSide::Both;
  posInfo->parValue_ = 100;
  strncpy(posInfo->feeCurrency_, "BNB", sizeof(posInfo->feeCurrency_) - 1);

  const auto symbolInfoGroup =
      posInfo->getSymbolGroupUsedToCalcPnl("BTC", "USDT");
  std::string ret;
  for (const auto& symbolInfo : symbolInfoGroup) {
    ret = ret + symbolInfo->toStr() + "\n";
  }
  if (!ret.empty()) ret.pop_back();
  ret = "\n" + ret;

  EXPECT_TRUE(ret == R"(
Binance Spot BNB-BTC
Binance Spot BNB-USDT
Binance Spot BTC-BNB
Binance Spot BTC-ETH
Binance Spot BTC-USDT
Binance Spot ETH-BTC
Binance Spot ETH-USDT)");
}

TEST(testPosInfo, testGetSymbolGroupUsedToCalcPnlOfPerp) {
  auto posInfo = std::make_shared<PosInfo>();
  posInfo->userId_ = 1;
  posInfo->acctId_ = 2;
  posInfo->stgId_ = 3;
  posInfo->stgInstId_ = 4;
  posInfo->marketCode_ = MarketCode::Binance;
  posInfo->symbolType_ = SymbolType::Perp;
  strncpy(posInfo->symbolCode_, "DOT-USDT-Perp",
          sizeof(posInfo->symbolCode_) - 1);
  posInfo->side_ = Side::Bid;
  posInfo->posSide_ = PosSide::Both;
  posInfo->parValue_ = 100;
  strncpy(posInfo->feeCurrency_, "BNB", sizeof(posInfo->feeCurrency_) - 1);

  const auto symbolInfoGroup =
      posInfo->getSymbolGroupUsedToCalcPnl("BTC", "USDT");
  std::string ret;
  for (const auto& symbolInfo : symbolInfoGroup) {
    ret = ret + symbolInfo->toStr() + "\n";
  }
  if (!ret.empty()) ret.pop_back();
  ret = "\n" + ret;

  EXPECT_TRUE(ret == R"(
Binance Perp BNB-BTC
Binance Perp BNB-USDT
Binance Perp BTC-BNB
Binance Perp BTC-USD
Binance Perp BTC-USDT
Binance Perp USD-BTC
Binance Perp USD-USDT
Binance Perp USDT-BTC)");
}

TEST(testPosInfo, testGetSymbolGroupUsedToCalcPnlOfCPerp) {
  auto posInfo = std::make_shared<PosInfo>();
  posInfo->userId_ = 1;
  posInfo->acctId_ = 2;
  posInfo->stgId_ = 3;
  posInfo->stgInstId_ = 4;
  posInfo->marketCode_ = MarketCode::Binance;
  posInfo->symbolType_ = SymbolType::CPerp;
  strncpy(posInfo->symbolCode_, "DOT-USD-CPerp",
          sizeof(posInfo->symbolCode_) - 1);
  posInfo->side_ = Side::Bid;
  posInfo->posSide_ = PosSide::Both;
  posInfo->parValue_ = 100;
  strncpy(posInfo->feeCurrency_, "BNB", sizeof(posInfo->feeCurrency_) - 1);

  const auto symbolInfoGroup =
      posInfo->getSymbolGroupUsedToCalcPnl("BTC", "USDT");
  std::string ret;
  for (const auto& symbolInfo : symbolInfoGroup) {
    ret = ret + symbolInfo->toStr() + "\n";
  }
  if (!ret.empty()) ret.pop_back();
  ret = "\n" + ret;

  EXPECT_TRUE(ret == R"(
Binance CPerp BNB-BTC
Binance CPerp BNB-USDT
Binance CPerp BTC-BNB
Binance CPerp BTC-DOT
Binance CPerp BTC-USDT
Binance CPerp DOT-BTC
Binance CPerp DOT-USDT)");
}

int main(int argc, char** argv) {
  testing::AddGlobalTestEnvironment(new global_event);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
