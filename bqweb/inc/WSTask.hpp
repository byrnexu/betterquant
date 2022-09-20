/*!
 * \file WSTask.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "WebDef.hpp"

namespace bq::web {

struct TaskFromSrv {
 public:
  TaskFromSrv(const TaskFromSrv&) = delete;
  TaskFromSrv& operator=(const TaskFromSrv&) = delete;
  TaskFromSrv(TaskFromSrv&&) = delete;
  TaskFromSrv& operator=(TaskFromSrv&&) = delete;

  TaskFromSrv() = default;
  TaskFromSrv(bq::web::WSCli* wsCli,
              const bq::web::ConnMetadataSPtr& connMetadata,
              const bq::web::MsgSPtr& msg);

  const std::int64_t localTs_;
  WSCli* wsCli_;
  ConnMetadataSPtr connMetadata_;
  MsgSPtr msg_;
};

}  // namespace bq::web
