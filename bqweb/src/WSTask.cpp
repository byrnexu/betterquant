/*!
 * \file WSTask.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "WSTask.hpp"

#include "WebDef.hpp"
#include "util/Datetime.hpp"

namespace bq::web {

TaskFromSrv::TaskFromSrv(bq::web::WSCli* wsCli,
                         const bq::web::ConnMetadataSPtr& connMetadata,
                         const bq::web::MsgSPtr& msg)
    : localTs_(GetTotalUSSince1970()),
      wsCli_(wsCli),
      connMetadata_(connMetadata),
      msg_(msg) {}

}  // namespace bq::web
