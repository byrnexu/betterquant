/*!
 * \file PosMgrRestorer.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "PosMgrRestorer.hpp"

#include "Config.hpp"
#include "PosMgr.hpp"
#include "TDSrv.hpp"
#include "db/DBE.hpp"
#include "db/TBLAcctInfo.hpp"
#include "db/TBLRecSetMaker.hpp"
#include "db/TBLTradeInfo.hpp"
#include "def/BQDef.hpp"
#include "def/DataStruOfOthers.hpp"
#include "def/DataStruOfTD.hpp"
#include "def/Def.hpp"
#include "def/TaskOfSync.hpp"
#include "util/Datetime.hpp"
#include "util/Logger.hpp"
#include "util/Random.hpp"

namespace bq::td::srv {

PosMgrRestorer::PosMgrRestorer(TDSrv* tdSrv) : tdSrv_(tdSrv) {}

int PosMgrRestorer::exec() {
  const auto [retOfMakeAcct2No, acctId2lastNoUsedToCalcPos] =
      makeAcctId2lastNoUsedToCalcPos();
  if (retOfMakeAcct2No != 0) {
    return retOfMakeAcct2No;
  }

  const auto [retOfGetOrd, acctId2OrderInfoForPosNotCalc] =
      getAcct2OrderInfoForPosMgrNotCalc(acctId2lastNoUsedToCalcPos);
  if (retOfMakeAcct2No != 0) {
    return retOfMakeAcct2No;
  }

  restorePosMgr(acctId2OrderInfoForPosNotCalc);
  return 0;
}

std::tuple<int, std::map<AcctId, std::uint64_t>>
PosMgrRestorer::makeAcctId2lastNoUsedToCalcPos() {
  auto [retOfQryAllAcctId, acctId2NoUsedToCalcPos] = queryAllAcctId();
  if (retOfQryAllAcctId != 0) {
    return {retOfQryAllAcctId, std::map<AcctId, std::uint64_t>()};
  }

  auto [retOfQryAcctId2No, acctId2lastNoUsedToCalcPos] =
      queryAcctId2lastNoUsedToCalcPos();
  if (retOfQryAllAcctId != 0) {
    return {retOfQryAcctId2No, std::map<AcctId, std::uint64_t>()};
  }

  acctId2lastNoUsedToCalcPos.merge(acctId2NoUsedToCalcPos);
  for (const auto& rec : acctId2lastNoUsedToCalcPos) {
#ifndef NDEBUG
    LOG_I("Load acctId2lastNoUsedToCalcPos {} - {}", rec.first, rec.second);
#endif
  }

  return {0, acctId2lastNoUsedToCalcPos};
}

std::tuple<int, std::map<AcctId, std::uint64_t>>
PosMgrRestorer::queryAllAcctId() {
  std::map<AcctId, std::uint64_t> acctId2emptyNoUsedToCalcPos;

  const auto sql = R"(SELECT * FROM acctInfo;)";
  const auto [ret, tblRecSet] =
      db::TBLRecSetMaker<TBLAcctInfo>::ExecSql(tdSrv_->getDBEng(), sql);
  if (ret != 0) {
    LOG_W("Query all acctid from db failed. {}", sql);
    return {ret, acctId2emptyNoUsedToCalcPos};
  }

  for (const auto& tblRec : *tblRecSet) {
    const auto acctId = tblRec.second->getRecWithAllFields()->acctId;
    acctId2emptyNoUsedToCalcPos.emplace(acctId, 0);
  }

  return {0, acctId2emptyNoUsedToCalcPos};
}

std::tuple<int, std::map<AcctId, std::uint64_t>>
PosMgrRestorer::queryAcctId2lastNoUsedToCalcPos() {
  std::map<AcctId, std::uint64_t> acctId2lastNoUsedToCalcPos;

  const auto identity = GET_RAND_STR();
  const auto sql =
      "SELECT * FROM `posInfo` AS a WHERE `lastNoUsedToCalcPos` = "
      "(SELECT MAX(b.`lastNoUsedToCalcPos`) FROM `posInfo` AS b "
      "WHERE a.`acctId` = b.`acctId`)";
  const auto [ret, execRet] = tdSrv_->getDBEng()->syncExec(identity, sql);
  if (ret != 0) {
    LOG_W("Make acctId2lastNoUsedToCalcPos failed. [{}]", sql);
    return {-1, acctId2lastNoUsedToCalcPos};
  }

  DocSPtr doc = std::make_shared<Doc>();
  doc->Parse(execRet.data());

  if ((*doc)["recordSetGroup"].Size() == 0) {
    return {0, acctId2lastNoUsedToCalcPos};
  }

  const auto statusCode = (*doc)["statusCode"].GetInt();
  if (statusCode != 0) {
    const std::string statusMsg = (*doc)["statusMsg"].GetString();
    LOG_W("Make acctId2lastNoUsedToCalcPos failed. [{} - {}] {}", statusCode,
          statusMsg, sql);
    return {-1, acctId2lastNoUsedToCalcPos};
  }

  for (std::size_t i = 0; i < (*doc)["recordSetGroup"][0].Size(); ++i) {
    const auto acctId = (*doc)["recordSetGroup"][0][i]["acctId"].GetUint();
    const auto lastNoUsedToCalcPos =
        (*doc)["recordSetGroup"][0][i]["lastNoUsedToCalcPos"].GetUint64();
    acctId2lastNoUsedToCalcPos.emplace(acctId, lastNoUsedToCalcPos);
  }

  return {0, acctId2lastNoUsedToCalcPos};
}

std::tuple<int, std::map<AcctId, std::map<std::uint64_t, OrderInfoSPtr>>>
PosMgrRestorer::getAcct2OrderInfoForPosMgrNotCalc(
    const std::map<AcctId, std::uint64_t>& acctId2lastNoUsedToCalcPos) {
  std::map<AcctId, std::map<std::uint64_t, OrderInfoSPtr>>
      acctId2OrderInfoForPosNotCalc;

  for (const auto rec : acctId2lastNoUsedToCalcPos) {
    const auto acctId = rec.first;
    const auto lastNoUsedToCalcPos = rec.second;
    const auto sql = fmt::format(
        "SELECT * FROM tradeInfo "
        "WHERE acctId = {} and noUsedToCalcPos > {};",
        acctId, lastNoUsedToCalcPos);

    const auto [ret, tblRecSet] =
        db::TBLRecSetMaker<TBLTradeInfo>::ExecSql(tdSrv_->getDBEng(), sql);
    if (ret != 0) {
      LOG_W("Query tradeInfo from db failed. {}", sql);
      continue;
    }

    for (const auto& tblRec : *tblRecSet) {
      const auto& tradeInfo = tblRec.second->getRecWithAllFields();
      const auto orderInfo = MakeOrderInfo(tradeInfo);
      acctId2OrderInfoForPosNotCalc[acctId].emplace(orderInfo->noUsedToCalcPos_,
                                                    orderInfo);
    }
  }

  return {0, acctId2OrderInfoForPosNotCalc};
}

void PosMgrRestorer::restorePosMgr(
    const std::map<AcctId, std::map<std::uint64_t, OrderInfoSPtr>>&
        acctId2OrderInfoForPosNotCalc) {
  auto posMgr = std::make_shared<PosMgr>();
  const auto sql = fmt::format("SELECT * FROM `posInfo`");
  posMgr->init(CONFIG, tdSrv_->getDBEng(), sql);

  for (const auto& rec : acctId2OrderInfoForPosNotCalc) {
    const auto acctId = rec.first;
    const auto& no2OrderInfo = rec.second;
    LOG_W("Begin to restore pos of acctId {}. [order num = {}]", acctId,
          no2OrderInfo.size());

    for (const auto& rec : no2OrderInfo) {
      const auto& orderInfo = rec.second;
      LOG_W("Begin to restore pos by order info. {}", orderInfo->toShortStr());
      const auto posChgInfo = posMgr->updateByOrderInfoFromTDGW(orderInfo);
      tdSrv_->cacheTaskOfSyncGroup(MSG_ID_SYNC_POS_INFO, posChgInfo,
                                   SyncToRiskMgr::False, SyncToDB::True);
    }
  }
}

}  // namespace bq::td::srv
