/*!
 * \file TDSrvRiskPluginFlowCtrl.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "TDSrvRiskPlugin.hpp"
#include "util/Pch.hpp"

namespace bq {
struct OrderInfo;
using OrderInfoSPtr = std::shared_ptr<OrderInfo>;
class FlowCtrlSvc;
using FlowCtrlSvcSPtr = std::shared_ptr<FlowCtrlSvc>;
}  // namespace bq

namespace bq::td::srv {

class TDSrv;

class BOOST_SYMBOL_VISIBLE TDSrvRiskPluginFlowCtrl : public TDSrvRiskPlugin {
 public:
  TDSrvRiskPluginFlowCtrl(const TDSrvRiskPluginFlowCtrl&) = delete;
  TDSrvRiskPluginFlowCtrl& operator=(const TDSrvRiskPluginFlowCtrl&) = delete;
  TDSrvRiskPluginFlowCtrl(const TDSrvRiskPluginFlowCtrl&&) = delete;
  TDSrvRiskPluginFlowCtrl& operator=(const TDSrvRiskPluginFlowCtrl&&) = delete;

  explicit TDSrvRiskPluginFlowCtrl(TDSrv* tdSrv);

 private:
  boost::dll::fs::path getLocation() const final;

 private:
  int doOnOrder(const OrderInfoSPtr& order) final;
  int doOnCancelOrder(const OrderInfoSPtr& order) final;

 private:
  int doOnOrderRet(const OrderInfoSPtr& order) final;
  int doOnCancelOrderRet(const OrderInfoSPtr& order) final;

 private:
  FlowCtrlSvcSPtr flowCtrlSvc_{nullptr};
};

}  // namespace bq::td::srv

bq::td::srv::TDSrvRiskPluginFlowCtrl* Create(bq::td::srv::TDSrv* tdSrv) {
  return new bq::td::srv::TDSrvRiskPluginFlowCtrl(tdSrv);
}

BOOST_DLL_ALIAS_SECTIONED(Create, CreatePlugin, PlugIn)
