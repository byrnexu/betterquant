/*!
 * \file TBLMonitor.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "db/DBE.hpp"
#include "db/TBLRecSetMaker.hpp"
#include "def/Const.hpp"
#include "def/Def.hpp"
#include "util/Logger.hpp"
#include "util/Pch.hpp"
#include "util/Random.hpp"
#include "util/Scheduler.hpp"

namespace bq::db {

enum class EnableMonitoring { True = 1, False = 2 };

template <typename TableSchema>
using CBOnTblChg = std::function<void(const TBLRecSetSPtr<TableSchema>&,
                                      const TBLRecSetSPtr<TableSchema>&,
                                      const TBLRecSetSPtr<TableSchema>&)>;

template <typename TableSchema>
class TBLMonitor {
 public:
  TBLMonitor(const TBLMonitor&) = delete;
  TBLMonitor& operator=(const TBLMonitor&) = delete;
  TBLMonitor(const TBLMonitor&&) = delete;
  TBLMonitor& operator=(const TBLMonitor&&) = delete;

  TBLMonitor(const db::DBEngSPtr& dbEng, std::uint32_t intervalOfMonit,
             const std::string& sql,
             const CBOnTblChg<TableSchema>& cbOnTblChg = nullptr,
             EnableMonitoring enableMonitoring = EnableMonitoring::True)
      : dbEng_(dbEng),
        intervalOfMonit_(intervalOfMonit),
        sql_(sql),
        tblRecSet_(std::make_shared<TBLRecSet<TableSchema>>()),
        cbOnTblChg_(cbOnTblChg),
        enableMonitoring_(enableMonitoring) {}

 public:
  int start() {
    doMonit();
    if (enableMonitoring_ == EnableMonitoring::True) {
      schedulerOfMonitor_ = std::make_shared<Scheduler>(
          "TBL_MONITOR", [this]() { doMonit(); }, intervalOfMonit_);
      auto ret = schedulerOfMonitor_->start();
      if (ret != 0) {
        LOG_E("[{}] Start tbl monitor failed.", TableSchema::TableName);
        return ret;
      }
      LOG_D("[{}] Start tbl monitor success.", TableSchema::TableName);
    }
    return 0;
  }

 private:
  int doMonit() {
    auto [ret, newTBLRecSet] =
        TBLRecSetMaker<TableSchema>::ExecSql(dbEng_, sql_);
    if (ret != 0) {
      LOG_W("[{}] Do monit failed. [sql = {}]", TableSchema::TableName, sql_);
      return ret;
    }
    LOG_D("[{}] Begin to compare data in cache and db. [sql = {}]",
          TableSchema::TableName, sql_);

    TBLRecSetSPtr<TableSchema> tblRecSetAdd;
    TBLRecSetSPtr<TableSchema> tblRecSetDel;
    TBLRecSetSPtr<TableSchema> tblRecSetChg;
    std::tie(tblRecSetAdd, tblRecSetDel, tblRecSetChg) =
        TBLRecSetCompare(newTBLRecSet, tblRecSet_);
    tblRecSet_ = newTBLRecSet;

    if (isFirstTimeOfMonit) {
      isFirstTimeOfMonit = false;
      initNecessaryDataStructures(tblRecSetAdd);
      return 0;
    }

    if (!tblRecSetAdd->empty() || !tblRecSetDel->empty() ||
        !tblRecSetChg->empty()) {
      handleTBLRecSetOfCompRet(tblRecSetAdd, tblRecSetDel, tblRecSetChg);
      if (cbOnTblChg_) {
        cbOnTblChg_(tblRecSetAdd, tblRecSetDel, tblRecSetChg);
      }
      return 0;
    }
    return 0;
  }

 private:
  virtual void initNecessaryDataStructures(
      const db::TBLRecSetSPtr<TableSchema>& tblRecOfAll) {}

  virtual void handleTBLRecSetOfCompRet(
      const db::TBLRecSetSPtr<TableSchema>& tblRecSetAdd,
      const db::TBLRecSetSPtr<TableSchema>& tblRecSetDel,
      const db::TBLRecSetSPtr<TableSchema>& tblRecSetChg) {}

 public:
  void stop() {
    if (enableMonitoring_ == EnableMonitoring::True) {
      schedulerOfMonitor_->stop();
      LOG_D("[{}] Stop tbl monitor finished.", TableSchema::TableName);
    }
  };

 private:
  db::DBEngSPtr dbEng_{nullptr};

 private:
  SchedulerSPtr schedulerOfMonitor_{nullptr};
  std::uint32_t intervalOfMonit_;

  std::string sql_;

  TBLRecSetSPtr<TableSchema> tblRecSet_{nullptr};
  bool isFirstTimeOfMonit{true};

  CBOnTblChg<TableSchema> cbOnTblChg_{nullptr};
  EnableMonitoring enableMonitoring_{EnableMonitoring::True};
};

}  // namespace bq::db
