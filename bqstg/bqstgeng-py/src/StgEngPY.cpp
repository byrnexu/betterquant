/*!
 * \file PosSnapshotImpl.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/09
 *
 * \brief
 */

#include <boost/python.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/utility.hpp>
#include <chrono>
#include <cstring>
#include <memory>
#include <thread>

#include "ArrayIndexingSuite.hpp"
#include "ArrayRef.hpp"
#include "CXX2PYTuple.hpp"
#include "StgEng.hpp"
#include "StgEngImpl.hpp"
#include "StgInstTaskHandlerImpl.hpp"
#include "def/AssetInfo.hpp"
#include "def/BQConst.hpp"
#include "def/BQDef.hpp"
#include "def/Const.hpp"
#include "def/DataStruOfMD.hpp"
#include "def/DataStruOfOthers.hpp"
#include "def/DataStruOfStg.hpp"
#include "def/DataStruOfTD.hpp"
#include "def/Def.hpp"
#include "def/OrderInfo.hpp"
#include "def/PosInfo.hpp"
#include "def/SimedTDInfo.hpp"
#include "def/StgInstInfo.hpp"
#include "def/SymbolInfo.hpp"
#include "util/PosSnapshot.hpp"

using namespace boost::python;

namespace bq::stg {

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(queryPnlOverloads, queryPnl, 2, 4)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(queryPnlGroupByOverloads,
                                       queryPnlGroupBy, 2, 4)

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(orderOverloads, order, 7, 9)

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(queryHisMDBetween2TsOverloadsByFields,
                                       queryHisMDBetween2Ts, 6, 7)

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(
    querySpecificNumOfHisMDBeforeTsOverloadsByFields,
    querySpecificNumOfHisMDBeforeTs, 6, 7)

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(
    querySpecificNumOfHisMDAfterTsOverloadsByFields,
    querySpecificNumOfHisMDAfterTs, 6, 7)

BOOST_PYTHON_MODULE(bqstgeng) {
  // char array
  class_<array_ref<char>>("CharArray")
      .def(array_indexing_suite<array_ref<char>>());

  // enum ExecAtStartup
  enum_<ExecAtStartup>("ExecAtStartup")
      .value("IsTrue", ExecAtStartup::True)
      .value("IsFalse", ExecAtStartup::False);

  // enum MarketCode
  enum_<MarketCode>("MarketCode")
      .value("Okex", MarketCode::Okex)
      .value("Binance", MarketCode::Binance)
      .value("Ftx", MarketCode::Ftx)
      .value("Kraken", MarketCode::Kraken)
      .value("Others", MarketCode::Others);

  // enum SymbolType
  enum_<SymbolType>("SymbolType")
      .value("Spot", SymbolType::Spot)
      .value("Futures", SymbolType::Futures)
      .value("CFutures", SymbolType::CFutures)
      .value("Perp", SymbolType::Perp)
      .value("CPerp", SymbolType::CPerp)
      .value("Option", SymbolType::Option)
      .value("Index", SymbolType::index)
      .value("Others", SymbolType::Others);

  // enum Side
  enum_<Side>("Side")
      .value("Bid", Side::Bid)
      .value("Ask", Side::Ask)
      .value("Others", Side::Others);

  // enum PosSide
  enum_<PosSide>("PosSide")
      .value("Long", PosSide::Long)
      .value("Short", PosSide::Short)
      .value("Both", PosSide::Both)
      .value("Others", PosSide::Others);

  // enum OrderType
  enum_<OrderType>("OrderType")
      .value("Limit", OrderType::Limit)
      .value("Others", OrderType::Others);

  // enum OrderTypeExtra
  enum_<OrderTypeExtra>("OrderTypeExtra")
      .value("Normal", OrderTypeExtra::Normal)
      .value("MakeOnly", OrderTypeExtra::MakeOnly)
      .value("Ioc", OrderTypeExtra::Ioc)
      .value("Fok", OrderTypeExtra::Fok)
      .value("Others", OrderTypeExtra::Others);

  // enum MDType
  enum_<MDType>("MDType")
      .value("Trades", MDType::Trades)
      .value("Books", MDType::Books)
      .value("Tickers", MDType::Tickers)
      .value("Candle", MDType::Candle)
      .value("Others", MDType::Others);

  // enum OrderStatus
  enum_<OrderStatus>("OrderStatus")
      .value("Created", OrderStatus::Created)
      .value("ConfirmedInLocal", OrderStatus::ConfirmedInLocal)
      .value("Pending", OrderStatus::Pending)
      .value("ConfirmedByExch", OrderStatus::ConfirmedByExch)
      .value("PartialFilled", OrderStatus::PartialFilled)
      .value("Filled", OrderStatus::Filled)
      .value("Canceled", OrderStatus::Canceled)
      .value("PartialFilledCanceled", OrderStatus::PartialFilledCanceled)
      .value("Failed", OrderStatus::Failed)
      .value("Others", OrderStatus::Others);

  // enum LiquidityDirection
  enum_<LiquidityDirection>("LiquidityDirection")
      .value("Maker", LiquidityDirection::Maker)
      .value("Taker", LiquidityDirection::Taker);

  // StgInstInfo
  class_<StgInstInfo, std::shared_ptr<StgInstInfo>>("StgInstInfo", init<>())
      .def_readwrite("product_id", &StgInstInfo::productId_)
      .def_readwrite("stg_id", &StgInstInfo::stgId_)
      .def_readwrite("stg_name", &StgInstInfo::stgName_)
      .def_readwrite("user_id_of_author", &StgInstInfo::userIdOfAuthor_)
      .def_readwrite("stg_inst_id", &StgInstInfo::stgInstId_)
      .def_readwrite("stg_inst_params", &StgInstInfo::stgInstParams_)
      .def_readwrite("stg_inst_name", &StgInstInfo::stgInstName_)
      .def_readwrite("user_id", &StgInstInfo::userId_)
      .def_readwrite("is_del", &StgInstInfo::isDel_)
      .def("to_str", &StgInstInfo::toStr);

  // SHMHeader
  class_<SHMHeader>("SHMHeader", init<>())
      .def(init<std::uint16_t>())
      .def("to_str", &SHMHeader::toStr);

  // OrderInfo
  class_<OrderInfo, std::shared_ptr<OrderInfo>>("order_info", init<>())
      .def_readwrite("shm_header", &OrderInfo::shmHeader_)
      .def_readwrite("product_id", &OrderInfo::productId_)
      .def_readwrite("user_id", &OrderInfo::userId_)
      .def_readwrite("acct_id", &OrderInfo::acctId_)
      .def_readwrite("stg_id", &OrderInfo::stgId_)
      .def_readwrite("stg_inst_id", &OrderInfo::stgInstId_)
      .def_readwrite("algo_id", &OrderInfo::algoId_)
      .def_readwrite("order_id", &OrderInfo::orderId_)
      .def_readwrite("exch_order_id", &OrderInfo::exchOrderId_)
      .def_readwrite("parent_order_id", &OrderInfo::parentOrderId_)
      .def_readwrite("market_code", &OrderInfo::marketCode_)
      .def_readwrite("symbol_type", &OrderInfo::symbolType_)
      .add_property(
          "symbol_code",
          static_cast<array_ref<char> (*)(OrderInfo*)>(
              [](OrderInfo* obj) { return array_ref<char>(obj->symbolCode_); }),
          "Data bytes array of symbolcode")
      .add_property(
          "exch_symbol_code",
          static_cast<array_ref<char> (*)(OrderInfo*)>([](OrderInfo* obj) {
            return array_ref<char>(obj->exchSymbolCode_);
          }),
          "Data bytes array of exch symbolcode")
      .def_readwrite("side", &OrderInfo::side_)
      .def_readwrite("pos_side", &OrderInfo::posSide_)
      .def_readwrite("order_price", &OrderInfo::orderPrice_)
      .def_readwrite("order_size", &OrderInfo::orderSize_)
      .def_readwrite("par_value", &OrderInfo::parValue_)
      .def_readwrite("order_type", &OrderInfo::orderType_)
      .def_readwrite("order_type_extra", &OrderInfo::orderTypeExtra_)
      .def_readwrite("order_time", &OrderInfo::orderTime_)
      .def_readwrite("fee_", &OrderInfo::fee_)
      .add_property(
          "fee_currency",
          static_cast<array_ref<char> (*)(OrderInfo*)>([](OrderInfo* obj) {
            return array_ref<char>(obj->feeCurrency_);
          }),
          "Data bytes array of fee currency")
      .def_readwrite("deal_size", &OrderInfo::dealSize_)
      .def_readwrite("avg_deal_price", &OrderInfo::avgDealPrice_)
      .add_property(
          "last_trade_id",
          static_cast<array_ref<char> (*)(OrderInfo*)>([](OrderInfo* obj) {
            return array_ref<char>(obj->lastTradeId_);
          }),
          "Data bytes array of fee currency")
      .def_readwrite("last_deal_price", &OrderInfo::lastDealPrice_)
      .def_readwrite("last_deal_size", &OrderInfo::lastDealSize_)
      .def_readwrite("last_deal_time", &OrderInfo::lastDealTime_)
      .def_readwrite("order_status", &OrderInfo::orderStatus_)
      .def_readonly("no_used_to_calc_pos", &OrderInfo::noUsedToCalcPos_)
      .def_readwrite("status_code", &OrderInfo::statusCode_)
      .def("to_short_str", &OrderInfo::toShortStr);

  boost::python::def<OrderInfoSPtr (*)()>("make_order_info", MakeOrderInfo);

  // symbolInfo
  class_<SymbolInfo, std::shared_ptr<SymbolInfo>>(
      "SymbolInfo", init<MarketCode, SymbolType, std::string>())
      .def_readwrite("market_code", &SymbolInfo::marketCode_)
      .def_readwrite("symbol_code", &SymbolInfo::symbolCode_)
      .def_readwrite("symbol_type", &SymbolInfo::symbolType_);

  // symbolInfoGroup
  class_<std::vector<std::shared_ptr<SymbolInfo>>>("SymbolInfoGroup")
      .def(vector_indexing_suite<std::vector<std::shared_ptr<SymbolInfo>>,
                                 true>());

  // assetInfo
  class_<AssetInfo, std::shared_ptr<AssetInfo>>("AssetInfo", init<>())
      .def_readwrite("acct_id", &AssetInfo::acctId_)
      .def_readwrite("market_code", &AssetInfo::marketCode_)
      .def_readwrite("symbol_type", &AssetInfo::symbolType_)
      .add_property(
          "asset_name",
          static_cast<array_ref<char> (*)(AssetInfo*)>(
              [](AssetInfo* obj) { return array_ref<char>(obj->assetName_); }),
          "Data bytes array of asset name")
      .def_readwrite("vol", &AssetInfo::vol_)
      .def_readwrite("cross_vol", &AssetInfo::crossVol_)
      .def_readwrite("frozen", &AssetInfo::frozen_)
      .def_readwrite("available", &AssetInfo::available_)
      .def_readwrite("pnl_unreal", &AssetInfo::pnlUnreal_)
      .def_readwrite("max_withdraw", &AssetInfo::maxWithdraw_)
      .def_readwrite("updatetime", &AssetInfo::updateTime_)
      .def_readonly("key_hash", &AssetInfo::keyHash_)
      .def("to_str", &AssetInfo::toStr);

  // posInfo
  class_<PosInfo, std::shared_ptr<PosInfo>>("PosInfo", init<>())
      .def_readonly("key_hash", &PosInfo::keyHash_)
      .def_readwrite("product_id", &PosInfo::productId_)
      .def_readwrite("user_id", &PosInfo::userId_)
      .def_readwrite("acct_id", &PosInfo::acctId_)
      .def_readwrite("stg_id", &PosInfo::stgId_)
      .def_readwrite("stg_inst_id", &PosInfo::stgInstId_)
      .def_readwrite("algo_id", &PosInfo::algoId_)
      .def_readwrite("market_code", &PosInfo::marketCode_)
      .def_readwrite("symbol_type", &PosInfo::symbolType_)
      .add_property(
          "symbol_code",
          static_cast<array_ref<char> (*)(OrderInfo*)>(
              [](OrderInfo* obj) { return array_ref<char>(obj->symbolCode_); }),
          "Data bytes array of symbolcode")
      .def_readwrite("side", &PosInfo::side_)
      .def_readwrite("pos_side", &PosInfo::posSide_)
      .def_readwrite("par_value", &PosInfo::parValue_)
      .add_property(
          "fee_currency",
          static_cast<array_ref<char> (*)(OrderInfo*)>([](OrderInfo* obj) {
            return array_ref<char>(obj->feeCurrency_);
          }),
          "Data bytes array of fee currency")
      .def_readwrite("fee", &PosInfo::fee_)
      .def_readwrite("pos", &PosInfo::pos_)
      .def_readwrite("pre_pos", &PosInfo::prePos_)
      .def_readwrite("avg_open_price", &PosInfo::avgOpenPrice_)
      .def_readwrite("pnl_unreal", &PosInfo::pnlUnReal_)
      .def_readwrite("pnl_real", &PosInfo::pnlReal_)
      .def_readwrite("total_bid_size", &PosInfo::totalBidSize_)
      .def_readwrite("total_ask_size", &PosInfo::totalAskSize_)
      .def_readonly("last_no_used_to_calc_pos", &PosInfo::lastNoUsedToCalcPos_)
      .def_readwrite("updatetime", &PosInfo::updateTime_)
      .def("to_str", &PosInfo::toStr);

  // pnl
  class_<Pnl, std::shared_ptr<Pnl>>("Pnl", init<>())
      .def_readwrite("query_cond", &Pnl::queryCond_)
      .def_readwrite("pnl_un_real", &Pnl::pnlUnReal_)
      .def_readwrite("pnl_real", &Pnl::pnlReal_)
      .def_readwrite("fee", &Pnl::fee_)
      .def_readwrite("upate_time", &Pnl::updateTime_)
      .def_readwrite("quote_currency_for_calc", &Pnl::quoteCurrencyForCalc_)
      .def_readwrite("status_code", &Pnl::statusCode_)
      .def("to_str", &Pnl::toStr)
      .def("get_total_pnl", &Pnl::getTotalPnl)
      .def("is_valid", &Pnl::isValid);

  // PosInfoDetail
  class_<std::map<std::string, PosInfoSPtr>>("PosInfoDetail")
      .def(map_indexing_suite<std::map<std::string, PosInfoSPtr>, true>());

  // RetOfQueryPnl
  using RetOfQueryPnl = std::tuple<int, PnlSPtr>;
  class_<RetOfQueryPnl>("ret_of_query_pnl", init<int, PnlSPtr>())
      .def("__len__", &tuple_length<RetOfQueryPnl>)
      .def("__getitem__", &get_tuple_item<RetOfQueryPnl>);

  // Key2PnlGroup
  class_<Key2PnlGroup, std::shared_ptr<Key2PnlGroup>>("Key2PnlGroup")
      .def(map_indexing_suite<Key2PnlGroup, true>());

  // RetOfQueryPnlGroupBy
  using RetOfQueryPnlGroupBy = std::tuple<int, Key2PnlGroupSPtr>;
  class_<RetOfQueryPnlGroupBy>("ret_of_query_pnl_group_by",
                               init<int, Key2PnlGroupSPtr>())
      .def("__len__", &tuple_length<RetOfQueryPnlGroupBy>)
      .def("__getitem__", &get_tuple_item<RetOfQueryPnlGroupBy>);

  // PosInfoGroupSPtr
  class_<PosInfoGroup, std::shared_ptr<PosInfoGroup>>("PosInfoGroup")
      .def(vector_indexing_suite<PosInfoGroup, true>());

  // RetOfQUeryPosInfoGroup
  using RetOfQUeryPosInfoGroup = std::tuple<int, PosInfoGroupSPtr>;
  class_<RetOfQUeryPosInfoGroup>("ret_of_query_pos_info_group",
                                 init<int, PosInfoGroupSPtr>())
      .def("__len__", &tuple_length<RetOfQUeryPosInfoGroup>)
      .def("__getitem__", &get_tuple_item<RetOfQUeryPosInfoGroup>);

  // Key2PosInfoBundle
  class_<Key2PosInfoBundle, std::shared_ptr<Key2PosInfoBundle>>(
      "Key2PosInfoBundle")
      .def(map_indexing_suite<Key2PosInfoBundle, true>());

  // RetOfQueryPosInfoGroupBy
  using RetOfQueryPosInfoGroupBy = std::tuple<int, Key2PosInfoBundleSPtr>;
  class_<RetOfQueryPosInfoGroupBy>("ret_of_query_pos_info_group_by",
                                   init<int, Key2PosInfoBundleSPtr>())
      .def("__len__", &tuple_length<RetOfQueryPosInfoGroupBy>)
      .def("__getitem__", &get_tuple_item<RetOfQueryPosInfoGroupBy>);

  // AssetsUpdate
  class_<AssetsUpdate, std::shared_ptr<AssetsUpdate>>("AssetsUpdate")
      .def(map_indexing_suite<AssetsUpdate, true>());

  // posSnapshot
  class_<PosSnapshot, std::shared_ptr<PosSnapshot>, boost::noncopyable>(
      "PosSnapshot",
      init<std::map<std::string, PosInfoSPtr>, MarketDataCacheSPtr>())
      .def("get_pos_info_detail", &PosSnapshot::getPosInfoDetail,
           return_value_policy<reference_existing_object>())
      .def("query_pnl", &PosSnapshot::queryPnl,
           queryPnlOverloads(args("query_cond", "quote_currency_for_calc",
                                  "quote_currency_for_conv",
                                  "orig_quote_currency_of_ubased_contract")))
      .def("query_pnl_group_by", &PosSnapshot::queryPnlGroupBy,
           queryPnlGroupByOverloads(
               args("group_cond", "quote_currency_for_calc",
                    "quote_currency_for_conv",
                    "orig_quote_currency_of_ubased_contract")))
      .def("query_pos_info_group", &PosSnapshot::queryPosInfoGroup,
           args("query_cond"))
      .def("query_pos_info_group_by", &PosSnapshot::queryPosInfoGroupBy,
           args("query_cond"));

  using RetOfOrder = std::tuple<int, OrderId>;
  using OrderByFields = RetOfOrder (StgEng::*)(
      const StgInstInfoSPtr&, AcctId, const std::string&, Side, PosSide,
      Decimal, Decimal, AlgoId, const SimedTDInfoSPtr&);
  using OrderByOrderInfo = RetOfOrder (StgEng::*)(OrderInfoSPtr&);

  class_<RetOfOrder>("ret_of_order", init<int, OrderId>())
      .def("__len__", &tuple_length<RetOfOrder>)
      .def("__getitem__", &get_tuple_item<RetOfOrder>);

  using RetOfGetOrderInfo = std::tuple<int, OrderInfoSPtr>;
  class_<RetOfGetOrderInfo>("ret_of_get_order_info", init<int, OrderInfoSPtr>())
      .def("__len__", &tuple_length<RetOfGetOrderInfo>)
      .def("__getitem__", &get_tuple_item<RetOfGetOrderInfo>);

  using RetOfQryHisMD = std::tuple<int, std::string>;
  class_<RetOfQryHisMD>("ret_of_qry_his_md", init<int, std::string>())
      .def("__len__", &tuple_length<RetOfQryHisMD>)
      .def("__getitem__", &get_tuple_item<RetOfQryHisMD>);

  using QryHisMDBetweenTsByFields = RetOfQryHisMD (StgEng::*)(
      MarketCode marketCode, SymbolType symbolType,
      const std::string& symbolCode, MDType mdType, std::uint64_t tsBegin,
      std::uint64_t tsEnd, const std::string& ext);

  using QryHisMDBetweenTsByTopic = RetOfQryHisMD (StgEng::*)(
      const std::string& topic, std::uint64_t tsBegin, std::uint64_t tsEnd);

  using QryHisMDBeforeTsByFields = RetOfQryHisMD (StgEng::*)(
      MarketCode marketCode, SymbolType symbolType,
      const std::string& symbolCode, MDType mdType, std::uint64_t ts, int num,
      const std::string& ext);

  using QryHisMDBeforeTsByTopic = RetOfQryHisMD (StgEng::*)(
      const std::string& topic, std::uint64_t ts, int num);

  using QryHisMDAfterTsByFields = RetOfQryHisMD (StgEng::*)(
      MarketCode marketCode, SymbolType symbolType,
      const std::string& symbolCode, MDType mdType, std::uint64_t ts, int num,
      const std::string& ext);

  using QryHisMDAfterTsByTopic = RetOfQryHisMD (StgEng::*)(
      const std::string& topic, std::uint64_t ts, int num);

  // StgEng
  class_<StgEng, boost::noncopyable>("StgEng", init<std::string>())
      .def("init", &StgEng::init, args("stg_inst_task_handler"))
      .def("run", &StgEng::run)
      .def<OrderByFields>(
          "order", &StgEng::order,
          orderOverloads(args("stg_inst_info", "acct_id", "symbol_code", "side",
                              "pos_side", "order_price", "order_size",
                              "algo_id", "simed_td_info")))
      .def<OrderByOrderInfo>("order", &StgEng::order, args("order_info"))
      .def("cancel_order", &StgEng::cancelOrder, args("order_id"))
      .def("get_order_info", &StgEng::getOrderInfo, args("order_id"))
      .def("sub", &StgEng::sub, args("subscriber", "topic"))
      .def("unsub", &StgEng::sub, args("subscriber", "topic"))
      .def<QryHisMDBetweenTsByFields>(
          "query_his_md_between_2_ts", &StgEng::queryHisMDBetween2Ts,
          queryHisMDBetween2TsOverloadsByFields(
              args("market_code", "symbol_type", "symbol_code", "mdtype",
                   "ts_begin", "ts_end", "ext")))
      .def<QryHisMDBetweenTsByTopic>("query_his_md_between_2_ts",
                                     &StgEng::queryHisMDBetween2Ts,
                                     args("topic", "ts_begin", "ts_end"))
      .def<QryHisMDBeforeTsByFields>(
          "query_specific_num_of_his_md_before_ts",
          &StgEng::querySpecificNumOfHisMDBeforeTs,
          querySpecificNumOfHisMDBeforeTsOverloadsByFields(
              args("market_code", "symbol_type", "symbol_code", "mdtype", "ts",
                   "num", "ext")))
      .def<QryHisMDBeforeTsByTopic>("query_specific_num_of_his_md_before_ts",
                                    &StgEng::querySpecificNumOfHisMDBeforeTs,
                                    args("topic", "ts", "num"))
      .def<QryHisMDAfterTsByFields>(
          "query_specific_num_of_his_md_after_ts",
          &StgEng::querySpecificNumOfHisMDAfterTs,
          querySpecificNumOfHisMDAfterTsOverloadsByFields(
              args("market_code", "symbol_type", "symbol_code", "mdtype", "ts",
                   "num", "ext")))
      .def<QryHisMDAfterTsByTopic>("query_specific_num_of_his_md_after_ts",
                                   &StgEng::querySpecificNumOfHisMDAfterTs,
                                   args("topic", "ts", "num"))
      .def("install_stg_inst_timer", &StgEng::installStgInstTimer,
           args("stg_inst_id", "timer_name", "exec_at_startup",
                "millisec_interval", "max_exec_times"))
      .def("save_stg_private_data", &StgEng::saveStgPrivateData,
           args("stg_inst_id", "json_str"))
      .def("load_stg_private_data", &StgEng::loadStgPrivateData,
           args("stg_inst_id"))
      .def("save_to_db", &StgEng::saveToDB, args("pnl"));

  def("get_status_msg", GetStatusMsg, args("status_code"));

  // TransDetail
  class_<TransDetail, TransDetailSPtr>("TransDetail", init<>())
      .def(init<Decimal, Decimal, LiquidityDirection>(
          args("self", "slippage", "filled_per", "ld")))
      .def_readwrite("slippage", &TransDetail::slippage_)
      .def_readwrite("filled_per", &TransDetail::filledPer_)
      .def_readwrite("liquidity_direction", &TransDetail::liquidityDirection_)
      .def("to_str", &TransDetail::toStr);

  def("make_trans_detail", MakeTransDetail, args("trans_detail_in_json_fmt"));

  // TransDetailGroup
  class_<TransDetailGroup>("TransDetailGroup")
      .def(vector_indexing_suite<TransDetailGroup, true>());

  // SimedTDInfo
  class_<SimedTDInfo, SimedTDInfoSPtr>("SimedTDInfo", init<>())
      .def(init<OrderStatus, std::vector<TransDetailSPtr>>())
      .def_readwrite("order_status", &SimedTDInfo::orderStatus_)
      .def_readwrite("trans_detail_group", &SimedTDInfo::transDetailGroup_);

  // RetOfMakeSimedTDInfo
  using RetOfMakeSimedTDInfo = std::tuple<int, SimedTDInfoSPtr>;
  class_<RetOfMakeSimedTDInfo>("ret_of_make_simed_td_info",
                               init<int, SimedTDInfoSPtr>())
      .def("__len__", &tuple_length<RetOfMakeSimedTDInfo>)
      .def("__getitem__", &get_tuple_item<RetOfMakeSimedTDInfo>);
}

}  // namespace bq::stg
