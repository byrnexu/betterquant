/*!
 * \file PosSnapshotImpl.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/09
 *
 * \brief
 */

#pragma once

#include <boost/python.hpp>

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
  int init(PyObject* stgInstTaskHandler);

 private:
  void installStgInstTaskHandler(PyObject* value);

 public:
  int run();

 public:
  void installStgInstTimer(StgInstId stgInstId, const std::string& timerName,
                           ExecAtStartup execAtStartUp,
                           std::uint32_t milliSecInterval,
                           std::uint64_t maxExecTimes);

 public:
  std::tuple<int, OrderId> order(const StgInstInfoSPtr& stgInstInfo,
                                 AcctId acctId, const std::string& symbolCode,
                                 Side side, PosSide posSide, Decimal orderPrice,
                                 Decimal orderSize);

  std::tuple<int, OrderId> order(OrderInfoSPtr& orderInfo);

  int cancelOrder(OrderId orderId);

  std::tuple<int, OrderInfoSPtr> getOrderInfo(OrderId orderId) const;

  int sub(StgInstId subscriber, const std::string& topic);
  int unSub(StgInstId subscriber, const std::string& topic);

  bool saveStgPrivateData(StgInstId stgInstId, const std::string& jsonStr);
  std::string loadStgPrivateData(StgInstId stgInstId);

  void saveToDB(const PnlSPtr& pnl);

 private:
  StgEngImplSPtr stgEngImpl_{nullptr};

  PyObject* stgInstTaskHandler_;
  mutable std::mutex mtxPY_;
};

}  // namespace bq::stg
