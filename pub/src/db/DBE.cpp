#include "db/DBE.hpp"

namespace bq::db {

DBEngSPtr MakeDBEng(const bq::db::DBEngParamSPtr& dbEngParam,
                    const CBOnExecRet& cbOnExecRet) {
  const auto dbEng = std::make_shared<DBEng>(dbEngParam, cbOnExecRet);
  return dbEng;
}

std::tuple<int, DBEngSPtr> MakeDBEng(const std::string& dbEngParamInStrFmt,
                                     const CBOnExecRet& cbOnExecRet) {
  auto [ret, dbEngParam] = MakeDBEngParam(dbEngParamInStrFmt);
  if (ret != 0) {
    const auto statusMsg = fmt::format("Make db engine failed.");
    return {ret, nullptr};
  }
  return {0, MakeDBEng(dbEngParam, cbOnExecRet)};
}

}  // namespace bq::db
