/*!
 * \file MDSvcOfBinance.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "MDSvcOfBinance.hpp"

#include "Config.hpp"
#include "ReqParserOfBinance.hpp"
#include "RspParserOfBinance.hpp"
#include "SubAndUnSubSvcOfBinance.hpp"
#include "SymbolTableMaintOfBinance.hpp"
#include "WSCliOfExchBinance.hpp"

namespace bq::md::svc::binance {

int MDSvcOfBinance::beforeInit() {
  const auto reqParser = std::make_shared<ReqParserOfBinance>(this);
  setReqParser(reqParser);

  const auto rspParser = std::make_shared<RspParserOfBinance>(this);
  setRspParser(rspParser);

  const auto symbolTableMaint =
      std::make_shared<SymbolTableMaintOfBinance>(this);
  setSymbolTableMaint(symbolTableMaint);

  const auto wsCliOfExch = std::make_shared<WSCliOfExchBinance>(this);
  setWSCliOfExch(wsCliOfExch);

  const auto subAndUnSubSvc = std::make_shared<SubAndUnSubSvcOfBinance>(this);
  setSubAndUnSubSvc(subAndUnSubSvc);

  return 0;
}

}  // namespace bq::md::svc::binance
