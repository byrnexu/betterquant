/*!
 * \file WSCliOfExch.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "WebDef.hpp"
#include "def/BQConst.hpp"
#include "def/BQDef.hpp"
#include "util/Pch.hpp"

namespace bq {
struct AssetInfo;
using AssetInfoSPtr = std::shared_ptr<AssetInfo>;
struct OrderInfo;
using OrderInfoSPtr = std::shared_ptr<OrderInfo>;
template <typename Task>
class TaskDispatcher;
template <typename Task>
using TaskDispatcherSPtr = std::shared_ptr<TaskDispatcher<Task>>;
}  // namespace bq

namespace bq::td {
struct WSCliAsyncTaskArg;
using WSCliAsyncTaskArgSPtr = std::shared_ptr<WSCliAsyncTaskArg>;
}  // namespace bq::td

namespace bq::td::svc {

class TDSvc;

class WSCliOfExch;
using WSCliOfExchSPtr = std::shared_ptr<WSCliOfExch>;

class WSCliOfExch {
 public:
  WSCliOfExch(const WSCliOfExch&) = delete;
  WSCliOfExch& operator=(const WSCliOfExch&) = delete;
  WSCliOfExch(const WSCliOfExch&&) = delete;
  WSCliOfExch& operator=(const WSCliOfExch&&) = delete;

  explicit WSCliOfExch(TDSvc* tdSvc);

 public:
  int init();

 private:
  int initWSCli();
  int initTaskDispatcher();

 public:
  int start();
  void stop();

 private:
  void OnWSCliOpen(web::WSCli* wsCli,
                   const web::ConnMetadataSPtr& connMetadata);
  virtual void onBeforeOpen(web::WSCli* wsCli,
                            const web::ConnMetadataSPtr& connMetadata) {}

  void OnWSCliMsg(web::WSCli* wsCli, const web::ConnMetadataSPtr& connMetadata,
                  const web::MsgSPtr& msg);

  std::tuple<int, WSCliAsyncTaskSPtr> makeAsyncTask(
      const web::TaskFromSrvSPtr& task);

  virtual WSCliAsyncTaskArgSPtr MakeWSCliAsyncTaskArg(
      const web::TaskFromSrvSPtr& task) const = 0;

 private:
  void handleAsyncTask(WSCliAsyncTaskSPtr& asyncTask);

 private:
  void handleOrder(WSCliAsyncTaskSPtr& asyncTask);
  void handleSyncUnclosedOrder(WSCliAsyncTaskSPtr& asyncTask);
  void handleSyncAssetsUpdate(WSCliAsyncTaskSPtr& asyncTask);

 private:
  virtual OrderInfoSPtr makeOrderInfoFromExch(
      WSCliAsyncTaskSPtr& asyncTask) = 0;
  virtual std::vector<AssetInfoSPtr> makeAssetsUpdate(
      WSCliAsyncTaskSPtr& asyncTask) {
    return std::vector<AssetInfoSPtr>();
  }

 public:
  web::WSCliSPtr getWSCli() const { return wsCli_; }
  TaskDispatcherSPtr<web::TaskFromSrvSPtr> getTaskDispatcher() const {
    return taskDispatcher_;
  }

 protected:
  TDSvc* tdSvc_;

  web::WSCliSPtr wsCli_{nullptr};
  TaskDispatcherSPtr<web::TaskFromSrvSPtr> taskDispatcher_{nullptr};
};

}  // namespace bq::td::svc
