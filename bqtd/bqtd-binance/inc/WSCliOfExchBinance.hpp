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

namespace bq::td::svc::binance {

class WSCliOfExchBinance;
using WSCliOfExchBinanceSPtr = std::shared_ptr<WSCliOfExchBinance>;

class WSCliOfExchBinance : public WSCliOfExch {
 public:
  using WSCliOfExch::WSCliOfExch;

 private:
  void onBeforeOpen(web::WSCli* wsCli,
                    const web::ConnMetadataSPtr& connMetadata) final;

 private:
  WSCliAsyncTaskArgSPtr MakeWSCliAsyncTaskArg(
      const web::TaskFromSrvSPtr& task) const final;

 private:
  std::vector<AssetInfoSPtr> makeAssetsUpdate(
      WSCliAsyncTaskSPtr& asyncTask) final;

 private:
  OrderInfoSPtr makeOrderInfoFromExch(WSCliAsyncTaskSPtr& asyncTask) final;

  OrderInfoSPtr makeOrderInfoFromExchOfSpot(WSCliAsyncTaskSPtr& asyncTask);
  OrderInfoSPtr makeOrderInfoFromExchOfContracts(WSCliAsyncTaskSPtr& asyncTask);
};

}  // namespace bq::td::svc::binance
