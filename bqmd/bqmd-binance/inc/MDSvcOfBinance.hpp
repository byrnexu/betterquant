/*!
 * \file MDSvcOfBinance.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "MDSvc.hpp"

namespace bq::md::svc::binance {

class MDSvcOfBinance : public MDSvc {
 public:
  using MDSvc::MDSvc;

 private:
  int beforeInit() final;
};

}  // namespace bq::md::svc::binance
