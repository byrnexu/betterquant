/*!
 * \file BQUtil.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "def/BQDef.hpp"
#include "util/Pch.hpp"

namespace bq {

struct SHMIPCTask;
using SHMIPCTaskSPtr = std::shared_ptr<SHMIPCTask>;

struct StgInstInfo;
using StgInstInfoSPtr = std::shared_ptr<StgInstInfo>;

std::tuple<int, std::string> GetAddrFromTopic(const std::string& appName,
                                              const std::string& topic);

AcctId GetAcctIdFromTask(const SHMIPCTaskSPtr& task);

std::string convertTopic(const std::string& topic);

std::string ToPrettyStr(Decimal value);

std::string MakeCommonHttpBody(int statusCode, std::string data = "");

void PrintLogo();

}  // namespace bq
