/*!
 * \file bq_v1_ManualIntervention.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/25
 *
 * \brief
 */

#pragma once

#include <drogon/HttpController.h>

#include "def/BQDef.hpp"

using namespace drogon;

namespace bq {
namespace v1 {

class ManualIntervention : public drogon::HttpController<ManualIntervention> {
 public:
  METHOD_LIST_BEGIN

  // http://192.168.19.113/v1/manualIntervention?stgId=10000&stgInstId=1
  ADD_METHOD_TO(ManualIntervention::manualIntervention,
                "/v1/ManualIntervention?stgId={stgId}&stgInstId={stgInstId}",
                Post);
  METHOD_LIST_END

  void manualIntervention(
      const HttpRequestPtr &req,
      std::function<void(const HttpResponsePtr &)> &&callback, StgId stgId,
      StgInstId stgInstId) const;
};

}  // namespace v1
}  // namespace bq
