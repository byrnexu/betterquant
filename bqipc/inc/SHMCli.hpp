/*!
 * \file SHMCli.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "SHMIPCBase.hpp"
#include "SHMIPCMsgId.hpp"
#include "util/StdExt.hpp"

namespace bq {

class SHMCli : public SHMIPCBase {
 public:
  SHMCli(const SHMCli&) = delete;
  SHMCli& operator=(const SHMCli&) = delete;
  SHMCli(const SHMCli&&) = delete;
  SHMCli& operator=(const SHMCli&&) = delete;

  using SHMIPCBase::SHMIPCBase;

 public:
  void setClientChannel(ClientChannel clientChannel);

 private:
  void beforeInit() final;
  void afterInit() final;

 private:
  void beforeUninit() final;

 public:
  void asyncSendReqWithZeroCopy(const FillSHMBufCallback& fillSHMBufCallback,
                                MsgId msgId, std::size_t shmBufLenOfReq);
  void asyncSendMsgWithZeroCopy(const FillSHMBufCallback& fillSHMBufCallback,
                                MsgId msgId, std::size_t shmBufLenOfMsg);

 private:
  void beforeAsyncSendReq(void* data, MsgId msgId);

 private:
  void writeRawDataToSHMWithZeroCopy(
      const FillSHMBufCallback& fillSHMBufCallback, MsgId msgId,
      std::size_t shmBufLen);

 private:
  std::string publisherName_;
  std::ext::spin_mutex mtxWriteRawDataToSHM_;
  iox::popo::UntypedPublisher* publisher_{nullptr};
};

}  // namespace bq
