/*!
 * \file AcctInfoIF.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "def/BQDefIF.hpp"
#include "util/PchBase.hpp"

namespace bq {

struct AcctInfo;
using AcctInfoSPtr = std::shared_ptr<AcctInfo>;

struct AcctInfo {
  AcctId acctId_;
  MarketCode marketCode_;
  SymbolType symbolType_;
};

}  // namespace bq
