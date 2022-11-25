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

class WebSrv : public SvcBase {
 public:
  using SvcBase::SvcBase;

 private:
  int prepareInit() final;
  int doInit() final;

 private:
  int initDBEng();

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

 private:
  db::DBEngSPtr dbEng_{nullptr};
  std::shared_ptr<std::thread> threadDrogon_;
};

}  // namespace bq
