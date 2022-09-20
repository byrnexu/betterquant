/*!
 * \file SymbolTableMaintOfBinance.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "SymbolTableMaint.hpp"
#include "def/Def.hpp"

namespace bq::md::svc::binance {

class SymbolTableMaintOfBinance : public SymbolTableMaint {
 public:
  using SymbolTableMaint::SymbolTableMaint;

 private:
  std::tuple<int, db::TBLRecSetSPtr<TBLSymbolInfo>> convertSymbolTableFromExch(
      const std::string& symbolTableFromExch) final;

  std::tuple<int, db::symbolInfo::RecordSPtr> doMakeSymbolInfo(
      const Val& v) const final;
};

}  // namespace bq::md::svc::binance
