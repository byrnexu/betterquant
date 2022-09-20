/*!
 * \file Pnl.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "def/Pnl.hpp"

#include "def/SymbolInfoIF.hpp"
#include "util/BQUtil.hpp"
#include "util/Datetime.hpp"
#include "util/Pch.hpp"

namespace bq {

std::string Pnl::toStr() const {
  const auto delay = GetTotalMSSince1970() - updateTime_ / 1000;
  const auto ret = fmt::format(
      "{} totalPnl: {} {}; pnlUnReal: {} {}; pnlReal: {} {}; fee: {} {}; "
      "delay: {}ms; statusCode: {}",
      queryCond_, ToPrettyStr(getTotalPnl()), quoteCurrencyForCalc_,
      ToPrettyStr(pnlUnReal_), quoteCurrencyForCalc_, ToPrettyStr(pnlReal_),
      quoteCurrencyForCalc_, ToPrettyStr((-1 * fee_)), quoteCurrencyForCalc_,
      delay, statusCode_);
  return ret;
}

Decimal Pnl::getTotalPnl() const { return pnlUnReal_ + pnlReal_ - fee_; }

bool Pnl::isValid(std::uint32_t secDelayOfPrice) const {
  const auto realDelay = GetTotalSecSince1970() - updateTime_ / 1000000;
  return realDelay < secDelayOfPrice && statusCode_ == 0;
}

// clang-format off
std::string Pnl::getSqlOfInsert() const {
const auto now = GetTotalUSSince1970();
std::uint64_t delay;
if (now >= updateTime_){
  delay = (now - updateTime_) / 1000;
} else {
  delay = 0;
}
const auto sql = fmt::format(
"INSERT INTO `BetterQuant`.`hisPnl` ("
  "`queryCond`,"
  "`quoteCurrency`,"
  "`totalPnl`,"
  "`pnlUnReal`,"
  "`pnlReal`,"
  "`fee`,"
  "`delay`,"
  "`statusCode`"
")"
"VALUES"
"("
  "'{}',"  // queryCond
  "'{}',"  // quoteCurrency
  " {} ,"  // totalPnl
  " {} ,"  // pnlUnReal
  " {} ,"  // pnlReal
  " {} ,"  // fee
  " {} ,"  // delay
  " {}  "  // statusCode
"); ",
  queryCond_,
  quoteCurrencyForCalc_,
  getTotalPnl(),
  pnlUnReal_,
  pnlReal_,
  fee_,
  delay,
  statusCode_ 
);
return sql;
} ;
// clang-format on

}  // namespace bq
