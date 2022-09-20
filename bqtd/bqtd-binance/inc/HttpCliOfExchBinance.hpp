/*!
 * \file HttpCliOfExchBinance.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "HttpCliOfExch.hpp"

namespace bq {
struct JsonData;
using JsonDataSPtr = std::shared_ptr<JsonData>;
}  // namespace bq

namespace bq::db {
template <typename TableSchema>
class TBLRec;
template <typename TableSchema>
using TBLRecSPtr = std::shared_ptr<TBLRec<TableSchema>>;
template <typename TableSchema>
using TBLRecSet = std::map<std::string, TBLRecSPtr<TableSchema>>;
template <typename TableSchema>
using TBLRecSetSPtr = std::shared_ptr<TBLRecSet<TableSchema>>;
}  // namespace bq::db

namespace bq::db::trdSymbol {
struct FieldGroupOfAll;
using Record = FieldGroupOfAll;
using RecordSPtr = std::shared_ptr<Record>;
using RecordWPtr = std::weak_ptr<Record>;
}  // namespace bq::db::trdSymbol

namespace bq::td::svc::binance {

class HttpCliOfExchBinance;
using HttpCliOfExchBinanceSPtr = std::shared_ptr<HttpCliOfExchBinance>;

class HttpCliOfExchBinance : public HttpCliOfExch {
 public:
  HttpCliOfExchBinance(const HttpCliOfExchBinance&) = delete;
  HttpCliOfExchBinance& operator=(const HttpCliOfExchBinance&) = delete;
  HttpCliOfExchBinance(const HttpCliOfExchBinance&&) = delete;
  HttpCliOfExchBinance& operator=(const HttpCliOfExchBinance&&) = delete;

  using HttpCliOfExch::HttpCliOfExch;

 private:
  int doOrder(const OrderInfoSPtr& orderInfo) final;
  std::tuple<bool, int, std::string> rspOfOrderIsFailed(
      const std::string& text) final;

 private:
  int doCancelOrder(const OrderInfoSPtr& orderInfo) final;
  std::tuple<bool, int, std::string> rspOfCancelOrderIsFailed(
      const std::string& text) final;

 private:
  std::tuple<int, std::string> doGetListenKey() final;

 private:
  void doExtendConnLifecycle() final;
  void handleRspOfExtendConnLifecycle(cpr::Response rsp);

 private:
  std::vector<AssetInfoSPtr> doSyncAssetsSnapshot() final;
  JsonDataSPtr queryAssetInfoGroup();

  std::tuple<int, std::vector<AssetInfoSPtr>> makeAssetInfoGroupOfSpot(
      const JsonDataSPtr& jsonData);

  std::tuple<int, std::vector<AssetInfoSPtr>> makeAssetInfoGroupUBasedContracts(
      const JsonDataSPtr& jsonData);

  std::tuple<int, std::vector<AssetInfoSPtr>> makeAssetInfoGroupCBasedContracts(
      const JsonDataSPtr& jsonData);

 private:
  OrderInfoSPtr doSyncUnclosedOrderInfo(SHMIPCAsyncTaskSPtr& asyncTask) final;

  JsonDataSPtr queryOrderInfo(const OrderInfoSPtr& orderInfo);
  OrderInfoSPtr makeOrderInfoFromExchOfSpot(
      const OrderInfoSPtr& orderInfoInOrdMgr, const JsonDataSPtr& jsonData);
  OrderInfoSPtr makeOrderInfoFromExchOfUBasedContracts(
      const OrderInfoSPtr& orderInfoInOrdMgr, const JsonDataSPtr& jsonData);
  OrderInfoSPtr makeOrderInfoFromExchOfCBasedContracts(
      const OrderInfoSPtr& orderInfoInOrdMgr, const JsonDataSPtr& jsonData);

 private:
  void doTestOrder() final;
  void doTestOrderOfSpot();
  void doTestOrderOfUBasedContracts();
  void doTestOrderOfCBasedContracts();
  void doTestCancelOrder() final;

 private:
  std::string listenKey_;
};

}  // namespace bq::td::svc::binance
