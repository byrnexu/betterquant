/*!
 * \file DBConnpool.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "db/DBEngDef.hpp"
#include "util/Pch.hpp"
#include "util/StdExt.hpp"

namespace bq::db {

struct Conn {
  Conn(const Conn&) = delete;
  Conn& operator=(const Conn&) = delete;
  Conn(const Conn&&) = delete;
  Conn& operator=(const Conn&&) = delete;

  Conn(int no, bool idle, const std::shared_ptr<sql::Connection>& sqlConn)
      : no_(no), idle_(idle), sqlConn_(sqlConn) {}

  int no_{0};
  bool idle_{true};
  std::shared_ptr<sql::Connection> sqlConn_{nullptr};
};

class DBConnpool {
 public:
  DBConnpool(const DBConnpool& p) = delete;
  DBConnpool& operator=(const DBConnpool& p) = delete;
  DBConnpool(const DBConnpool&&) = delete;
  DBConnpool& operator=(const DBConnpool&&) = delete;

  DBConnpool(const DBEngParamSPtr& dbEngParam, ConnType connType);

 public:
  int init();

 private:
  sql::ConnectOptionsMap makeConnProperties(
      const DBEngParamSPtr& dbEngParam) const;

 public:
  std::uint32_t getSize() const;
  ConnSPtr getIdleConn() const;
  void giveBackConn(const ConnSPtr& conn);

 private:
  std::vector<ConnSPtr> connGroup_;
  mutable std::ext::spin_mutex mtxConnGroup_;

  const DBEngParamSPtr dbEngParam_{nullptr};
  const ConnType connType_;
};

}  // namespace bq::db
