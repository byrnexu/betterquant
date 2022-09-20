/*!
 * \file DBE.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "db/DBEng.hpp"

namespace bq::db {

DBEngSPtr MakeDBEng(const bq::db::DBEngParamSPtr& dbEngParam,
                    const CBOnExecRet& cbOnExecRet);

std::tuple<int, DBEngSPtr> MakeDBEng(const std::string& dbEngParamInStrFmt,
                                     const CBOnExecRet& cbOnExecRet);

}  // namespace bq::db
