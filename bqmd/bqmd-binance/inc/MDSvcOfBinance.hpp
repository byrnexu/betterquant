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
