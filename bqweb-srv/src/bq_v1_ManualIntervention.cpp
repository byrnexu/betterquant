/*!
 * \file bq_v1_ManualIntervention.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/25
 *
 * \brief
 */

#include "bq_v1_ManualIntervention.hpp"

#include "Config.hpp"
#include "HisMD.hpp"
#include "SHMIPCMsgId.hpp"
#include "SHMSrv.hpp"
#include "WebSrv.hpp"
#include "def/CommonIPCData.hpp"
#include "def/Def.hpp"
#include "def/StatusCode.hpp"
#include "util/BQUtil.hpp"
#include "util/Logger.hpp"

using namespace bq::v1;

void ManualIntervention::manualIntervention(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback, StgId stgId,
    StgInstId stgInstId) const {
  Doc doc;
  std::string body = std::string(req->body());
  if (doc.Parse(body.data()).HasParseError()) {
    LOG_W("Parse body failed. {0} [offset {1}] {2}",
          GetParseError_En(doc.GetParseError()), doc.GetErrorOffset(), body);
    auto resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(k200OK);
    resp->setContentTypeCode(CT_APPLICATION_JSON);
    const auto rspBody =
        MakeCommonHttpBody(SCODE_WEB_SRV_INVALID_BODY_IN_REQ, body);
    resp->setBody(rspBody);
    callback(resp);
    return;
  }

  if (!body.empty()) {
    body[0] = ',';
  } else {
    body = "}";
  }
  auto data = fmt::format(R"({{"stgId":{},"stgInstId":{})", stgId, stgInstId);
  data = data + body;

  LOG_I("Recv http request. {}", data)

  WebSrv::get_mutable_instance().getSHMSrvOfStgEng()->pushMsgWithZeroCopy(
      [&](void *shmBuf) {
        auto commonIPCData = static_cast<CommonIPCData *>(shmBuf);
        memcpy(commonIPCData->data_, data.c_str(), data.size());
      },
      stgId, MSG_ID_ON_STG_MANUAL_INTERVENTION,
      sizeof(CommonIPCData) + data.size() + 1);

  LOG_I("Forward http request. {}", data)

  auto resp = HttpResponse::newHttpResponse();
  resp->setStatusCode(k200OK);
  resp->setContentTypeCode(CT_APPLICATION_JSON);
  const auto rspBody = MakeCommonHttpBody(SCODE_SUCCESS, body);
  resp->setBody(rspBody);
  callback(resp);

  LOG_I("Send http response. {}", rspBody)
}
