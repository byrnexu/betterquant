#pragma once

#include "BQPub.hpp"
#include "Pub.hpp"
#include "SHMIPCPub.hpp"

namespace bq {
struct Pnl;
using PnlSPtr = std::shared_ptr<Pnl>;
}  // namespace bq

namespace bq::stg {

class StgEngImpl;
using StgEngImplSPtr = std::shared_ptr<StgEngImpl>;

class StgEng;
using StgEngSPtr = std::shared_ptr<StgEng>;

class StgInstTaskHandlerBase;
using StgInstTaskHandlerBaseSPtr = std::shared_ptr<StgInstTaskHandlerBase>;

class StgEng {
 public:
  StgEng(const StgEng&) = delete;
  StgEng& operator=(const StgEng&) = delete;
  StgEng(const StgEng&&) = delete;
  StgEng& operator=(const StgEng&&) = delete;

  explicit StgEng(const std::string& configFilename);

 public:
  int init();
  int run();

 public:
  void installStgInstTaskHandler(const StgInstTaskHandlerBaseSPtr& taskHandler);

 public:
  OrderInfoSPtr order(const StgInstInfoSPtr& stgInstInfo, AcctId acctId,
                      const std::string& symbolCode, Side side, PosSide posSide,
                      Decimal orderPrice, Decimal orderSize);

  OrderInfoSPtr order(OrderInfoSPtr& orderInfo);

  OrderInfoSPtr cancelOrder(OrderId orderId);

 public:
  MarketDataCacheSPtr getMarketDataCache() const;

 public:
  int sub(StgInstId subscriber, const std::string& topic);
  int unSub(StgInstId subscriber, const std::string& topic);

 public:
  void installStgInstTimer(StgInstId stgInstId, const std::string& timerName,
                           ExecAtStartup execAtStartUp,
                           std::uint32_t milliSecInterval,
                           std::uint64_t maxExecTimes = UINT64_MAX);

 public:
  bool saveStgPrivateData(StgInstId stgInstId, const std::string& jsonStr);
  std::string loadStgPrivateData(StgInstId stgInstId);

 public:
  void saveToDB(const PnlSPtr& pnl);

 private:
  StgEngImplSPtr stgEngImpl_{nullptr};
};

}  // namespace bq::stg
