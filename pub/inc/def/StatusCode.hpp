/*!
 * \file StatusCode.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

// internal statusCode
const static int SCODE_SUCCESS = 0;

const static int SCODE_BQPUB_TOPIC_ALREADY_SUB = -1101;
const static int SCODE_BQPUB_TOPIC_NOT_SUB = -1102;
const static int SCODE_BQPUB_INVALID_TOPIC = -1103;
const static int SCODE_BQPUB_CALC_PRICE_FAILED = -1104;

const static int SCODE_BQPUB_INVALID_QRY_COND = -1111;
const static int SCODE_BQPUB_PNL_NOT_EXISTS = -1112;
const static int SCODE_BQPUB_POS_INFO_GROUP_NOT_EXISTS = -1113;

const static int SCODE_TD_SVC_EXCEED_FLOW_CTRL = -3501;

const static int SCODE_TD_SRV_TDGW_NOT_EXISTS = -4001;

const static int SCODE_HIS_MD_INVALID_TS = -4501;
const static int SCODE_HIS_MD_INVALID_NUM = -4502;
const static int SCODE_HIS_MD_RECORDS_LESS_THAN_NUM_OF_QURIES = -4503;
const static int SCODE_HIS_MD_NUM_OF_RECORDS_GREATER_THAN_LIMIT = -4504;

const static int SCODE_DB_CAN_NOT_FIND_SYM_CODE = -5001;
const static int SCODE_DB_CAN_NOT_FIND_EXCH_SYM_CODE = -5002;
const static int SCODE_DB_CAN_NOT_FIND_STG_INST = -5003;
const static int SCODE_DB_CAN_NOT_FIND_ACCT_INFO = -5004;

const static int SCODE_STG_MUST_HAVE_STG_INST_1 = -6002;
const static int SCODE_STG_INST_ID_MUST_START_FROM_1 = -6003;

const static int SCODE_STG_ENG_INVALID_CONFIG_FILENAME = -6041;
const static int SCODE_STG_INST_TASK_HANDLER_NOT_INSTALL = -6051;

const static int SCODE_STG_SEND_HTTP_REQ_TO_QUERY_HIS_MD_FAILED = -6061;

const static int SCODE_ORD_MGR_ADD_ORDER_INFO_FAILED = -7001;
const static int SCODE_ORD_MGR_REMOVE_ORDER_INFO_FAILED = -7002;
const static int SCODE_ORD_MGR_CAN_NOT_FIND_ORDER = -7005;

// web srv statusCode [-8001, -9000)
const static int SCODE_WEB_SRV_INVALID_BODY_IN_REQ = -8001;

// external mapping statusCode [-9001, -10000)
const static int SCODE_TD_SRV_RISK_EXCEED_FLOW_CTRL = -9001;

// external statusCode
const static int SCODE_TD_SVC_PARSE_HTTP_RSP_OF_ORDER_FAILED = -11001;
const static int SCODE_TD_SVC_PARSE_HTTP_RSP_OF_CANCEL_ORDER_FAILED = -11002;
const static int SCODE_EXCH_ORDER_NOT_EXISTS = -11003;

const static int SCODE_MD_SVC_SNAPSHOT_NOT_EXISTS = -13001;
const static int SCODE_MD_SVC_FINAL_UPDATE_ID_TOO_SMALL = -13002;
const static int SCODE_MD_SVC_FIRST_UPDATE_ID_TOO_LARGE = -13003;
const static int SCODE_MD_SVC_UPDATE_DATA_DISCONTINUOUS = -13004;

inline std::string GetStatusMsg(int statusCode) {
  if (statusCode == SCODE_SUCCESS) {
    return "Success";
  } else if (statusCode == SCODE_BQPUB_TOPIC_ALREADY_SUB) {
    return "Topic already sub";
  } else if (statusCode == SCODE_BQPUB_TOPIC_NOT_SUB) {
    return "Topic not sub";
  } else if (statusCode == SCODE_BQPUB_INVALID_TOPIC) {
    return "Invalid topic";
  } else if (statusCode == SCODE_BQPUB_CALC_PRICE_FAILED) {
    return "Calc price failed";
  } else if (statusCode == SCODE_BQPUB_INVALID_QRY_COND) {
    return "Invalid query condition";
  } else if (statusCode == SCODE_BQPUB_PNL_NOT_EXISTS) {
    return "Pnl not exists";
  } else if (statusCode == SCODE_BQPUB_POS_INFO_GROUP_NOT_EXISTS) {
    return "PosInfoGroup not exists";
  } else if (statusCode == SCODE_TD_SVC_EXCEED_FLOW_CTRL) {
    return "TDSvc exceed flow control";
  } else if (statusCode == SCODE_TD_SRV_TDGW_NOT_EXISTS) {
    return "TDGW not exists";
  } else if (statusCode == SCODE_HIS_MD_INVALID_TS) {
    return "Invalid ts in query condition";
  } else if (statusCode == SCODE_HIS_MD_INVALID_NUM) {
    return "Invalid num in query condition";
  } else if (statusCode == SCODE_HIS_MD_RECORDS_LESS_THAN_NUM_OF_QURIES) {
    return "The number of records is less than the number of queries";
  } else if (statusCode == SCODE_HIS_MD_NUM_OF_RECORDS_GREATER_THAN_LIMIT) {
    return "The number of returned records is greater than the limit";
  } else if (statusCode == SCODE_DB_CAN_NOT_FIND_SYM_CODE) {
    return "Can not find symbolcode";
  } else if (statusCode == SCODE_DB_CAN_NOT_FIND_EXCH_SYM_CODE) {
    return "Can not find exchange symbolcode";
  } else if (statusCode == SCODE_DB_CAN_NOT_FIND_STG_INST) {
    return "Can not find stg inst";
  } else if (statusCode == SCODE_DB_CAN_NOT_FIND_ACCT_INFO) {
    return "Can not find account info";
  } else if (statusCode == SCODE_STG_MUST_HAVE_STG_INST_1) {
    return "Stg must have stg inst 1";
  } else if (statusCode == SCODE_STG_INST_ID_MUST_START_FROM_1) {
    return "Stg inst id must start from 1";
  } else if (statusCode == SCODE_STG_ENG_INVALID_CONFIG_FILENAME) {
    return "Invalid config filename";
  } else if (statusCode == SCODE_STG_INST_TASK_HANDLER_NOT_INSTALL) {
    return "StgInstTaskHandler not install";
  } else if (statusCode == SCODE_STG_SEND_HTTP_REQ_TO_QUERY_HIS_MD_FAILED) {
    return "Stg send http request to query his market data failed";
  } else if (statusCode == SCODE_ORD_MGR_ADD_ORDER_INFO_FAILED) {
    return "Add orderinfo failed";
  } else if (statusCode == SCODE_ORD_MGR_REMOVE_ORDER_INFO_FAILED) {
    return "Remove order info failed";
  } else if (statusCode == SCODE_ORD_MGR_CAN_NOT_FIND_ORDER) {
    return "Can not find orderinfo";
  } else if (statusCode == SCODE_TD_SVC_PARSE_HTTP_RSP_OF_ORDER_FAILED) {
    return "Parse http rsp of order failed";
  } else if (statusCode == SCODE_TD_SVC_PARSE_HTTP_RSP_OF_CANCEL_ORDER_FAILED) {
    return "Parse http rsp of cancel order failed";
  } else if (statusCode == SCODE_EXCH_ORDER_NOT_EXISTS) {
    return "Exch order not exists";
  } else if (statusCode == SCODE_MD_SVC_SNAPSHOT_NOT_EXISTS) {
    return "Snapshot not exists";
  } else if (statusCode == SCODE_MD_SVC_FINAL_UPDATE_ID_TOO_SMALL) {
    return "Final update id too small";
  } else if (statusCode == SCODE_MD_SVC_FIRST_UPDATE_ID_TOO_LARGE) {
    return "First update id too large";
  } else if (statusCode == SCODE_MD_SVC_UPDATE_DATA_DISCONTINUOUS) {
    return "Update data discontinuous";
  } else if (statusCode == SCODE_WEB_SRV_INVALID_BODY_IN_REQ) {
    return "Invalid body in request.";
  } else if (statusCode == SCODE_TD_SRV_RISK_EXCEED_FLOW_CTRL) {
    return "TDSrv risk exceed flow control";
  } else {
    return "N/A";
  }
  return "N/A";
}
