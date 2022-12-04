/*!
 * \file SimedTDInfo.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/12/01
 *
 * \brief
 */

#include "def/SimedTDInfo.hpp"

#include "def/BQConst.hpp"
#include "def/BQDef.hpp"
#include "def/Const.hpp"
#include "def/Def.hpp"
#include "def/StatusCode.hpp"
#include "util/Float.hpp"
#include "util/Logger.hpp"
#include "util/Pch.hpp"

namespace bq {

std::string TransDetail::toStr() const {
  std::string direction;
  if (liquidityDirection_ == LiquidityDirection::Maker) {
    direction = "M";
  } else if (liquidityDirection_ == LiquidityDirection::Taker) {
    direction = "T";
  } else {
    LOG_W(
        "Convert trans detail to str failed "
        "because of invalid liquidityDirection {}.",
        magic_enum::enum_name(liquidityDirection_));
  }
  const auto ret =
      fmt::format("{}{}{}{}{}", slippage_, SEP_OF_TRANS_DETAIL_FIELD,
                  filledPer_, SEP_OF_TRANS_DETAIL_FIELD, direction);
  return ret;
}

TransDetailSPtr MakeTransDetail(const std::string& transDetailInJsonFmt) {
  std::vector<std::string> fieldGroup;
  boost::split(fieldGroup, transDetailInJsonFmt,
               boost::is_any_of(SEP_OF_TRANS_DETAIL_FIELD));
  TransDetailSPtr transDetail = std::make_shared<TransDetail>();
  transDetail->slippage_ = CONV(Decimal, fieldGroup[0]);
  transDetail->filledPer_ = CONV(Decimal, fieldGroup[1]);
  boost::to_lower(fieldGroup[2]);
  if (fieldGroup[2] == "t") {
    transDetail->liquidityDirection_ = LiquidityDirection::Taker;
  } else if (fieldGroup[2] == "m") {
    transDetail->liquidityDirection_ = LiquidityDirection::Maker;
  } else {
    LOG_W("Make trans detail failed because of invalid liquidityDirection {}.",
          fieldGroup[2]);
    return nullptr;
  }
  return transDetail;
}

SimedTDInfo::SimedTDInfo(
    OrderStatus orderStatus,
    const std::vector<std::tuple<Decimal, Decimal, LiquidityDirection>>&
        transDetail) {
  orderStatus_ = orderStatus;
  for (const auto& rec : transDetail) {
    const auto [slippage, filledPer, liquidityDirection] = rec;
    const auto t =
        std::make_shared<TransDetail>(slippage, filledPer, liquidityDirection);
    transDetailGroup_.emplace_back(t);
  }
}

SimedTDInfo::SimedTDInfo(OrderStatus orderStatus,
                         const TransDetailGroup& transDetailGroup)
    : orderStatus_(orderStatus),
      transDetailGroup_(std::move(transDetailGroup)) {}

std::string MakeSimedTDInfoInJsonFmt(
    OrderStatus orderStatus,
    const std::vector<std::tuple<Decimal, Decimal, LiquidityDirection>>&
        transDetailGroup) {
  rapidjson::StringBuffer strBuf;
  rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);

  writer.StartObject();
  writer.Key("s");
  writer.Uint(magic_enum::enum_integer(orderStatus));
  std::string t;
  for (const auto& rec : transDetailGroup) {
    const auto [slippage, filledPer, liquidityDirection] = rec;
    std::string direction;
    if (liquidityDirection == LiquidityDirection::Maker) {
      direction = "M";
    } else if (liquidityDirection == LiquidityDirection::Taker) {
      direction = "T";
    } else {
      LOG_W(
          "Make simed td info failed because of invalid liquidityDirection {}.",
          magic_enum::enum_name(liquidityDirection));
      continue;
    }
    const auto td =
        fmt::format("{}{}{}{}{}", slippage, SEP_OF_TRANS_DETAIL_FIELD,
                    filledPer, SEP_OF_TRANS_DETAIL_FIELD, direction);
    t = t + td + SEP_OF_TRANS_DETAIL_REC;
  }
  if (!t.empty()) t.pop_back();
  writer.Key("t");
  writer.String(t.c_str());
  writer.EndObject();

  return strBuf.GetString();
}

// simedTDInfoInJsonFmt = {"s":100,"t":"0,0.1,T;0,0.9,M"}
std::tuple<int, SimedTDInfoSPtr> MakeSimedTDInfo(
    const std::string& simedTDInfoInJsonFmt) {
  Doc doc;
  if (doc.Parse(simedTDInfoInJsonFmt.data()).HasParseError()) {
    LOG_W("Parse data failed. {0} [offset {1}] {2}",
          GetParseError_En(doc.GetParseError()), doc.GetErrorOffset(),
          simedTDInfoInJsonFmt);
    return {SCODE_BQPUB_INVALID_FORMAT_OF_SIMED_TD_INFO, nullptr};
  }

  auto simedTDInfo = std::make_shared<SimedTDInfo>();

  const auto s = doc["s"].GetUint();
  simedTDInfo->orderStatus_ = magic_enum::enum_cast<OrderStatus>(s).value();

  std::vector<std::string> transDetailInJsonFmtGroup;
  const std::string t = doc["t"].GetString();
  if (!t.empty()) {
    boost::split(transDetailInJsonFmtGroup, t,
                 boost::is_any_of(SEP_OF_TRANS_DETAIL_REC));
    for (const auto& transDetailInJsonFmt : transDetailInJsonFmtGroup) {
      const auto transDetail = MakeTransDetail(transDetailInJsonFmt);
      simedTDInfo->transDetailGroup_.emplace_back(transDetail);
    }
  }

  const auto statusCode = checkIfSimedTDInfoValid(simedTDInfo);
  return {statusCode, simedTDInfo};
}

int checkIfSimedTDInfoValid(const SimedTDInfoSPtr& simedTDInfo) {
  switch (simedTDInfo->orderStatus_) {
    case OrderStatus::ConfirmedByExch:
      if (!simedTDInfo->transDetailGroup_.empty()) {
        return SCODE_BQPUB_INVALID_TRANS_DETAIL_IN_SIMED_TD_INFO;
      }
      break;

    case OrderStatus::Filled: {
      if (simedTDInfo->transDetailGroup_.empty()) {
        return SCODE_BQPUB_INVALID_TRANS_DETAIL_IN_SIMED_TD_INFO;
      }
      Decimal totalPer = 0;
      for (std::size_t i = 0; i < simedTDInfo->transDetailGroup_.size(); ++i) {
        const auto slippage = simedTDInfo->transDetailGroup_[i]->slippage_;
        if (isApproximatelyEqual(slippage, 1.0) ||
            isDefinitelyGreaterThan(slippage, 1.0)) {
          return SCODE_BQPUB_INVALID_TRANS_DETAIL_IN_SIMED_TD_INFO;
        }
        totalPer += simedTDInfo->transDetailGroup_[i]->filledPer_;
      }
      if (!isApproximatelyEqual(totalPer, 1.0)) {
        return SCODE_BQPUB_INVALID_TRANS_DETAIL_IN_SIMED_TD_INFO;
      }
    } break;

    case OrderStatus::PartialFilled: {
      if (simedTDInfo->transDetailGroup_.empty()) {
        return SCODE_BQPUB_INVALID_TRANS_DETAIL_IN_SIMED_TD_INFO;
      }
      Decimal totalPer = 0;
      for (std::size_t i = 0; i < simedTDInfo->transDetailGroup_.size(); ++i) {
        const auto slippage = simedTDInfo->transDetailGroup_[i]->slippage_;
        if (isApproximatelyEqual(slippage, 1.0) ||
            isDefinitelyGreaterThan(slippage, 1.0)) {
          return SCODE_BQPUB_INVALID_TRANS_DETAIL_IN_SIMED_TD_INFO;
        }
        totalPer += simedTDInfo->transDetailGroup_[i]->filledPer_;
      }
      if (isApproximatelyEqual(totalPer, 1.0) ||
          isDefinitelyGreaterThan(totalPer, 1.0)) {
        return SCODE_BQPUB_INVALID_TRANS_DETAIL_IN_SIMED_TD_INFO;
      }
    } break;

    case OrderStatus::Failed:
      if (!simedTDInfo->transDetailGroup_.empty()) {
        return SCODE_BQPUB_INVALID_TRANS_DETAIL_IN_SIMED_TD_INFO;
      }
      break;

    default:
      return SCODE_BQPUB_INVALID_ORDER_STATUS_IN_SIMED_TD_INFO;
  }

  return 0;
}

std::string ConvertSimedTDInfoToJsonFmt(const SimedTDInfoSPtr& simedTDInfo) {
  rapidjson::StringBuffer strBuf;
  rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);

  writer.StartObject();
  writer.Key("s");
  writer.Uint(magic_enum::enum_integer(simedTDInfo->orderStatus_));
  std::string t;
  for (const auto& transDetail : simedTDInfo->transDetailGroup_) {
    t = t + transDetail->toStr() + SEP_OF_TRANS_DETAIL_REC;
  }
  if (!t.empty()) t.pop_back();
  writer.Key("t");
  writer.String(t.c_str());
  writer.EndObject();

  return strBuf.GetString();
}

}  // namespace bq
