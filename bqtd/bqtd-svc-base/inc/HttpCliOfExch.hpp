/*!
 * \file HttpCliOfExch.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "def/BQDef.hpp"
#include "util/Pch.hpp"
#include "util/Util.hpp"

namespace bq {
struct AssetInfo;
using AssetInfoSPtr = std::shared_ptr<AssetInfo>;
}  // namespace bq

namespace bq {
struct OrderInfo;
using OrderInfoSPtr = std::shared_ptr<OrderInfo>;
}  // namespace bq

namespace bq::td::svc {
class TDSvc;
}

namespace bq::td::svc {

class HttpCliOfExch;
using HttpCliOfExchSPtr = std::shared_ptr<HttpCliOfExch>;

class HttpCliOfExch {
 public:
  HttpCliOfExch(const HttpCliOfExch&) = delete;
  HttpCliOfExch& operator=(const HttpCliOfExch&) = delete;
  HttpCliOfExch(const HttpCliOfExch&&) = delete;
  HttpCliOfExch& operator=(const HttpCliOfExch&&) = delete;

  explicit HttpCliOfExch(TDSvc* tdSvc) : tdSvc_(tdSvc) {}

 public:
  int order(const OrderInfoSPtr& orderInfo) { return doOrder(orderInfo); }

 private:
  virtual int doOrder(const OrderInfoSPtr& orderInfo) { return 0; }

 protected:
  void handleRspOfOrder(OrderInfoSPtr orderInfo, cpr::Response rsp);

 private:
  virtual std::tuple<bool, int, std::string> rspOfOrderIsFailed(
      const std::string& text) = 0;

 public:
  int cancelOrder(const OrderInfoSPtr& orderInfo) {
    return doCancelOrder(orderInfo);
  }

 private:
  virtual int doCancelOrder(const OrderInfoSPtr& orderInfo) { return 0; }

 protected:
  void handleRspOfCancelOrder(OrderInfoSPtr orderInfo, cpr::Response rsp);

 private:
  virtual std::tuple<bool, int, std::string> rspOfCancelOrderIsFailed(
      const std::string& text) = 0;

 public:
  std::tuple<int, std::string> getListenKey() { return doGetListenKey(); }

 private:
  virtual std::tuple<int, std::string> doGetListenKey() { return {0, ""}; }

 public:
  void extendConnLifecycle() { doExtendConnLifecycle(); }

 private:
  virtual void doExtendConnLifecycle() {}

 public:
  void syncAssetsSnapshot();

 private:
  virtual std::vector<AssetInfoSPtr> doSyncAssetsSnapshot() {
    return std::vector<AssetInfoSPtr>();
  }

 public:
  void syncUnclosedOrderInfo(SHMIPCAsyncTaskSPtr& asyncTask);

 private:
  virtual OrderInfoSPtr doSyncUnclosedOrderInfo(
      SHMIPCAsyncTaskSPtr& asyncTask) {
    return nullptr;
  }

 public:
  void testOrder() { return doTestOrder(); }

 private:
  virtual void doTestOrder() {}

 public:
  void testCancelOrder() { return doTestCancelOrder(); }

 private:
  virtual void doTestCancelOrder() {}

 protected:
  TDSvc* tdSvc_;
};

};  // namespace bq::td::svc
