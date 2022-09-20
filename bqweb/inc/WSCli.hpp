/*!
 * \file WSCli.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "WebDef.hpp"
#include "util/Scheduler.hpp"
#include "util/StdExt.hpp"

namespace bq::web {

class WSCli;
using WSCliSPtr = std::shared_ptr<WSCli>;

using WSEndpoint = websocketpp::client<websocketpp::config::asio_tls>;
using WSCtxSPtr = websocketpp::lib::shared_ptr<boost::asio::ssl::context>;
using ConnGroup = std::map<int, ConnMetadataSPtr>;
enum class IsReconnect { True = 1, False = 2 };
enum class IsPingPong { True = 1, False = 2 };

class WSCli {
 public:
  WSCli(const WSCli&) = delete;
  WSCli& operator=(const WSCli&) = delete;
  WSCli(WSCli&&) = delete;
  WSCli& operator=(WSCli&&) = delete;

  WSCli(const WSParamSPtr& wsParam, const CBOnWSCliMsg& cbOnWSCliMsg,
        const CBOnWSCliOpen& cbOnWSCliOpen,
        const CBOnWSCliClose& cbOnWSCliClose,
        const CBOnWSCliFail& cbOnWSCliFail, const PingPongSvcSPtr& pingPongSvc);

  int start();

 private:
  WSCtxSPtr onTlsInit(websocketpp::connection_hdl);

  void onOpen(ConnMetadataSPtr connMetadata, websocketpp::connection_hdl hdl);
  void onClose(ConnMetadataSPtr connMetadata, websocketpp::connection_hdl hdl);
  void onFail(ConnMetadataSPtr connMetadata, websocketpp::connection_hdl hdl);

  void reconnect(ConnMetadataSPtr connMetadata,
                 websocketpp::connection_hdl hdl);

  void onMsg(ConnMetadataSPtr connMetadata, websocketpp::connection_hdl hdl,
             MsgSPtr msg);

  IsPingPong handlePingPong(ConnMetadataSPtr connMetadata,
                            websocketpp::connection_hdl hdl, MsgSPtr msg);
  void handlePing(ConnMetadataSPtr connMetadata,
                  websocketpp::connection_hdl hdl, MsgSPtr msg);
  void handlePong(ConnMetadataSPtr connMetadata,
                  websocketpp::connection_hdl hdl, MsgSPtr msg);

  void sendPingAndCheckConn();
  void sendPing();
  void checkConn();

 public:
  void stop();

 private:
  ConnGroup getOpenConnGroup();
  void closeOpenConnGroup(ConnGroup openConnGroup);
  void closeConn(const ConnMetadataSPtr& connMetadata);

 public:
  std::tuple<int, int> connect(std::string const& uri, int no = 0,
                               IsReconnect isReconnect = IsReconnect::False);

  int send(int no, std::string msg);
  int send(std::string msg);

  ConnMetadataSPtr getMetadata(int no) const;
  ConnMetadataSPtr getMetadata() const;

 private:
  WSParamSPtr wsParam_{nullptr};
  CBOnWSCliMsg cbOnWSCliMsg_{nullptr};
  CBOnWSCliOpen cbOnWSCliOpen_{nullptr};
  CBOnWSCliClose cbOnWSCliClose_{nullptr};
  CBOnWSCliFail cbOnWSCliFail_{nullptr};

  int nextId_{0};

  WSEndpoint wsEndpoint_;
  websocketpp::lib::shared_ptr<websocketpp::lib::thread> thread_{nullptr};

  ConnGroup connGroup_;
  mutable std::ext::spin_mutex mtxConnGroup_;

  PingPongSvcSPtr pingPongSvc_{nullptr};
  SchedulerSPtr schedulerPingPong_;

  bool stopped_{false};
};

}  // namespace bq::web
