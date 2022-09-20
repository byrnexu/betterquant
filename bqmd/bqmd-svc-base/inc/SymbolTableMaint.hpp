/*!
 * \file SymbolTableMaint.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "db/TBLSymbolInfo.hpp"
#include "util/Pch.hpp"

namespace bq {
class Scheduler;
using SchedulerSPtr = std::shared_ptr<Scheduler>;
namespace db {
template <typename TableSchema>
class TBLRec;
template <typename TableSchema>
using TBLRecSPtr = std::shared_ptr<TBLRec<TableSchema>>;
template <typename TableSchema>
using TBLRecSet = std::map<std::string, TBLRecSPtr<TableSchema>>;
template <typename TableSchema>
using TBLRecSetSPtr = std::shared_ptr<TBLRecSet<TableSchema>>;
}  // namespace db
}  // namespace bq

namespace bq::md::svc {

class MDSvc;

class SymbolTableMaint {
 public:
  SymbolTableMaint(const SymbolTableMaint&) = delete;
  SymbolTableMaint& operator=(const SymbolTableMaint&) = delete;
  SymbolTableMaint(const SymbolTableMaint&&) = delete;
  SymbolTableMaint& operator=(const SymbolTableMaint&&) = delete;

  explicit SymbolTableMaint(MDSvc* const mdSvc) : mdSvc_(mdSvc) {}

 public:
  int start();

 private:
  int execSymbolTableMaint();

  virtual std::tuple<int, db::TBLRecSetSPtr<TBLSymbolInfo>>
  querySymbolTableFromExch();

  virtual std::tuple<int, db::TBLRecSetSPtr<TBLSymbolInfo>>
  convertSymbolTableFromExch(const std::string& symbolTableFromExch) = 0;

 protected:
  std::tuple<int, db::symbolInfo::RecordSPtr> makeSymbolInfo(
      const Val& v) const {
    auto [ret, symbolInfo] = doMakeSymbolInfo(v);
    if (ret != 0) {
      return {ret, symbolInfo};
    }
    setDftValForUninitFields(symbolInfo);
    return {ret, symbolInfo};
  }

 private:
  virtual std::tuple<int, db::symbolInfo::RecordSPtr> doMakeSymbolInfo(
      const Val& v) const = 0;

  void setDftValForUninitFields(db::symbolInfo::RecordSPtr& symbolInfo) const;

  virtual std::tuple<int, db::TBLRecSetSPtr<TBLSymbolInfo>>
  querySymbolTableFromDB();

  void syncRecToTable(const db::TBLRecSetSPtr<TBLSymbolInfo>& tblRecSetAdd,
                      const db::TBLRecSetSPtr<TBLSymbolInfo>& tblRecSetDel,
                      const db::TBLRecSetSPtr<TBLSymbolInfo>& tblRecSetChg);

 public:
  void stop();

 protected:
  MDSvc* const mdSvc_;

 private:
  SchedulerSPtr schedulerSymbolTableMaint_{nullptr};
};

}  // namespace bq::md::svc
