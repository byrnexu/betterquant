#pragma once

#include "def/Def.hpp"
#include "util/Pch.hpp"

namespace bq::md::svc {

class MDSvc;

class SHMSrvMsgHandler;
using SHMSrvMsgHandlerSPtr = std::shared_ptr<SHMSrvMsgHandler>;

class SHMSrvMsgHandler {
 public:
  SHMSrvMsgHandler(const SHMSrvMsgHandler&) = delete;
  SHMSrvMsgHandler& operator=(const SHMSrvMsgHandler&) = delete;
  SHMSrvMsgHandler(const SHMSrvMsgHandler&&) = delete;
  SHMSrvMsgHandler& operator=(const SHMSrvMsgHandler&&) = delete;
  SHMSrvMsgHandler(MDSvc* mdSvc) : mdSvc_(mdSvc) {}

  void handleReq(const void* shmBuf, std::size_t shmBufLen) {
    doHandleReq(shmBuf, shmBufLen);
  }

 private:
  virtual void doHandleReq(const void* shmBuf, std::size_t shmBufLen) {}

 private:
  MDSvc* mdSvc_{nullptr};
};

}  // namespace bq::md::svc
