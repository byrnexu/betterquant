/*!
 * \file TBLMonitorOfSymbolInfo.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "db/TBLMonitor.hpp"
#include "db/TBLSymbolInfo.hpp"
#include "def/StatusCode.hpp"
#include "util/StdExt.hpp"

namespace bq::db {

class TBLMonitorOfSymbolInfo : public TBLMonitor<TBLSymbolInfo> {
  using RecSymbolInfo = symbolInfo::Record;
  using RecSymbolInfoSPtr = std::shared_ptr<RecSymbolInfo>;

  struct KeyMarketCodeSymbolCode
      : boost::multi_index::composite_key<
            symbolInfo::Record,
            MIDX_MEMER(symbolInfo::Record, std::uint64_t, hashOfMktSym_)> {};

  struct KeyMarketCodeSymbolTypeExchSymbolCode
      : boost::multi_index::composite_key<
            symbolInfo::Record, MIDX_MEMER(symbolInfo::Record, std::uint64_t,
                                           hashOfMktSymExchSym_)> {};

  struct TagMarketCodeSymbolCode {};
  struct TagMarketCodeSymbolTypeExchSymbolCode {};

  using MIdxMarketCodeSymbolCode = boost::multi_index::ordered_unique<
      boost::multi_index::tag<TagMarketCodeSymbolCode>, KeyMarketCodeSymbolCode,
      boost::multi_index::composite_key_result_less<
          KeyMarketCodeSymbolCode::result_type>>;

  using MIdxMarketCodeSymbolTypeExchSymbolCode =
      boost::multi_index::ordered_unique<
          boost::multi_index::tag<TagMarketCodeSymbolTypeExchSymbolCode>,
          KeyMarketCodeSymbolTypeExchSymbolCode,
          boost::multi_index::composite_key_result_less<
              KeyMarketCodeSymbolTypeExchSymbolCode::result_type>>;

  using MIDXSymbolInfo = boost::multi_index::multi_index_container<
      RecSymbolInfoSPtr,
      boost::multi_index::indexed_by<MIdxMarketCodeSymbolCode,
                                     MIdxMarketCodeSymbolTypeExchSymbolCode>>;

 public:
  using TBLMonitor<TBLSymbolInfo>::TBLMonitor;

 public:
  std::tuple<int, RecSymbolInfoSPtr> getRecSymbolInfoBySymbolCode(
      const std::string& marketCode, const std::string& symbolCode) const {
    const auto hashData = fmt::format("{}{}", marketCode, symbolCode);
    const auto hash = XXH3_64bits(hashData.data(), hashData.size());
    {
      std::lock_guard<std::ext::spin_mutex> guard(mtxMIDXSymbolInfo_);
      auto& idx = midxSymbolInfo_.get<TagMarketCodeSymbolCode>();
      const auto iter = idx.find(hash);
      if (iter != std::end(idx)) {
        return {0, *iter};
      }
    }
    LOG_W("Can not find symbol info of symbol {} - {}.", marketCode,
          symbolCode);
    return {SCODE_DB_CAN_NOT_FIND_SYM_CODE, nullptr};
  }

  std::tuple<int, RecSymbolInfoSPtr> getRecSymbolInfoByExchSymbolCode(
      const std::string& marketCode, const std::string& symbolType,
      const std::string& exchSymbolCode) const {
    const auto hashData =
        fmt::format("{}{}{}", marketCode, symbolType, exchSymbolCode);
    const auto hash = XXH3_64bits(hashData.data(), hashData.size());
    {
      std::lock_guard<std::ext::spin_mutex> guard(mtxMIDXSymbolInfo_);
      auto& idx = midxSymbolInfo_.get<TagMarketCodeSymbolTypeExchSymbolCode>();
      const auto iter = idx.find(hash);
      if (iter != std::end(idx)) {
        return {0, *iter};
      }
    }
    LOG_W("Can not find symbol info of exch symbol {} - {} - {}.", marketCode,
          symbolType, exchSymbolCode);
    return {SCODE_DB_CAN_NOT_FIND_EXCH_SYM_CODE, nullptr};
  }

  std::tuple<int, std::string> getSymbolCode(
      const std::string& marketCode, const std::string& symbolType,
      const std::string& exchSymbolCode) const {
    const auto [ret, symbolInfo] = getRecSymbolInfoByExchSymbolCode(
        marketCode, symbolType, exchSymbolCode);
    if (ret == 0) {
      return {0, symbolInfo->symbolCode};
    }
    LOG_W("Can not find symbol code of {} - {} - {}.", marketCode, symbolType,
          exchSymbolCode);
    return {SCODE_DB_CAN_NOT_FIND_SYM_CODE, ""};
  }

  std::tuple<int, std::string> getExchSymbolCode(
      const std::string& marketCode, const std::string& symbolCode) const {
    const auto [ret, symbolInfo] =
        getRecSymbolInfoBySymbolCode(marketCode, symbolCode);
    if (ret == 0) {
      return {0, symbolInfo->exchSymbolCode};
    }
    LOG_W("Can not find exch symbol code of {} - {}.", marketCode, symbolCode);
    return {SCODE_DB_CAN_NOT_FIND_EXCH_SYM_CODE, ""};
  }

 private:
  void initNecessaryDataStructures(
      const TBLRecSetSPtr<TBLSymbolInfo>& tblRecSet) final {
    std::size_t numOfSymbol = 0;
    {
      std::lock_guard<std::ext::spin_mutex> guard(mtxMIDXSymbolInfo_);
      for (const auto& tblRec : *tblRecSet) {
        auto symbolInfo = tblRec.second->getRecWithAllFields();
        symbolInfo->initHashInfo();
        midxSymbolInfo_.emplace(symbolInfo);
        LOG_T(fmt::format("Add rec {} - {} to tbl symbol info of cache.",
                          symbolInfo->marketCode, symbolInfo->symbolCode));
      }
      numOfSymbol = midxSymbolInfo_.size();
    }
    LOG_I("Load {} numbers of symbols from db.", numOfSymbol);
  }

  void handleTBLRecSetOfCompRet(
      const TBLRecSetSPtr<TBLSymbolInfo>& tblRecSetAdd,
      const TBLRecSetSPtr<TBLSymbolInfo>& tblRecSetDel,
      const TBLRecSetSPtr<TBLSymbolInfo>& tblRecSetChg) final {
    auto eraseRecFromMIDXSymbolInfo =
        [this](const TBLRecSPtr<TBLSymbolInfo>& tblRec) {
          auto& idx = midxSymbolInfo_.get<TagMarketCodeSymbolCode>();
          const auto iter =
              idx.find(tblRec->getRecWithAllFields()->hashOfMktSym_);
          if (iter != std::end(idx)) {
            idx.erase(iter);
          } else {
            const auto marketCode = tblRec->getRecWithAllFields()->marketCode;
            const auto symbolCode = tblRec->getRecWithAllFields()->symbolCode;
            LOG_W(
                "Handle tbl rec set of comp ret failed because of no rec in "
                "tbl symbol info of cache. marketCode = {}, symbolCode = {}",
                marketCode, symbolCode);
          }
        };

    {
      std::lock_guard<std::ext::spin_mutex> guard(mtxMIDXSymbolInfo_);
      for (const auto& rec : *tblRecSetAdd) {
        auto item = rec.second->getRecWithAllFields();
        item->initHashInfo();
        midxSymbolInfo_.emplace(item);
      }

      for (auto& rec : *tblRecSetDel) {
        rec.second->getRecWithAllFields()->initHashInfo();
        eraseRecFromMIDXSymbolInfo(rec.second);
      }

      for (const auto& rec : *tblRecSetChg) {
        eraseRecFromMIDXSymbolInfo(rec.second);
        auto item = rec.second->getRecWithAllFields();
        item->initHashInfo();
        midxSymbolInfo_.emplace(item);
      }
    }
  }

 private:
  MIDXSymbolInfo midxSymbolInfo_;
  mutable std::ext::spin_mutex mtxMIDXSymbolInfo_;
};

}  // namespace bq::db
