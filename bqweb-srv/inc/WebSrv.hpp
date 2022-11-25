/*!
 * \file WebSrv.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/20
 *
 * \brief
 */

#pragma once

#include "SHMIPCDef.hpp"
#include "db/DBEngDef.hpp"
#include "util/Pch.hpp"
#include "util/SvcBase.hpp"

namespace bq {

template <typename Task>
class TaskDispatcher;
template <typename Task>
using TaskDispatcherSPtr = std::shared_ptr<TaskDispatcher<Task>>;

class WebSrv : public SvcBase, public boost::serialization::singleton<WebSrv> {
 public:
  using SvcBase::SvcBase;

 private:
  int prepareInit() final;
  int doInit() final;

 private:
  int initDBEng();
  int initWebSrvTaskDispatcher();
  void initSHMSrv();

 public:
  int doRun() final;

 private:
  void startDrogon();

 private:
  void doExit(const boost::system::error_code* ec, int signalNum) final;

 private:
  void stopDrogon();

 public:
  db::DBEngSPtr getDBEng() const { return dbEng_; }

  TaskDispatcherSPtr<SHMIPCTaskSPtr> getWebSrvTaskDispatcher() const {
    return webSrvTaskDispatcher_;
  }

  SHMSrvSPtr getSHMSrvOfStgEng() const { return shmSrvOfStgEng_; }

 private:
  db::DBEngSPtr dbEng_{nullptr};

  TaskDispatcherSPtr<SHMIPCTaskSPtr> webSrvTaskDispatcher_{nullptr};
  SHMSrvSPtr shmSrvOfStgEng_{nullptr};

  std::shared_ptr<std::thread> threadDrogon_;
};

}  // namespace bq
