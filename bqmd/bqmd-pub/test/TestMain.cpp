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

#include "def/Def.hpp"
#include "util/BQMDHis.hpp"
#include "util/Logger.hpp"

using namespace bq;
using namespace bq::md;

class global_event : public testing::Environment {
 public:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST(test, testBetween) {
  auto getValue = [](const auto& exchTs2HisMDGroup) {
    std::string ret;
    for (const auto& rec : *exchTs2HisMDGroup) {
      Doc doc;
      doc.Parse(rec.second.data());
      ret = ret + std::to_string(doc["data"]["value"].GetInt());
    }
    return ret;
  };

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBetweenTs("testData", "MD@Binance@Spot@BTC-USDT@Candle",
                                  1669338658437000, 1669338658437000);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBetweenTs("testData", "MD@Binance@Spot@BTC-USDT@Candle",
                                  1669338658437000, 1669338658437000 + 1);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "1");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBetweenTs("testData", "MD@Binance@Spot@BTC-USDT@Candle",
                                  1669338658437000, 1669338718861000);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "1");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBetweenTs("testData", "MD@Binance@Spot@BTC-USDT@Candle",
                                  1669338658437000, 1669338718861000 + 1);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBetweenTs("testData", "MD@Binance@Spot@BTC-USDT@Candle",
                                  1669338658437000, 1669458658437000);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBetweenTs("testData", "MD@Binance@Spot@BTC-USDT@Candle",
                                  1669338658437000, 1669458718861000);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "1234");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBetweenTs("testData", "MD@Binance@Spot@BTC-USDT@Candle",
                                  1669338658437000, 1669528658437000);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123456");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBetweenTs("testData", "MD@Binance@Spot@BTC-USDT@Candle",
                                  1669338658437000, 1669528718861000);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "12345678");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBetweenTs("testData", "MD@Binance@Spot@BTC-USDT@Candle",
                                  1669338658437000, 1669528718861000 + 1);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123456789");
  }
}

TEST(test, testBefore) {
  auto getValue = [](const auto& exchTs2HisMDGroup) {
    std::string ret;
    for (const auto& rec : *exchTs2HisMDGroup) {
      Doc doc;
      doc.Parse(rec.second.data());
      ret = ret + std::to_string(doc["data"]["value"].GetInt());
    }
    return ret;
  };

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBeforeTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669528718861000, 1);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "9");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBeforeTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669528718861000, 2);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "89");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBeforeTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669528718861000, 3);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "789");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBeforeTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669528718861000, 4);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "6789");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBeforeTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669528718861000, 5);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "56789");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBeforeTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669528718861000, 6);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "456789");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBeforeTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669528718861000, 7);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "3456789");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBeforeTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669528718861000, 8);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "23456789");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBeforeTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669528718861000, 9);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123456789");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBeforeTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338718861000, 1);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "3");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBeforeTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338718861000, 2);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "23");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBeforeTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338718861000, 3);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBeforeTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338718861000 + 1, 1);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "3");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBeforeTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338718861000 + 1, 2);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "23");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBeforeTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338718861000 + 1, 3);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123");
  }
}

TEST(test, testAfter) {
  auto getValue = [](const auto& exchTs2HisMDGroup) {
    std::string ret;
    for (const auto& rec : *exchTs2HisMDGroup) {
      Doc doc;
      doc.Parse(rec.second.data());
      ret = ret + std::to_string(doc["data"]["value"].GetInt());
    }
    return ret;
  };

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDAfterTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338658437000, 1);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "1");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDAfterTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338658437000, 2);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "12");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDAfterTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338658437000, 3);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDAfterTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338658437000, 4);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "1234");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDAfterTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338658437000, 5);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "12345");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDAfterTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338658437000, 6);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123456");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDAfterTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338658437000, 7);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "1234567");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDAfterTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338658437000, 8);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "12345678");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDAfterTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338658437000, 9);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123456789");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDAfterTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338718861000, 1);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "2");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDAfterTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338718861000, 2);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "23");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDAfterTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338718861000, 3);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "234");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDAfterTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338718861000 - 1, 1);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "2");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDAfterTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338718861000 - 1, 2);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "23");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDAfterTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Candle", 1669338718861000 - 1, 3);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "234");
  }
}

TEST(test, testBetweenByLT) {
  auto getValue = [](const auto& exchTs2HisMDGroup) {
    std::string ret;
    for (const auto& rec : *exchTs2HisMDGroup) {
      Doc doc;
      doc.Parse(rec.second.data());
      ret = ret + std::to_string(doc["data"]["value"].GetInt());
    }
    return ret;
  };

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBetweenTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Trades", 1669338658437000,
        1669338658437000, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBetweenTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Trades", 1669338658437000,
        1669338658437000 + 1, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "1");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBetweenTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Trades", 1669338658437000,
        1669338718861000, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "1");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBetweenTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Trades", 1669338658437000,
        1669338718861000 + 1, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBetweenTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Trades", 1669338658437000,
        1669458658437000, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBetweenTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Trades", 1669338658437000,
        1669458718861000, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "1234");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBetweenTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Trades", 1669338658437000,
        1669528658437000, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123456");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBetweenTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Trades", 1669338658437000,
        1669528718861000, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "12345678");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] = MDHis::LoadHisMDBetweenTs(
        "testData", "MD@Binance@Spot@BTC-USDT@Trades", 1669338658437000,
        1669528718861000 + 1, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123456789");
  }
}

TEST(test, testBeforeByLT) {
  auto getValue = [](const auto& exchTs2HisMDGroup) {
    std::string ret;
    for (const auto& rec : *exchTs2HisMDGroup) {
      Doc doc;
      doc.Parse(rec.second.data());
      ret = ret + std::to_string(doc["data"]["value"].GetInt());
    }
    return ret;
  };

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBeforeTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                 1669528718861000, 1, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "9");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBeforeTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                 1669528718861000, 2, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "89");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBeforeTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                 1669528718861000, 3, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "789");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBeforeTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                 1669528718861000, 4, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "6789");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBeforeTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                 1669528718861000, 5, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "56789");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBeforeTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                 1669528718861000, 6, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "456789");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBeforeTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                 1669528718861000, 7, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "3456789");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBeforeTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                 1669528718861000, 8, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "23456789");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBeforeTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                 1669528718861000, 9, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123456789");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBeforeTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                 1669338718861000, 1, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "3");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBeforeTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                 1669338718861000, 2, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "23");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBeforeTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                 1669338718861000, 3, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBeforeTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                 1669338718861000 + 1, 1, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "3");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBeforeTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                 1669338718861000 + 1, 2, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "23");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDBeforeTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                 1669338718861000 + 1, 3, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123");
  }
}

TEST(test, testAfterByLT) {
  auto getValue = [](const auto& exchTs2HisMDGroup) {
    std::string ret;
    for (const auto& rec : *exchTs2HisMDGroup) {
      Doc doc;
      doc.Parse(rec.second.data());
      ret = ret + std::to_string(doc["data"]["value"].GetInt());
    }
    return ret;
  };

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDAfterTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                1669338658437000, 1, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "1");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDAfterTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                1669338658437000, 2, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "12");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDAfterTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                1669338658437000, 3, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDAfterTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                1669338658437000, 4, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "1234");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDAfterTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                1669338658437000, 5, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "12345");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDAfterTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                1669338658437000, 6, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123456");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDAfterTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                1669338658437000, 7, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "1234567");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDAfterTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                1669338658437000, 8, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "12345678");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDAfterTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                1669338658437000, 9, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "123456789");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDAfterTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                1669338718861000, 1, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "2");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDAfterTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                1669338718861000, 2, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "23");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDAfterTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                1669338718861000, 3, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "234");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDAfterTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                1669338718861000 - 1, 1, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "2");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDAfterTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                1669338718861000 - 1, 2, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "23");
  }

  {
    const auto [stausCode, exchTs2HisMDGroup] =
        MDHis::LoadHisMDAfterTs("testData", "MD@Binance@Spot@BTC-USDT@Trades",
                                1669338718861000 - 1, 3, IndexType::ByLocalTs);
    const auto value = getValue(exchTs2HisMDGroup);
    EXPECT_TRUE(value == "234");
  }
}

int main(int argc, char** argv) {
  testing::AddGlobalTestEnvironment(new global_event);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
