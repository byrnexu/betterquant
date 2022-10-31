/*!
 * \file PosSnapshotImpl.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/09
 *
 * \brief
 */

#include "StgEng.hpp"

#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/utility.hpp>
#include <chrono>
#include <cstring>
#include <memory>
#include <thread>

#include "StgEngImpl.hpp"
#include "StgEngPYUtil.hpp"
#include "StgInstTaskHandlerImpl.hpp"
#include "def/AssetInfo.hpp"
#include "def/BQConst.hpp"
#include "def/BQDef.hpp"
#include "def/Const.hpp"
#include "def/DataStruOfMD.hpp"
#include "def/DataStruOfOthers.hpp"
#include "def/DataStruOfStg.hpp"
#include "def/DataStruOfTD.hpp"
#include "def/Def.hpp"
#include "def/OrderInfo.hpp"
#include "def/PosInfo.hpp"
#include "def/StgInstInfo.hpp"
#include "def/SymbolInfo.hpp"
#include "util/Logger.hpp"
#include "util/PosSnapshot.hpp"

namespace bq::stg {

StgEng::StgEng(const std::string& configFilename)
    : stgEngImpl_(std::make_shared<StgEngImpl>(configFilename)) {}

int StgEng::init(PyObject* stgInstTaskHandler) {
  StgInstTaskHandlerBundle stgInstTaskHandlerBundle;
  const auto stgInstTaskHandlerImpl = std::make_shared<StgInstTaskHandlerImpl>(
      stgEngImpl_.get(), stgInstTaskHandlerBundle);
  stgEngImpl_->installStgInstTaskHandler(stgInstTaskHandlerImpl);
  const auto ret = stgEngImpl_->init();
  if (ret != 0) {
    return ret;
  }
  installStgInstTaskHandler(stgInstTaskHandler);
  return 0;
}

void StgEng::installStgInstTaskHandler(PyObject* value) {
  stgInstTaskHandler_ = value;

  stgEngImpl_->getStgInstTaskHandler()
      ->getStgInstTaskHandlerBundle()
      .onOrderRet_ = [this](const StgInstInfoSPtr& stgInstInfo,
                            const OrderInfoSPtr& orderInfo) {
    {
      std::lock_guard<std::mutex> guard(mtxPY_);
      try {
        boost::python::call_method<void>(stgInstTaskHandler_, "on_order_ret",
                                         stgInstInfo, orderInfo);
      } catch (const boost::python::error_already_set& e) {
        if (PyErr_Occurred()) {
          const auto msg = handlePYErr();
          LOG_E("Python interpreter error: \n {}", msg);
        }
        boost::python::handle_exception();
        PyErr_Clear();
      }
    }
  };

  stgEngImpl_->getStgInstTaskHandler()
      ->getStgInstTaskHandlerBundle()
      .onCancelOrderRet_ = [this](const StgInstInfoSPtr& stgInstInfo,
                                  const OrderInfoSPtr& orderInfo) {
    {
      std::lock_guard<std::mutex> guard(mtxPY_);
      try {
        boost::python::call_method<void>(
            stgInstTaskHandler_, "on_cancel_order_ret", stgInstInfo, orderInfo);
      } catch (const boost::python::error_already_set& e) {
        if (PyErr_Occurred()) {
          const auto msg = handlePYErr();
          LOG_E("Python interpreter error: \n {}", msg);
        }
        boost::python::handle_exception();
        PyErr_Clear();
      }
    }
  };

  stgEngImpl_->getStgInstTaskHandler()
      ->getStgInstTaskHandlerBundle()
      .onTrades_ = [this](const StgInstInfoSPtr& stgInstInfo,
                          const TradesSPtr& trades) {
    const auto marketData = trades->toJson();
    {
      std::lock_guard<std::mutex> guard(mtxPY_);
      try {
        boost::python::call_method<void>(stgInstTaskHandler_, "on_trades",
                                         stgInstInfo, marketData);
      } catch (const boost::python::error_already_set& e) {
        if (PyErr_Occurred()) {
          const auto msg = handlePYErr();
          LOG_E("Python interpreter error: \n {}", msg);
        }
        boost::python::handle_exception();
        PyErr_Clear();
      }
    }
  };

  stgEngImpl_->getStgInstTaskHandler()->getStgInstTaskHandlerBundle().onBooks_ =
      [this](const StgInstInfoSPtr& stgInstInfo, const BooksSPtr& books) {
        const auto marketData = books->toJson();
        {
          std::lock_guard<std::mutex> guard(mtxPY_);
          try {
            boost::python::call_method<void>(stgInstTaskHandler_, "on_books",
                                             stgInstInfo, marketData);
          } catch (const boost::python::error_already_set& e) {
            if (PyErr_Occurred()) {
              const auto msg = handlePYErr();
              LOG_E("Python interpreter error: \n {}", msg);
            }
            boost::python::handle_exception();
            PyErr_Clear();
          }
        }
      };

  stgEngImpl_->getStgInstTaskHandler()
      ->getStgInstTaskHandlerBundle()
      .onCandle_ = [this](const StgInstInfoSPtr& stgInstInfo,
                          const CandleSPtr& candle) {
    const auto marketData = candle->toJson();
    {
      std::lock_guard<std::mutex> guard(mtxPY_);
      try {
        boost::python::call_method<void>(stgInstTaskHandler_, "on_candle",
                                         stgInstInfo, marketData);
      } catch (const boost::python::error_already_set& e) {
        if (PyErr_Occurred()) {
          const auto msg = handlePYErr();
          LOG_E("Python interpreter error: \n {}", msg);
        }
        boost::python::handle_exception();
        PyErr_Clear();
      }
    }
  };

  stgEngImpl_->getStgInstTaskHandler()
      ->getStgInstTaskHandlerBundle()
      .onTickers_ = [this](const StgInstInfoSPtr& stgInstInfo,
                           const TickersSPtr& tickers) {
    const auto marketData = tickers->toJson();
    {
      std::lock_guard<std::mutex> guard(mtxPY_);
      try {
        boost::python::call_method<void>(stgInstTaskHandler_, "on_tickers",
                                         stgInstInfo, marketData);
      } catch (const boost::python::error_already_set& e) {
        if (PyErr_Occurred()) {
          const auto msg = handlePYErr();
          LOG_E("Python interpreter error: \n {}", msg);
        }
        boost::python::handle_exception();
        PyErr_Clear();
      }
    }
  };

  stgEngImpl_->getStgInstTaskHandler()
      ->getStgInstTaskHandlerBundle()
      .onStgStart_ = [this]() {
    {
      std::lock_guard<std::mutex> guard(mtxPY_);
      try {
        boost::python::call_method<void>(stgInstTaskHandler_, "on_stg_start");
      } catch (const boost::python::error_already_set& e) {
        if (PyErr_Occurred()) {
          const auto msg = handlePYErr();
          LOG_E("Python interpreter error: \n {}", msg);
        }
        boost::python::handle_exception();
        PyErr_Clear();
      }
    }
  };

  stgEngImpl_->getStgInstTaskHandler()
      ->getStgInstTaskHandlerBundle()
      .onStgInstStart_ = [this](const auto& stgInstInfo) {
    for (int i = 0; i < 1; ++i) {
      {
        std::lock_guard<std::mutex> guard(mtxPY_);
        try {
          boost::python::call_method<void>(stgInstTaskHandler_,
                                           "on_stg_inst_start", stgInstInfo);
        } catch (const boost::python::error_already_set& e) {
          if (PyErr_Occurred()) {
            const auto msg = handlePYErr();
            LOG_E("Python interpreter error: \n {}", msg);
          }
          boost::python::handle_exception();
          PyErr_Clear();
        }
      }

      /*
      const auto orderInfo = MakeOrderInfo();

      {
        std::lock_guard<std::mutex> guard(mtxPY_);
        try {
          boost::python::call_method<void>(stgInstTaskHandler_, "on_order_ret",
                                           stgInstInfo, orderInfo);
        } catch (const boost::python::error_already_set& e) {
          if (PyErr_Occurred()) {
            const auto msg = handlePYErr();
            LOG_E("Python interpreter error: \n {}", msg);
          }
          boost::python::handle_exception();
          PyErr_Clear();
        }
      }

      {
        std::lock_guard<std::mutex> guard(mtxPY_);
        try {
          boost::python::call_method<void>(stgInstTaskHandler_,
                                           "on_cancel_order_ret", stgInstInfo,
                                           orderInfo);
        } catch (const boost::python::error_already_set& e) {
          if (PyErr_Occurred()) {
            const auto msg = handlePYErr();
            LOG_E("Python interpreter error: \n {}", msg);
          }
          boost::python::handle_exception();
          PyErr_Clear();
        }
      }

      const auto trades = std::make_shared<Trades>();
      {
        std::lock_guard<std::mutex> guard(mtxPY_);
        try {
          boost::python::call_method<void>(stgInstTaskHandler_, "on_trades",
                                           stgInstInfo, trades->toJson());
        } catch (const boost::python::error_already_set& e) {
          if (PyErr_Occurred()) {
            const auto msg = handlePYErr();
            LOG_E("Python interpreter error: \n {}", msg);
          }
          boost::python::handle_exception();
          PyErr_Clear();
        }
      }

      const auto books = std::make_shared<Books>();
      {
        std::lock_guard<std::mutex> guard(mtxPY_);
        try {
          boost::python::call_method<void>(stgInstTaskHandler_, "on_books",
                                           stgInstInfo, books->toJson());
        } catch (const boost::python::error_already_set& e) {
          if (PyErr_Occurred()) {
            const auto msg = handlePYErr();
            LOG_E("Python interpreter error: \n {}", msg);
          }
          boost::python::handle_exception();
          PyErr_Clear();
        }
      }

      const auto tickers = std::make_shared<Tickers>();
      {
        std::lock_guard<std::mutex> guard(mtxPY_);
        try {
          boost::python::call_method<void>(stgInstTaskHandler_, "on_tickers",
                                           stgInstInfo, tickers->toJson());
        } catch (const boost::python::error_already_set& e) {
          if (PyErr_Occurred()) {
            const auto msg = handlePYErr();
            LOG_E("Python interpreter error: \n {}", msg);
          }
          boost::python::handle_exception();
          PyErr_Clear();
        }
      }

      const auto candle = std::make_shared<Candle>();
      {
        std::lock_guard<std::mutex> guard(mtxPY_);
        try {
          boost::python::call_method<void>(stgInstTaskHandler_, "on_candle",
                                           stgInstInfo, candle->toJson());
        } catch (const boost::python::error_already_set& e) {
          if (PyErr_Occurred()) {
            const auto msg = handlePYErr();
            LOG_E("Python interpreter error: \n {}", msg);
          }
          boost::python::handle_exception();
          PyErr_Clear();
        }
      }

      {
        std::lock_guard<std::mutex> guard(mtxPY_);
        try {
          boost::python::call_method<void>(stgInstTaskHandler_,
                                           "on_stg_inst_add", stgInstInfo);
        } catch (const boost::python::error_already_set& e) {
          if (PyErr_Occurred()) {
            const auto msg = handlePYErr();
            LOG_E("Python interpreter error: \n {}", msg);
          }
          boost::python::handle_exception();
          PyErr_Clear();
        }
      }

      {
        std::lock_guard<std::mutex> guard(mtxPY_);
        try {
          boost::python::call_method<void>(stgInstTaskHandler_,
                                           "on_stg_inst_del", stgInstInfo);
        } catch (const boost::python::error_already_set& e) {
          if (PyErr_Occurred()) {
            const auto msg = handlePYErr();
            LOG_E("Python interpreter error: \n {}", msg);
          }
          boost::python::handle_exception();
          PyErr_Clear();
        }
      }

      {
        std::lock_guard<std::mutex> guard(mtxPY_);
        try {
          boost::python::call_method<void>(stgInstTaskHandler_,
                                           "on_stg_inst_chg", stgInstInfo);
        } catch (const boost::python::error_already_set& e) {
          if (PyErr_Occurred()) {
            const auto msg = handlePYErr();
            LOG_E("Python interpreter error: \n {}", msg);
          }
          boost::python::handle_exception();
          PyErr_Clear();
        }
      }

      auto posSnapshot =
          std::make_shared<PosSnapshot>(std::map<std::string, PosInfoSPtr>(),
                                        std::make_shared<MarketDataCache>());
      {
        std::lock_guard<std::mutex> guard(mtxPY_);
        try {
          boost::python::call_method<void>(stgInstTaskHandler_,
                                           "on_pos_update_of_acct_id",
                                           stgInstInfo, posSnapshot);
        } catch (const boost::python::error_already_set& e) {
          if (PyErr_Occurred()) {
            const auto msg = handlePYErr();
            LOG_E("Python interpreter error: \n {}", msg);
          }
          boost::python::handle_exception();
          PyErr_Clear();
        }
      }

      {
        std::lock_guard<std::mutex> guard(mtxPY_);
        try {
          boost::python::call_method<void>(stgInstTaskHandler_,
                                           "on_pos_snapshot_of_acct_id",
                                           stgInstInfo, posSnapshot);
        } catch (const boost::python::error_already_set& e) {
          if (PyErr_Occurred()) {
            const auto msg = handlePYErr();
            LOG_E("Python interpreter error: \n {}", msg);
          }
          boost::python::handle_exception();
          PyErr_Clear();
        }
      }

      {
        std::lock_guard<std::mutex> guard(mtxPY_);
        try {
          boost::python::call_method<void>(stgInstTaskHandler_,
                                           "on_pos_update_of_stg_id",
                                           stgInstInfo, posSnapshot);
        } catch (const boost::python::error_already_set& e) {
          if (PyErr_Occurred()) {
            const auto msg = handlePYErr();
            LOG_E("Python interpreter error: \n {}", msg);
          }
          boost::python::handle_exception();
          PyErr_Clear();
        }
      }

      {
        std::lock_guard<std::mutex> guard(mtxPY_);
        try {
          boost::python::call_method<void>(stgInstTaskHandler_,
                                           "on_pos_snapshot_of_stg_id",
                                           stgInstInfo, posSnapshot);
        } catch (const boost::python::error_already_set& e) {
          if (PyErr_Occurred()) {
            const auto msg = handlePYErr();
            LOG_E("Python interpreter error: \n {}", msg);
          }
          boost::python::handle_exception();
          PyErr_Clear();
        }
      }

      {
        std::lock_guard<std::mutex> guard(mtxPY_);
        try {
          boost::python::call_method<void>(stgInstTaskHandler_,
                                           "on_pos_update_of_stg_inst_id",
                                           stgInstInfo, posSnapshot);
        } catch (const boost::python::error_already_set& e) {
          if (PyErr_Occurred()) {
            const auto msg = handlePYErr();
            LOG_E("Python interpreter error: \n {}", msg);
          }
          boost::python::handle_exception();
          PyErr_Clear();
        }
      }

      {
        std::lock_guard<std::mutex> guard(mtxPY_);
        try {
          boost::python::call_method<void>(stgInstTaskHandler_,
                                           "on_pos_snapshot_of_stg_inst_id",
                                           stgInstInfo, posSnapshot);
        } catch (const boost::python::error_already_set& e) {
          if (PyErr_Occurred()) {
            const auto msg = handlePYErr();
            LOG_E("Python interpreter error: \n {}", msg);
          }
          boost::python::handle_exception();
          PyErr_Clear();
        }
      }

      const auto assetInfoUpdate = std::make_shared<AssetInfo>();
      auto assetsUpdate = std::make_shared<AssetsUpdate>();
      assetsUpdate->emplace(assetInfoUpdate->getKey(), assetInfoUpdate);
      {
        std::lock_guard<std::mutex> guard(mtxPY_);
        try {
          boost::python::call_method<void>(stgInstTaskHandler_,
                                           "on_assets_update", stgInstInfo,
                                           assetsUpdate);
        } catch (const boost::python::error_already_set& e) {
          if (PyErr_Occurred()) {
            const auto msg = handlePYErr();
            LOG_E("Python interpreter error: \n {}", msg);
          }
          boost::python::handle_exception();
          PyErr_Clear();
        }
      }

      const auto assetInfoSnapshot = std::make_shared<AssetInfo>();
      auto assetsSnapshot = std::make_shared<AssetsSnapshot>();
      assetsSnapshot->emplace(assetInfoSnapshot->getKey(), assetInfoSnapshot);
      {
        std::lock_guard<std::mutex> guard(mtxPY_);
        try {
          boost::python::call_method<void>(stgInstTaskHandler_,
                                           "on_assets_snapshot", stgInstInfo,
                                           assetsSnapshot);
        } catch (const boost::python::error_already_set& e) {
          if (PyErr_Occurred()) {
            const auto msg = handlePYErr();
            LOG_E("Python interpreter error: \n {}", msg);
          }
          boost::python::handle_exception();
          PyErr_Clear();
        }
      }
      */

      // end of test
    }
  };

  stgEngImpl_->getStgInstTaskHandler()
      ->getStgInstTaskHandlerBundle()
      .onStgInstAdd_ = [this](const StgInstInfoSPtr& stgInstInfo) {
    {
      std::lock_guard<std::mutex> guard(mtxPY_);
      try {
        boost::python::call_method<void>(stgInstTaskHandler_, "on_stg_inst_add",
                                         stgInstInfo);
      } catch (const boost::python::error_already_set& e) {
        if (PyErr_Occurred()) {
          const auto msg = handlePYErr();
          LOG_E("Python interpreter error: \n {}", msg);
        }
        boost::python::handle_exception();
        PyErr_Clear();
      }
    }
  };

  stgEngImpl_->getStgInstTaskHandler()
      ->getStgInstTaskHandlerBundle()
      .onStgInstDel_ = [this](const StgInstInfoSPtr& stgInstInfo) {
    {
      std::lock_guard<std::mutex> guard(mtxPY_);
      try {
        boost::python::call_method<void>(stgInstTaskHandler_, "on_stg_inst_del",
                                         stgInstInfo);
      } catch (const boost::python::error_already_set& e) {
        if (PyErr_Occurred()) {
          const auto msg = handlePYErr();
          LOG_E("Python interpreter error: \n {}", msg);
        }
        boost::python::handle_exception();
        PyErr_Clear();
      }
    }
  };

  stgEngImpl_->getStgInstTaskHandler()
      ->getStgInstTaskHandlerBundle()
      .onStgInstChg_ = [this](const StgInstInfoSPtr& stgInstInfo) {
    {
      std::lock_guard<std::mutex> guard(mtxPY_);
      try {
        boost::python::call_method<void>(stgInstTaskHandler_, "on_stg_inst_chg",
                                         stgInstInfo);
      } catch (const boost::python::error_already_set& e) {
        if (PyErr_Occurred()) {
          const auto msg = handlePYErr();
          LOG_E("Python interpreter error: \n {}", msg);
        }
        boost::python::handle_exception();
        PyErr_Clear();
      }
    }
  };

  stgEngImpl_->getStgInstTaskHandler()
      ->getStgInstTaskHandlerBundle()
      .onStgInstTimer_ = [this](const StgInstInfoSPtr& stgInstInfo) {
    {
      std::lock_guard<std::mutex> guard(mtxPY_);
      try {
        boost::python::call_method<void>(stgInstTaskHandler_,
                                         "on_stg_inst_timer", stgInstInfo);
      } catch (const boost::python::error_already_set& e) {
        if (PyErr_Occurred()) {
          const auto msg = handlePYErr();
          LOG_E("Python interpreter error: \n {}", msg);
        }
        boost::python::handle_exception();
        PyErr_Clear();
      }
    }
  };

  stgEngImpl_->getStgInstTaskHandler()
      ->getStgInstTaskHandlerBundle()
      .onPosUpdateOfAcctId_ = [this](const StgInstInfoSPtr& stgInstInfo,
                                     const PosSnapshotSPtr& posSnapshot) {
    {
      std::lock_guard<std::mutex> guard(mtxPY_);
      try {
        boost::python::call_method<void>(stgInstTaskHandler_,
                                         "on_pos_update_of_acct_id",
                                         stgInstInfo, posSnapshot);
      } catch (const boost::python::error_already_set& e) {
        if (PyErr_Occurred()) {
          const auto msg = handlePYErr();
          LOG_E("Python interpreter error: \n {}", msg);
        }
        boost::python::handle_exception();
        PyErr_Clear();
      }
    }
  };

  stgEngImpl_->getStgInstTaskHandler()
      ->getStgInstTaskHandlerBundle()
      .onPosSnapshotOfAcctId_ = [this](const StgInstInfoSPtr& stgInstInfo,
                                       const PosSnapshotSPtr& posSnapshot) {
    {
      std::lock_guard<std::mutex> guard(mtxPY_);
      try {
        boost::python::call_method<void>(stgInstTaskHandler_,
                                         "on_pos_snapshot_of_acct_id",
                                         stgInstInfo, posSnapshot);
      } catch (const boost::python::error_already_set& e) {
        if (PyErr_Occurred()) {
          const auto msg = handlePYErr();
          LOG_E("Python interpreter error: \n {}", msg);
        }
        boost::python::handle_exception();
        PyErr_Clear();
      }
    }
  };

  stgEngImpl_->getStgInstTaskHandler()
      ->getStgInstTaskHandlerBundle()
      .onPosUpdateOfStgId_ = [this](const StgInstInfoSPtr& stgInstInfo,
                                    const PosSnapshotSPtr& posSnapshot) {
    {
      std::lock_guard<std::mutex> guard(mtxPY_);
      try {
        boost::python::call_method<void>(stgInstTaskHandler_,
                                         "on_pos_update_of_stg_id", stgInstInfo,
                                         posSnapshot);
      } catch (const boost::python::error_already_set& e) {
        if (PyErr_Occurred()) {
          const auto msg = handlePYErr();
          LOG_E("Python interpreter error: \n {}", msg);
        }
        boost::python::handle_exception();
        PyErr_Clear();
      }
    }
  };

  stgEngImpl_->getStgInstTaskHandler()
      ->getStgInstTaskHandlerBundle()
      .onPosSnapshotOfStgId_ = [this](const StgInstInfoSPtr& stgInstInfo,
                                      const PosSnapshotSPtr& posSnapshot) {
    {
      std::lock_guard<std::mutex> guard(mtxPY_);
      try {
        boost::python::call_method<void>(stgInstTaskHandler_,
                                         "on_pos_snapshot_of_stg_id",
                                         stgInstInfo, posSnapshot);
      } catch (const boost::python::error_already_set& e) {
        if (PyErr_Occurred()) {
          const auto msg = handlePYErr();
          LOG_E("Python interpreter error: \n {}", msg);
        }
        boost::python::handle_exception();
        PyErr_Clear();
      }
    }
  };

  stgEngImpl_->getStgInstTaskHandler()
      ->getStgInstTaskHandlerBundle()
      .onPosUpdateOfStgInstId_ = [this](const StgInstInfoSPtr& stgInstInfo,
                                        const PosSnapshotSPtr& posSnapshot) {
    {
      std::lock_guard<std::mutex> guard(mtxPY_);
      try {
        boost::python::call_method<void>(stgInstTaskHandler_,
                                         "on_pos_update_of_stg_inst_id",
                                         stgInstInfo, posSnapshot);
      } catch (const boost::python::error_already_set& e) {
        if (PyErr_Occurred()) {
          const auto msg = handlePYErr();
          LOG_E("Python interpreter error: \n {}", msg);
        }
        boost::python::handle_exception();
        PyErr_Clear();
      }
    }
  };

  stgEngImpl_->getStgInstTaskHandler()
      ->getStgInstTaskHandlerBundle()
      .onPosSnapshotOfStgInstId_ = [this](const StgInstInfoSPtr& stgInstInfo,
                                          const PosSnapshotSPtr& posSnapshot) {
    {
      std::lock_guard<std::mutex> guard(mtxPY_);
      try {
        boost::python::call_method<void>(stgInstTaskHandler_,
                                         "on_pos_snapshot_of_stg_inst_id",
                                         stgInstInfo, posSnapshot);
      } catch (const boost::python::error_already_set& e) {
        if (PyErr_Occurred()) {
          const auto msg = handlePYErr();
          LOG_E("Python interpreter error: \n {}", msg);
        }
        boost::python::handle_exception();
        PyErr_Clear();
      }
    }
  };

  stgEngImpl_->getStgInstTaskHandler()
      ->getStgInstTaskHandlerBundle()
      .onAssetsUpdate_ = [this](const StgInstInfoSPtr& stgInstInfo,
                                const AssetsUpdateSPtr& assetsUpdate) {
    {
      std::lock_guard<std::mutex> guard(mtxPY_);
      try {
        boost::python::call_method<void>(
            stgInstTaskHandler_, "on_assets_update", stgInstInfo, assetsUpdate);
      } catch (const boost::python::error_already_set& e) {
        if (PyErr_Occurred()) {
          const auto msg = handlePYErr();
          LOG_E("Python interpreter error: \n {}", msg);
        }
        boost::python::handle_exception();
        PyErr_Clear();
      }
    }
  };

  stgEngImpl_->getStgInstTaskHandler()
      ->getStgInstTaskHandlerBundle()
      .onAssetsSnapshot_ = [this](const StgInstInfoSPtr& stgInstInfo,
                                  const AssetsSnapshotSPtr& assetsSnapshot) {
    {
      std::lock_guard<std::mutex> guard(mtxPY_);
      try {
        boost::python::call_method<void>(stgInstTaskHandler_,
                                         "on_assets_snapshot", stgInstInfo,
                                         assetsSnapshot);
      } catch (const boost::python::error_already_set& e) {
        if (PyErr_Occurred()) {
          const auto msg = handlePYErr();
          LOG_E("Python interpreter error: \n {}", msg);
        }
        boost::python::handle_exception();
        PyErr_Clear();
      }
    }
  };
}

int StgEng::run() { return stgEngImpl_->run(); }

void StgEng::installStgInstTimer(StgInstId stgInstId,
                                 const std::string& timerName,
                                 ExecAtStartup execAtStartUp,
                                 std::uint32_t milliSecInterval,
                                 std::uint64_t maxExecTimes) {
  stgEngImpl_->installStgInstTimer(stgInstId, timerName, execAtStartUp,
                                   milliSecInterval, maxExecTimes);
}

std::tuple<int, OrderId> StgEng::order(const StgInstInfoSPtr& stgInstInfo,
                                       AcctId acctId,
                                       const std::string& symbolCode, Side side,
                                       PosSide posSide, Decimal orderPrice,
                                       Decimal orderSize) {
  return stgEngImpl_->order(stgInstInfo, acctId, symbolCode, side, posSide,
                            orderPrice, orderSize);
}

std::tuple<int, OrderId> StgEng::order(OrderInfoSPtr& orderInfo) {
  return stgEngImpl_->order(orderInfo);
}

int StgEng::cancelOrder(OrderId orderId) {
  return stgEngImpl_->cancelOrder(orderId);
}

std::tuple<int, OrderInfoSPtr> StgEng::getOrderInfo(OrderId orderId) const {
  return stgEngImpl_->getOrderInfo(orderId);
}

int StgEng::sub(StgInstId subscriber, const std::string& topic) {
  return stgEngImpl_->sub(subscriber, topic);
}

int StgEng::unSub(StgInstId subscriber, const std::string& topic) {
  return stgEngImpl_->unSub(subscriber, topic);
}

bool StgEng::saveStgPrivateData(StgInstId stgInstId,
                                const std::string& jsonStr) {
  return stgEngImpl_->saveStgPrivateData(stgInstId, jsonStr);
}

std::string StgEng::loadStgPrivateData(StgInstId stgInstId) {
  return stgEngImpl_->loadStgPrivateData(stgInstId);
}

void StgEng::saveToDB(const PnlSPtr& pnl) {
  if (!pnl) return;
  stgEngImpl_->saveToDB(pnl);
}

}  // namespace bq::stg
