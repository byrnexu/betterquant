#pragma once

#include "WebDef.hpp"

namespace bq::web {

inline std::tuple<int, WSCliSPtr> MakeWSCli(
    const bq::web::WSParamSPtr& wsParam, const CBOnWSCliMsg& cbOnWSCliMsg,
    const CBOnWSCliOpen& cbOnWSCliOpen, const CBOnWSCliClose& cbOnWSCliClose,
    const CBOnWSCliFail& cbOnWSCliFail, const PingPongSvcSPtr& pingPongSvc);

inline std::tuple<int, WSCliSPtr> MakeWSCli(
    const std::string& wsParamInStrFmt, const CBOnWSCliMsg& cbOnWSCliMsg,
    const CBOnWSCliOpen& cbOnWSCliOpen, const CBOnWSCliClose& cbOnWSCliClose,
    const CBOnWSCliFail& cbOnWSCliFail, const PingPongSvcSPtr& pingPongSvc);

}  // namespace bq::web
