/*!
 * \file TDSvcOfBinance.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "TDSvcOfBinance.hpp"

#include "Config.hpp"
#include "HttpCliOfExchBinance.hpp"
#include "TDSvcDef.hpp"
#include "TDSvcUtil.hpp"
#include "WSCliOfExchBinance.hpp"
#include "util/Literal.hpp"
#include "util/Logger.hpp"
#include "util/ScheduleTaskBundle.hpp"
#include "util/TaskDispatcher.hpp"

namespace bq::td::svc::binance {

int TDSvcOfBinance::beforeInit() {
  const auto httpCliOfExch = std::make_shared<HttpCliOfExchBinance>(this);
  setHttpCliOfExch(httpCliOfExch);

  const auto wsCliOfExch = std::make_shared<WSCliOfExchBinance>(this);
  setWSCliOfExch(wsCliOfExch);

  return 0;
}

void TDSvcOfBinance::doInitScheduleTaskBundle() {
  getScheduleTaskBundle()->emplace_back(std::make_shared<ScheduleTask>(
      "testOrder",
      [this]() {
        auto asyncTask = MakeTDSrvSignal(MSG_ID_ON_TEST_ORDER);
        const auto ret = getTDSrvTaskDispatcher()->dispatch(asyncTask);
        return ret == 0 ? true : false;
      },
      ExecAtStartup::False, MilliSecInterval(1000), ExecTimes(0)));

  getScheduleTaskBundle()->emplace_back(std::make_shared<ScheduleTask>(
      "testCancelOrder",
      [this]() {
        auto asyncTask = MakeTDSrvSignal(MSG_ID_ON_TEST_CANCEL_ORDER);
        const auto ret = getTDSrvTaskDispatcher()->dispatch(asyncTask);
        return ret == 0 ? true : false;
      },
      ExecAtStartup::False, MilliSecInterval(5000), ExecTimes(0)));
}

std::tuple<int, std::string> TDSvcOfBinance::getAddrOfWS() {
  const auto [ret, listenKey] = getHttpCliOfExch()->getListenKey();
  if (ret != 0) {
    LOG_W("Get addr of ws failed.");
    return {ret, ""};
  }
  const auto urlBase = CONFIG["addrOfWS"].as<std::string>();
  const auto addrOfWS = fmt::format("{}/{}", urlBase, listenKey);
  return {0, addrOfWS};
}

}  // namespace bq::td::svc::binance
