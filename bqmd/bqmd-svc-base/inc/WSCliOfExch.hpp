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
#include "util/TaskDispatcher.hpp"

namespace bq::md {
struct WSCliAsyncTaskArg;
using WSCliAsyncTaskArgSPtr = std::shared_ptr<WSCliAsyncTaskArg>;
}  // namespace bq::md

namespace bq::md::svc {

class MDSvc;

struct TopicGroupNeedMaint;
using TopicGroupNeedMaintSPtr = std::shared_ptr<TopicGroupNeedMaint>;

class WSCliOfExch;
using WSCliOfExchSPtr = std::shared_ptr<WSCliOfExch>;

class WSCliOfExch {
 public:
  WSCliOfExch(const WSCliOfExch&) = delete;
  WSCliOfExch& operator=(const WSCliOfExch&) = delete;
  WSCliOfExch(const WSCliOfExch&&) = delete;
  WSCliOfExch& operator=(const WSCliOfExch&&) = delete;

  explicit WSCliOfExch(MDSvc* mdSvc);

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
  virtual std::string handleMDTrades(WSCliAsyncTaskSPtr& asyncTask) = 0;
  virtual std::string handleMDTickers(WSCliAsyncTaskSPtr& asyncTask) = 0;
  virtual std::string handleMDCandle(WSCliAsyncTaskSPtr& asyncTask) = 0;
  virtual std::string handleMDBooks(WSCliAsyncTaskSPtr& asyncTask) = 0;

  void handleMDOthers(WSCliAsyncTaskSPtr& asyncTask);
  virtual bool isSubOrUnSubRet(WSCliAsyncTaskSPtr& asyncTask) = 0;

 private:
  void updateActiveTimeOfTopic(const std::string& topic);

 public:
  web::WSCliSPtr getWSCli() const { return wsCli_; }
  TaskDispatcherSPtr<web::TaskFromSrvSPtr> getTaskDispatcher() const {
    return taskDispatcher_;
  }

 protected:
  MDSvc* mdSvc_;

  web::WSCliSPtr wsCli_{nullptr};
  TaskDispatcherSPtr<web::TaskFromSrvSPtr> taskDispatcher_{nullptr};

  TopicGroupNeedMaintSPtr topicGroupNeedMaint_{nullptr};
  std::ext::spin_mutex mtxTopicGroupNeedMaint_;
};

}  // namespace bq::md::svc
