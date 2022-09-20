/*!
 * \file WSCli.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "WSCli.hpp"

#include "ConnMetadata.hpp"
#include "PingPongSvc.hpp"
#include "WebConst.hpp"
#include "WebDef.hpp"
#include "WebParam.hpp"
#include "util/Logger.hpp"
#include "util/Random.hpp"
#include "util/Scheduler.hpp"
#include "util/StdExt.hpp"

namespace bq::web {

WSCli::WSCli(const WSParamSPtr& wsParam, const CBOnWSCliMsg& cbOnWSCliMsg,
             const CBOnWSCliOpen& cbOnWSCliOpen,
             const CBOnWSCliClose& cbOnWSCliClose,
             const CBOnWSCliFail& cbOnWSCliFail,
             const PingPongSvcSPtr& pingPongSvc)
    : wsParam_(wsParam),
      cbOnWSCliMsg_(cbOnWSCliMsg),
      cbOnWSCliOpen_(cbOnWSCliOpen),
      cbOnWSCliClose_(cbOnWSCliClose),
      cbOnWSCliFail_(cbOnWSCliFail),
      nextId_(0),
      pingPongSvc_(pingPongSvc) {}

int WSCli::start() {
  LOG_D("Begin to start ws cli.");

  wsEndpoint_.clear_access_channels(websocketpp::log::alevel::all);
  wsEndpoint_.clear_error_channels(websocketpp::log::elevel::all);

  wsEndpoint_.init_asio();
  wsEndpoint_.start_perpetual();

  wsEndpoint_.set_tls_init_handler(
      bind(&WSCli::onTlsInit, this, websocketpp::lib::placeholders::_1));

  wsEndpoint_.set_open_handshake_timeout(wsParam_->timeoutOfOpenHandshake_);
  wsEndpoint_.set_close_handshake_timeout(wsParam_->timeoutOfCloseHandshake_);

  thread_.reset(new websocketpp::lib::thread(&WSEndpoint::run, &wsEndpoint_));

  schedulerPingPong_ = std::make_shared<Scheduler>(
      "WS_CLI", [this]() { sendPingAndCheckConn(); },
      wsParam_->intervalOfSendPingAndCheckConn_);
  auto ret = schedulerPingPong_->start();
  if (ret != 0) {
    LOG_E(("Start ws cli failed."));
    return -1;
  }

  return 0;
}

WSCtxSPtr WSCli::onTlsInit(websocketpp::connection_hdl) {
  WSCtxSPtr ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(
      boost::asio::ssl::context::sslv23);

  try {
    ctx->set_verify_mode(boost::asio::ssl::verify_none);
    ctx->set_options(boost::asio::ssl::context::default_workarounds |
                     boost::asio::ssl::context::no_sslv2 |
                     boost::asio::ssl::context::no_sslv3 |
                     boost::asio::ssl::context::single_dh_use);
  } catch (std::exception& e) {
    LOG_E("On tls init failed. [{}]", e.what());
    return ctx;
  }

  LOG_D("WSCli on tls init success. ");
  return ctx;
}

void WSCli::onOpen(ConnMetadataSPtr connMetadata,
                   websocketpp::connection_hdl hdl) {
  connMetadata->setStatus("Open");
  WSEndpoint::connection_ptr conn = wsEndpoint_.get_con_from_hdl(hdl);
  connMetadata->setServer(conn->get_response_header("Server"));
  LOG_I("WSCli open. {}", connMetadata->toStr());
  if (cbOnWSCliOpen_) {
    cbOnWSCliOpen_(this, connMetadata);
  }
}

void WSCli::onClose(ConnMetadataSPtr connMetadata,
                    websocketpp::connection_hdl hdl) {
  connMetadata->setStatus("Closed");
  WSEndpoint::connection_ptr conn = wsEndpoint_.get_con_from_hdl(hdl);
  connMetadata->setServer(conn->get_response_header("Server"));
  connMetadata->setErrorReason(conn->get_ec().message());
  LOG_I("WSCli close. {}", connMetadata->toStr());
  if (cbOnWSCliClose_) {
    cbOnWSCliClose_(this, connMetadata);
  }
  reconnect(connMetadata, hdl);
}

void WSCli::onFail(ConnMetadataSPtr connMetadata,
                   websocketpp::connection_hdl hdl) {
  connMetadata->setStatus("Failed");
  WSEndpoint::connection_ptr conn = wsEndpoint_.get_con_from_hdl(hdl);
  connMetadata->setServer(conn->get_response_header("Server"));
  connMetadata->setErrorReason(conn->get_ec().message());

  LOG_W("WSCli fail. {}", connMetadata->toStr());
  if (cbOnWSCliFail_) {
    cbOnWSCliFail_(this, connMetadata);
  }
  reconnect(connMetadata, hdl);
}

void WSCli::reconnect(ConnMetadataSPtr connMetadata,
                      websocketpp::connection_hdl hdl) {
  if (stopped_) {
    return;
  }
  const auto interval = GET_RAND_INT() % wsParam_->intervalOfReconnect_;
  LOG_I("Try to reconnect after {} milliseconds. {}", interval,
        connMetadata->toStr());
  std::this_thread::sleep_for(std::chrono::milliseconds(interval));
  connect(connMetadata->getUri(), connMetadata->getNo(), IsReconnect::True);
}

void WSCli::onMsg(ConnMetadataSPtr connMetadata,
                  websocketpp::connection_hdl hdl, MsgSPtr msg) {
  connMetadata->updateActiveTime();
  const auto isPingPong = handlePingPong(connMetadata, hdl, msg);
  if (isPingPong == IsPingPong::True) {
    return;
  }

  if (cbOnWSCliMsg_) {
    cbOnWSCliMsg_(this, connMetadata, msg);
  }
}

IsPingPong WSCli::handlePingPong(ConnMetadataSPtr connMetadata,
                                 websocketpp::connection_hdl hdl, MsgSPtr msg) {
  if (!pingPongSvc_) return IsPingPong::False;

  if (pingPongSvc_->isPing(msg)) {
    handlePing(connMetadata, hdl, msg);
    return IsPingPong::True;
  } else if (pingPongSvc_->isPong(msg)) {
    handlePong(connMetadata, hdl, msg);
    return IsPingPong::True;
  } else {
    return IsPingPong::False;
  }
}

void WSCli::handlePing(ConnMetadataSPtr connMetadata,
                       websocketpp::connection_hdl hdl, MsgSPtr msg) {
  LOG_D("Recv ping. {}", connMetadata->toStr());

  const auto pong = pingPongSvc_->makePong(msg);
  websocketpp::lib::error_code ec;
  wsEndpoint_.send(hdl, pong, websocketpp::frame::opcode::text, ec);
  if (ec) {
    LOG_W("Send pong failed. {} - {} {}", ec.value(), ec.message(),
          connMetadata->toStr());
    return;
  }
  LOG_D("Send pong. {}", connMetadata->toStr());
}

void WSCli::handlePong(ConnMetadataSPtr connMetadata,
                       websocketpp::connection_hdl hdl, MsgSPtr msg) {
  LOG_D("Recv pong. {}", connMetadata->toStr());
}

void WSCli::sendPingAndCheckConn() {
  sendPing();
  checkConn();
}

void WSCli::sendPing() {
  if (wsParam_->sendPing_ && pingPongSvc_ != nullptr) {
    const auto ping = pingPongSvc_->makePing();
    const auto openConnGroup = getOpenConnGroup();
    for (const auto& rec : openConnGroup) {
      websocketpp::lib::error_code ec;
      wsEndpoint_.send(rec.second->getHdl(), ping,
                       websocketpp::frame::opcode::text, ec);
      if (ec) {
        LOG_W("Send ping failed. {} - {} {}", ec.value(), ec.message(),
              rec.second->toStr());
        return;
      }
      LOG_D("Send ping. {}", rec.second->toStr());
    }
  }
}

void WSCli::checkConn() {
  const auto openConnGroup = getOpenConnGroup();
  for (const auto& rec : openConnGroup) {
    const auto& connMetadata = rec.second;
    if (connMetadata->getTimeDurOfNotRecvMsg() > wsParam_->expireTimeOfConn_) {
      LOG_W("Begin to close conn because of conn expired. {} > {} {}",
            connMetadata->getTimeDurOfNotRecvMsg(), wsParam_->expireTimeOfConn_,
            connMetadata->toStr());
      closeConn(connMetadata);
    }
  }
}

void WSCli::stop() {
  stopped_ = true;
  LOG_I("Begin to stop ws cli.");
  schedulerPingPong_->stop();
  wsEndpoint_.stop_perpetual();
  auto openConnGroup = getOpenConnGroup();
  closeOpenConnGroup(openConnGroup);
  if (thread_->joinable()) {
    thread_->join();
  }
  LOG_I("End of stop ws cli.");
}

ConnGroup WSCli::getOpenConnGroup() {
  ConnGroup openConnGroup;
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxConnGroup_);
    for (ConnGroup::const_iterator iter = std::begin(connGroup_);
         iter != std::end(connGroup_); ++iter) {
      if (iter->second->getStatus() == "Open") {
        openConnGroup.emplace(*iter);
      }
    }
  }
  return openConnGroup;
}

void WSCli::closeOpenConnGroup(ConnGroup openConnGroup) {
  for (ConnGroup::const_iterator iter = std::begin(openConnGroup);
       iter != std::end(openConnGroup); ++iter) {
    closeConn(iter->second);
  }
}

void WSCli::closeConn(const ConnMetadataSPtr& connMetadata) {
  LOG_I("Begin to close ws conn. {}", connMetadata->toStr());
  websocketpp::lib::error_code ec;
  wsEndpoint_.close(connMetadata->getHdl(),
                    websocketpp::close::status::going_away, "", ec);
  if (ec) {
    LOG_W("Close ws conn failed. {} - {} {}", ec.value(), ec.message(),
          connMetadata->toStr());
  }
}

std::tuple<int, int> WSCli::connect(std::string const& uri, int no,
                                    IsReconnect isReconnect) {
  if (isReconnect == IsReconnect::False) {
    no = nextId_++;
  }

  websocketpp::lib::error_code ec;
  WSEndpoint::connection_ptr conn = wsEndpoint_.get_connection(uri, ec);
  if (ec) {
    LOG_W("Connect to {} failed. {} - {}", uri, ec.value(), ec.message());
    return {-1, -1};
  }

  ConnMetadataSPtr connMetadata(new ConnMetadata(no, conn->get_handle(), uri));
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxConnGroup_);
    connGroup_[no] = connMetadata;
    LOG_I("Conn group size {}. {}", connGroup_.size(), connMetadata->toStr());
  }

  conn->set_open_handler(websocketpp::lib::bind(
      &WSCli::onOpen, this, connMetadata, websocketpp::lib::placeholders::_1));

  conn->set_close_handler(websocketpp::lib::bind(
      &WSCli::onClose, this, connMetadata, websocketpp::lib::placeholders::_1));

  conn->set_fail_handler(websocketpp::lib::bind(
      &WSCli::onFail, this, connMetadata, websocketpp::lib::placeholders::_1));

  conn->set_message_handler(websocketpp::lib::bind(
      &WSCli::onMsg, this, connMetadata, websocketpp::lib::placeholders::_1,
      websocketpp::lib::placeholders::_2));

  wsEndpoint_.connect(conn);

  return {0, no};
}

int WSCli::send(int no, std::string msg) {
  websocketpp::lib::error_code ec;

  ConnGroup::iterator iter;
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxConnGroup_);
    iter = connGroup_.find(no);
    if (iter == connGroup_.end()) {
      LOG_E("Send msg failed because no connection found with no {}. ", no);
      return -1;
    }
  }

  wsEndpoint_.send(iter->second->getHdl(), msg,
                   websocketpp::frame::opcode::text, ec);
  if (ec) {
    LOG_E("Send msg failed. {} - {} {}", ec.value(), ec.message(),
          iter->second->toStr());
    return -1;
  }

  return 0;
}

int WSCli::send(std::string msg) {
  websocketpp::lib::error_code ec;

  ConnGroup::iterator iter;
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxConnGroup_);
    if (connGroup_.empty()) {
      LOG_E("Send msg failed because no connection.");
      return -1;
    }
    iter = std::begin(connGroup_);
  }

  wsEndpoint_.send(iter->second->getHdl(), msg,
                   websocketpp::frame::opcode::text, ec);
  if (ec) {
    LOG_E("Send msg failed. {} - {} {}", ec.value(), ec.message(),
          iter->second->toStr());
    return -1;
  }

  return 0;
}

ConnMetadataSPtr WSCli::getMetadata(int no) const {
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxConnGroup_);
    ConnGroup::const_iterator iter = connGroup_.find(no);
    if (iter != connGroup_.end()) {
      return iter->second;
    } else {
      return ConnMetadataSPtr();
    }
  }
}

ConnMetadataSPtr WSCli::getMetadata() const {
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxConnGroup_);
    if (connGroup_.empty()) {
      return ConnMetadataSPtr();
    }
    return connGroup_.begin()->second;
  }
}

}  // namespace bq::web
