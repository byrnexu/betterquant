/*!
 * \file SHMCli.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "SHMCli.hpp"

#include "SHMHeader.hpp"
#include "SHMIPCConst.hpp"
#include "SHMIPCMsgId.hpp"
#include "SHMUtil.hpp"
#include "util/Datetime.hpp"
#include "util/Logger.hpp"

namespace bq {

void SHMCli::setClientChannel(ClientChannel clientChannel) {
  clientChannel_ = clientChannel;
}

void SHMCli::beforeInit() {
  auto waitForSubscriberToBeVisible =
      [&](iox::popo::UntypedPublisher* publisher) {
        int i = 0;
        while (true) {
          if (++i > TIMES_OF_WAIT_FOR_SUBSCRIBER) {
            LOG_I("Subscriber is not ready after {} times of attempts.", i);
            break;
          }
          if (publisher->hasSubscribers()) {
            LOG_D("Try {} times.", i);
            break;
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
      };

  publisher_ = new iox::popo::UntypedPublisher(
      {iox::capro::IdString_t(iox::cxx::TruncateToCapacity, service_),
       iox::capro::IdString_t(iox::cxx::TruncateToCapacity, instance_),
       iox::capro::IdString_t(iox::cxx::TruncateToCapacity, event_)},
      makePublisherOptions());
  publisher_->offer();
  waitForSubscriberToBeVisible(publisher_);
  publisherName_ = fmt::format("{}{}{}{}{}", service_, SEP_OF_SHM_SVC,
                               instance_, SEP_OF_SHM_SVC, event_);
  LOG_I("Create publisher. {} [{}]", appName_, publisherName_);
}

void SHMCli::beforeUninit() {
  if (publisher_) {
    delete publisher_;
    publisher_ = nullptr;
  }
}

void SHMCli::afterInit() {
  LOG_I("Client {} [{}] connect to [{}{}{}{}{}].", appName_, publisherName_,
        service_, SEP_OF_SHM_SVC, instance_, SEP_OF_SHM_SVC, event_);
}

void SHMCli::asyncSendReqWithZeroCopy(
    const FillSHMBufCallback& fillSHMBufCallback, MsgId msgId,
    std::size_t shmBufLenOfReq) {
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxWriteRawDataToSHM_);
    writeRawDataToSHMWithZeroCopy(fillSHMBufCallback, msgId, shmBufLenOfReq);
  }
}

void SHMCli::asyncSendMsgWithZeroCopy(
    const FillSHMBufCallback& fillSHMBufCallback, MsgId msgId,
    std::size_t shmBufLenOfMsg) {
  asyncSendReqWithZeroCopy(fillSHMBufCallback, msgId, shmBufLenOfMsg);
}

void SHMCli::beforeAsyncSendReq(void* data, MsgId msgId) {
  auto header = static_cast<SHMHeader*>(data);
  header->direction_ = Direction::Req;
  header->msgId_ = msgId;
  header->clientChannel_ = *clientChannel_;
  header->timestamp_ = GetTotalUSSince1970();
}

void SHMCli::writeRawDataToSHMWithZeroCopy(
    const FillSHMBufCallback& fillSHMBufCallback, MsgId msgId,
    std::size_t shmBufLen) {
  publisher_->loan(shmBufLen)
      .and_then([&](auto& userPayload) {
        memset(userPayload, 0, shmBufLen);
        beforeAsyncSendReq(userPayload, msgId);
        fillSHMBufCallback(userPayload);
        publisher_->publish(userPayload);
      })
      .or_else([&](auto& error) {
        std::ostringstream oss;
        oss << error;
        LOG_E("Unable to loan shm. {} [{}] [{}]", appName_, publisherName_,
              oss.str());
      });
}

}  // namespace bq
