/*!
 * \file MDSvcConst.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "util/Pch.hpp"

namespace bq::md::svc {

const static int UNDEFINED_FIELD_VALUE = -1;

const static std::string UNDEFINED_FIELD_MIN_DATETIME =
    "2000-01-01 00:00:00.000000";
const static std::string UNDEFINED_FIELD_MAX_DATETIME =
    "2030-01-01 00:00:00.000000";

const static std::string ET_LTAG = R"("exchTs":)";
const static std::string ET_RTAG = R"(,"localTs")";

}  // namespace bq::md::svc
