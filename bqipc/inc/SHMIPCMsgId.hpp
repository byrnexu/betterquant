/*!
 * \file SHMIPCMsgId.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "util/PchBase.hpp"

namespace bq {

using MsgId = std::uint16_t;

constexpr static MsgId MSG_ID_ON_PUSH_TOPIC = 10000;

constexpr static MsgId MSG_ID_ON_MD_TRADES = 10001;
constexpr static MsgId MSG_ID_ON_MD_TICKERS = 10002;
constexpr static MsgId MSG_ID_ON_MD_CANDLE = 10003;
constexpr static MsgId MSG_ID_ON_MD_BOOKS = 10004;

constexpr static MsgId MSG_ID_ON_STG_START = 10051;
constexpr static MsgId MSG_ID_ON_STG_INST_START = 10052;
constexpr static MsgId MSG_ID_ON_STG_INST_ADD = 10053;
constexpr static MsgId MSG_ID_ON_STG_INST_DEL = 10054;
constexpr static MsgId MSG_ID_ON_STG_INST_CHG = 10055;
constexpr static MsgId MSG_ID_ON_STG_INST_TIMER = 10056;
constexpr static MsgId MSG_ID_ON_STG_MANUAL_INTERVENTION = 10060;

constexpr static MsgId MSG_ID_ON_STG_REG = 10071;
constexpr static MsgId MSG_ID_ON_TDGW_REG = 10072;

constexpr static MsgId MSG_ID_EXTEND_CONN_LIFECYCLE = 10073;
constexpr static MsgId MSG_ID_SYNC_ASSETS = 10074;
constexpr static MsgId MSG_ID_SYNC_ASSETS_SNAPSHOT = 10075;
constexpr static MsgId MSG_ID_SYNC_UNCLOSED_ORDER_INFO = 10076;
constexpr static MsgId MSG_ID_SYNC_POS_INFO = 10077;

constexpr static MsgId MSG_ID_ON_ORDER = 10101;
constexpr static MsgId MSG_ID_ON_ORDER_RET = 10102;
constexpr static MsgId MSG_ID_ON_CANCEL_ORDER = 10103;
constexpr static MsgId MSG_ID_ON_CANCEL_ORDER_RET = 10104;

constexpr static MsgId MSG_ID_ON_TEST_ORDER = 10121;
constexpr static MsgId MSG_ID_ON_TEST_CANCEL_ORDER = 10122;

constexpr static MsgId MSG_ID_POS_UPDATE_OF_ACCT_ID = 10201;
constexpr static MsgId MSG_ID_POS_SNAPSHOT_OF_ACCT_ID = 10202;

constexpr static MsgId MSG_ID_POS_UPDATE_OF_STG_ID = 10203;
constexpr static MsgId MSG_ID_POS_SNAPSHOT_OF_STG_ID = 10204;

constexpr static MsgId MSG_ID_POS_UPDATE_OF_STG_INST_ID = 10205;
constexpr static MsgId MSG_ID_POS_SNAPSHOT_OF_STG_INST_ID = 10206;

constexpr static MsgId MSG_ID_ASSETS_UPDATE = 10207;
constexpr static MsgId MSG_ID_ASSETS_SNAPSHOT = 10208;

inline std::string GetMsgName(MsgId msgId) {
  switch (msgId) {
    case MSG_ID_ON_PUSH_TOPIC:
      return "onPushTopic";
    case MSG_ID_ON_MD_TRADES:
      return "onTrades";
    case MSG_ID_ON_MD_TICKERS:
      return "onMDTickers";
    case MSG_ID_ON_MD_CANDLE:
      return "onMDCandle";
    case MSG_ID_ON_MD_BOOKS:
      return "onMDBooks";

    case MSG_ID_ON_STG_START:
      return "onStgStart";
    case MSG_ID_ON_STG_INST_START:
      return "onStgInstStart";

    case MSG_ID_ON_STG_INST_ADD:
      return "onStgInstAdd";
    case MSG_ID_ON_STG_INST_DEL:
      return "onStgInstDel";
    case MSG_ID_ON_STG_INST_CHG:
      return "onStgInstChg";
    case MSG_ID_ON_STG_INST_TIMER:
      return "onStgInstTimer";
    case MSG_ID_ON_STG_MANUAL_INTERVENTION:
      return "onStgManualIntervention";

    case MSG_ID_ON_STG_REG:
      return "onStgReg";
    case MSG_ID_ON_TDGW_REG:
      return "onTDGWReg";

    case MSG_ID_EXTEND_CONN_LIFECYCLE:
      return "extendConnLifecycle";
    case MSG_ID_SYNC_ASSETS:
      return "syncAssets";
    case MSG_ID_SYNC_ASSETS_SNAPSHOT:
      return "syncAssetsSnapshot";
    case MSG_ID_SYNC_UNCLOSED_ORDER_INFO:
      return "syncUnclosedOrderInfo";
    case MSG_ID_SYNC_POS_INFO:
      return "syncPosInfo";

    case MSG_ID_ON_ORDER:
      return "onOrder";
    case MSG_ID_ON_ORDER_RET:
      return "onOrderRet";
    case MSG_ID_ON_CANCEL_ORDER:
      return "onCancelOrder";
    case MSG_ID_ON_CANCEL_ORDER_RET:
      return "onCancelOrderRet";

    case MSG_ID_ON_TEST_ORDER:
      return "onTestOrder";
    case MSG_ID_ON_TEST_CANCEL_ORDER:
      return "onTestCancelOrder";

    case MSG_ID_POS_UPDATE_OF_ACCT_ID:
      return "posUpdateOfAcctId";
    case MSG_ID_POS_SNAPSHOT_OF_ACCT_ID:
      return "posSnapshotOfAcctId";

    case MSG_ID_POS_UPDATE_OF_STG_ID:
      return "posUpdateOfStgId";
    case MSG_ID_POS_SNAPSHOT_OF_STG_ID:
      return "posSnapshotOfStgId";

    case MSG_ID_POS_UPDATE_OF_STG_INST_ID:
      return "posUpdateOfStgInstId";
    case MSG_ID_POS_SNAPSHOT_OF_STG_INST_ID:
      return "posSnapshotOfStgInstId";

    case MSG_ID_ASSETS_UPDATE:
      return "assetsUpdate";
    case MSG_ID_ASSETS_SNAPSHOT:
      return "assetsSnapshot";

    default:
      return "unKnownMsg";
  }
}

}  // namespace bq
