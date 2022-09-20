/*!
 * \file PingPongSvc.hpp
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

class PingPongSvc;
using PingPongSvcSPtr = std::shared_ptr<PingPongSvc>;

class PingPongSvc {
 public:
  PingPongSvc() = default;
  PingPongSvc(const PingPongSvc&) = delete;
  PingPongSvc& operator=(const PingPongSvc&) = delete;
  PingPongSvc(const PingPongSvc&&) = delete;
  PingPongSvc& operator=(const PingPongSvc&&) = delete;

  bool isPing(const MsgSPtr& msg) const { return isPingImpl(msg); }
  bool isPong(const MsgSPtr& msg) const { return isPongImpl(msg); }

  std::string makePing() const { return makePingImpl(); }
  std::string makePong(const MsgSPtr& msg) const { return makePongImpl(msg); }

 protected:
  virtual bool isPingImpl(const MsgSPtr& msg) const { return false; }
  virtual std::string makePongImpl(const MsgSPtr& ping) const { return ""; }

  virtual std::string makePingImpl() const { return ""; }
  virtual bool isPongImpl(const MsgSPtr& msg) const { return false; }
};

}  // namespace bq::web
