/*!
 * \file SHMSrv.hpp
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

struct SafePublisher {
  iox::popo::UntypedPublisher* publisher_{nullptr};
  std::ext::spin_mutex mtxWriteRawDataToSHM_;
};
using SafePublisherSPtr = std::shared_ptr<SafePublisher>;

class SHMSrv : public SHMIPCBase {
 public:
  SHMSrv(const SHMSrv&) = delete;
  SHMSrv& operator=(const SHMSrv&) = delete;
  SHMSrv(const SHMSrv&&) = delete;
  SHMSrv& operator=(const SHMSrv&&) = delete;

  using SHMIPCBase::SHMIPCBase;

 private:
  void beforeInit() final;

 private:
  void beforeUninit() final;

 public:
  void sendRspWithZeroCopy(const FillSHMBufCallback& fillSHMBufCallback,
                           const SHMHeader* reqHeader,
                           std::size_t shmBufLenOfRsp);
  void pushMsgWithZeroCopy(const FillSHMBufCallback& fillSHMBufCallback,
                           ClientChannel clientChannel, MsgId msgId,
                           std::size_t shmBufLenOfMsg);

 public:
  void sendRsp(const SHMHeader* reqHeader, void* data, std::size_t len);

 private:
  void beforeSendRsp(const SHMHeader* reqHeader, void* data);

 public:
  void pushMsg(ClientChannel clientChannel, MsgId msgId, void* data,
               std::size_t len);

 private:
  void beforePushMsg(ClientChannel clientChannel, MsgId msgId, void* data);

 private:
  SafePublisherSPtr getSafePublisher(ClientChannel clientChannel);
  void writeRawDataToSHM(iox::popo::UntypedPublisher* const publisher,
                         void* data, std::size_t len);

 private:
  std::array<SafePublisherSPtr, std::numeric_limits<ClientChannel>::max()>
      safePublisherGroup_;
  std::ext::spin_mutex mtxPubliserGroup_;
};

}  // namespace bq
