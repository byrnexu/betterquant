/*!
 * \file WSCliOfExchBinance.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "WSCliOfExch.hpp"

namespace bq::md::svc::binance {

class BooksCache;
using BooksCacheSPtr = std::shared_ptr<BooksCache>;

class WSCliOfExchBinance;
using WSCliOfExchBinanceSPtr = std::shared_ptr<WSCliOfExchBinance>;

class WSCliOfExchBinance : public WSCliOfExch {
 public:
  WSCliOfExchBinance(const WSCliOfExchBinance&) = delete;
  WSCliOfExchBinance& operator=(const WSCliOfExchBinance&) = delete;
  WSCliOfExchBinance(const WSCliOfExchBinance&&) = delete;
  WSCliOfExchBinance& operator=(const WSCliOfExchBinance&&) = delete;

  explicit WSCliOfExchBinance(MDSvc* mdSvc);

 private:
  void onBeforeOpen(web::WSCli* wsCli,
                    const web::ConnMetadataSPtr& connMetadata) final;

  WSCliAsyncTaskArgSPtr MakeWSCliAsyncTaskArg(
      const web::TaskFromSrvSPtr& task) const final;

 private:
  std::string handleMDTrades(WSCliAsyncTaskSPtr& asyncTask) final;
  std::string handleMDTickers(WSCliAsyncTaskSPtr& asyncTask) final;
  std::string handleMDCandle(WSCliAsyncTaskSPtr& asyncTask) final;
  std::string handleMDBooks(WSCliAsyncTaskSPtr& asyncTask) final;
  bool isSubOrUnSubRet(WSCliAsyncTaskSPtr& asyncTask) final;

 private:
  BooksCacheSPtr booksCache_{nullptr};
};

}  // namespace bq::md::svc::binance
