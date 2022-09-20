/*!
 * \file WebConst.hpp
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

const static std::string DEFAULT_WS_PARAM =
    "svcName=WSCli; timeoutOfOpenHandshake=30000; "
    "timeoutOfCloseHandshake=30000; intervalOfReconnect=5000; "
    "intervalOfSendPingAndCheckConn=3000; sendPing=0; expireTimeOfConn=15000";

}
