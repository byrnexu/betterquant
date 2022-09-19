#pragma once

#include "util/Pch.hpp"

#if defined(SIGQUIT)
#include <sys/types.h>
#include <sys/wait.h>
#endif

namespace bq {

class SignalHandler;
using SignalHandlerSPtr = std::shared_ptr<SignalHandler>;

class SignalHandler {
  using CBSignalHandler =
      std::function<void(const boost::system::error_code&, int)>;

 public:
  SignalHandler(const std::string& moduleName,
                const CBSignalHandler& exitHandler) noexcept;

  boost::asio::io_context& io() noexcept;

  void run();

  void setDefaultSignalHandler(const CBSignalHandler& value);

 private:
  void registSignal();

  void handleSignal(const boost::system::error_code& e, int signalNumber);

 private:
  boost::asio::io_context io_;
  boost::asio::signal_set signalGroup_;

  const std::string moduleName_;
  CBSignalHandler exitHandler_{nullptr};
  CBSignalHandler defaultSignalHandler_{nullptr};
};

}  // namespace bq
