/*!
 * \file Web.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "Web.hpp"

#include "WSCli.hpp"
#include "WSTask.hpp"
#include "WebParam.hpp"
#include "util/Logger.hpp"

namespace bq::web {

std::tuple<int, WSCliSPtr> MakeWSCli(const bq::web::WSParamSPtr& wsParam,
                                     const CBOnWSCliMsg& cbOnWSCliMsg,
                                     const CBOnWSCliOpen& cbOnWSCliOpen,
                                     const CBOnWSCliClose& cbOnWSCliClose,
                                     const CBOnWSCliFail& cbOnWSCliFail,
                                     const PingPongSvcSPtr& pingPongSvc) {
  const auto wsCli =
      std::make_shared<WSCli>(wsParam, cbOnWSCliMsg, cbOnWSCliOpen,
                              cbOnWSCliClose, cbOnWSCliFail, pingPongSvc);
  return {0, wsCli};
}

std::tuple<int, WSCliSPtr> MakeWSCli(const std::string& wsParamInStrFmt,
                                     const CBOnWSCliMsg& cbOnWSCliMsg,
                                     const CBOnWSCliOpen& cbOnWSCliOpen,
                                     const CBOnWSCliClose& cbOnWSCliClose,
                                     const CBOnWSCliFail& cbOnWSCliFail,
                                     const PingPongSvcSPtr& pingPongSvc) {
  auto [ret, wsParam] = MakeWSParam(wsParamInStrFmt);
  if (ret != 0) {
    LOG_E("Make ws cli failed.");
    return {-1, nullptr};
  }
  return MakeWSCli(wsParam, cbOnWSCliMsg, cbOnWSCliOpen, cbOnWSCliClose,
                   cbOnWSCliFail, pingPongSvc);
}

}  // namespace bq::web
