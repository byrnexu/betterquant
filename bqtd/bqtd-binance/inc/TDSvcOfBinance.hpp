/*!
 * \file TDSvcOfBinance.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "TDSvc.hpp"

namespace bq::td::svc::binance {

class TDSvcOfBinance : public TDSvc {
 public:
  using TDSvc::TDSvc;

 private:
  void doInitScheduleTaskBundle() final;

 private:
  int beforeInit() final;

 private:
  std::tuple<int, std::string> getAddrOfWS() final;
};

}  // namespace bq::td::svc::binance
