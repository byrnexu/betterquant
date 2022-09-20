/*!
 * \file WebDef.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "util/Pch.hpp"

namespace bq::web {

struct WSParam;
using WSParamSPtr = std::shared_ptr<WSParam>;

class PingPongSvc;
using PingPongSvcSPtr = std::shared_ptr<PingPongSvc>;

class WSCli;
using WSCliSPtr = std::shared_ptr<WSCli>;

struct TaskFromSrv;
using TaskFromSrvSPtr = std::shared_ptr<TaskFromSrv>;
using TaskFromSrvUPtr = std::unique_ptr<TaskFromSrv>;

class ConnMetadata;
using ConnMetadataSPtr = websocketpp::lib::shared_ptr<ConnMetadata>;

using MsgSPtr = websocketpp::config::asio_tls_client::message_type::ptr;

using CBOnWSCliMsg = std::function<void(
    WSCli* wsCli, const ConnMetadataSPtr& connMetadata, const MsgSPtr& msg)>;
using CBOnWSCliOpen =
    std::function<void(WSCli* wsCli, const ConnMetadataSPtr& connMetadata)>;
using CBOnWSCliClose =
    std::function<void(WSCli* wsCli, const ConnMetadataSPtr& connMetadata)>;
using CBOnWSCliFail =
    std::function<void(WSCli* wsCli, const ConnMetadataSPtr& connMetadata)>;

}  // namespace bq::web
