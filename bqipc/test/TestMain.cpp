/*!
 * \file TestMain.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "SHMCli.hpp"
#include "SHMSrv.hpp"
#include "util/Datetime.hpp"
#include "util/Logger.hpp"

using namespace bq;

class global_event : public testing::Environment {
 public:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

struct TestData {
  SHMHeader header;
  std::uint64_t no_{0};
  char data[128];
};

SHMSrvSPtr shmSrv;
SHMCliSPtr shmCli;

int timesOfSrvRecv = 0;
std::multiset<std::uint64_t> timeUsedOfSrvRecvGroup;

int timesOfCliRecv = 0;
std::multiset<std::uint64_t> timeUsedOfCliRecvGroup;

void onSHMSrvDataRecv(const void* shmBufOfReq, std::size_t shmBufLenOfReq) {
  const auto testData = static_cast<const TestData*>(shmBufOfReq);
  const auto now = GetTotalNSSince1970();
  auto td = now - testData->header.timestamp_;
  timeUsedOfSrvRecvGroup.emplace(td);
  if (++timesOfSrvRecv % 1000 == 0) {
    LOG_I("server {} recv: times: {} no: {}", testData->header.clientChannel_,
          timesOfSrvRecv, testData->no_);
  }

  const auto header = static_cast<const SHMHeader*>(shmBufOfReq);
  shmSrv->sendRspWithZeroCopy(
      [&](void* shmBufOfRsp) {
        memcpy(shmBufOfRsp, shmBufOfReq, sizeof(TestData));
      },
      header, sizeof(TestData));
}

void onSHMCliDataRecv(const void* shmBufOfRsp, std::size_t shmBufLenOfRsp) {
  const auto testData = static_cast<const TestData*>(shmBufOfRsp);
  const auto now = GetTotalNSSince1970();
  const auto td = now - testData->header.timestamp_;
  timeUsedOfCliRecvGroup.emplace(td);
  if (++timesOfCliRecv % 1000 == 0) {
    LOG_I("client {} recv: times: {} no: {}", testData->header.clientChannel_,
          timesOfCliRecv, testData->no_);
  }
}

void statics(const std::string& hint,
             const std::multiset<std::uint64_t>& timeUsedGroup) {
  double total = 0;
  std::uint64_t max = 0;
  std::uint64_t min = UINT64_MAX;
  for (auto td : timeUsedGroup) {
    total += td;
    if (td > max) max = td;
    if (td < min) min = td;
  }
  const auto iter =
      std::next(std::begin(timeUsedGroup), timeUsedGroup.size() / 2);
  const auto med = *iter;
  LOG_I("{} total num: {}; avg: {}; med: {}; min: {}; max: {}", hint,
        timeUsedGroup.size(), total / double(timeUsedGroup.size()), med, min,
        max);
}

TEST(test, testSHMIPC) {
  return;
  shmSrv = std::make_shared<SHMSrv>(
      "TDFront@TDService@StgEngChannel@Trade",
      [](const auto* shmBufOfReq, auto shmBufLenOfReq) {
        onSHMSrvDataRecv(shmBufOfReq, shmBufLenOfReq);
      });
  shmSrv->start();
  std::this_thread::sleep_for(std::chrono::seconds(1));

  shmCli = std::make_shared<SHMCli>(
      "TDFront@TDService@StgEngChannel@Trade",
      [](const auto* shmBufOfRsp, auto shmBufLenOfRsp) {
        onSHMCliDataRecv(shmBufOfRsp, shmBufLenOfRsp);
      });
  shmCli->setClientChannel(1);
  shmCli->start();
  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::vector<std::shared_ptr<std::thread>> threadGroup;
  for (int i = 0; i < 10; ++i) {
    threadGroup.emplace_back(std::make_shared<std::thread>([&]() {
      for (int no = 0; no < 10000; ++no) {
        shmCli->asyncSendReqWithZeroCopy(
            [no](void* shmBufOfReq) {
              auto testData = static_cast<TestData*>(shmBufOfReq);
              testData->header.timestamp_ = GetTotalNSSince1970();
              testData->no_ = no;
            },
            0, sizeof(TestData));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }));
  }

  for (auto& thread : threadGroup) {
    if (thread->joinable()) thread->join();
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  shmCli->stop();
  shmSrv->stop();
  statics("server", timeUsedOfSrvRecvGroup);
  statics("client", timeUsedOfCliRecvGroup);
}

int main(int argc, char** argv) {
  testing::AddGlobalTestEnvironment(new global_event);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
