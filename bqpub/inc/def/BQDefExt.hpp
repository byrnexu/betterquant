/*!
 * \file BQDefExt.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "util/PchBase.hpp"

namespace bq::web {
struct TaskFromSrv;
using TaskFromSrvSPtr = std::shared_ptr<TaskFromSrv>;
}  // namespace bq::web

namespace bq {

template <typename Task>
struct AsyncTask;

using WSCliAsyncTask = AsyncTask<bq::web::TaskFromSrvSPtr>;
using WSCliAsyncTaskSPtr = std::shared_ptr<WSCliAsyncTask>;

}  // namespace bq
