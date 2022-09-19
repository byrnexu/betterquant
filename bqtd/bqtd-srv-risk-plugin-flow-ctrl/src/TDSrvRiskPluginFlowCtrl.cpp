#include "TDSrvRiskPluginFlowCtrl.hpp"

#include "AssetsMgr.hpp"
#include "ConfigOfPlugin.hpp"
#include "OrdMgr.hpp"
#include "PosMgr.hpp"
#include "TDSrv.hpp"
#include "db/TBLMonitorOfSymbolInfo.hpp"
#include "def/BQConst.hpp"
#include "def/BQDef.hpp"
#include "def/Const.hpp"
#include "def/Def.hpp"
#include "def/StatusCode.hpp"
#include "util/FlowCtrlSvc.hpp"
#include "util/Logger.hpp"
#include "util/Random.hpp"
#include "util/StdExt.hpp"

namespace bq::td::srv {

boost::dll::fs::path TDSrvRiskPluginFlowCtrl::getLocation() const {
  return boost::dll::this_line_location();
}

TDSrvRiskPluginFlowCtrl::TDSrvRiskPluginFlowCtrl(TDSrv* tdSrv)
    : TDSrvRiskPlugin(tdSrv) {
  flowCtrlSvc_ = std::make_shared<FlowCtrlSvc>(CONFIG_OF_PLUGIN);
}

int TDSrvRiskPluginFlowCtrl::doOnOrder(const OrderInfoSPtr& order) {
  L_I("[{}] On order req.", name());
  const auto taskName = fmt::format("{}-{}", order->acctId_,
                                    GetMsgName(order->shmHeader_.msgId_));
  if (flowCtrlSvc_->exceedFlowCtrl(taskName)) {
    return SCODE_TD_SRV_RISK_EXCEED_FLOW_CTRL;
  }
  return 0;
}

int TDSrvRiskPluginFlowCtrl::doOnCancelOrder(const OrderInfoSPtr& order) {
  L_I("[{}] On cancel order req.", name());
  return 0;
}

int TDSrvRiskPluginFlowCtrl::doOnOrderRet(const OrderInfoSPtr& order) {
  L_I("[{}] On order ret.", name());
  return 0;
}
int TDSrvRiskPluginFlowCtrl::doOnCancelOrderRet(const OrderInfoSPtr& order) {
  L_I("[{}] On cancel order ret.", name());
  return 0;
}

}  // namespace bq::td::srv
