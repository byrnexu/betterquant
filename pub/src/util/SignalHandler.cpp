/*!
 * \file SignalHandler.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "util/SignalHandler.hpp"

#include "util/Logger.hpp"
#include "util/Pch.hpp"

#if defined(SIGQUIT)
#include <sys/types.h>
#include <sys/wait.h>
#endif

namespace bq {

SignalHandler::SignalHandler(const std::string& moduleName,
                             const CBSignalHandler& exitHandler) noexcept
    : signalGroup_(io_), moduleName_(moduleName), exitHandler_(exitHandler) {
  registSignal();
}

boost::asio::io_context& SignalHandler::io() noexcept { return io_; }

void SignalHandler::run() {
  LOG_D("[{}] Signal svc start.", moduleName_);
  boost::system::error_code ec;
  const auto num = io_.run(ec);
  if (ec) {
    LOG_E("[{}] IO context run failed. [errno = {}, errmsg = {}]", moduleName_,
          ec.value(), ec.message());
    return;
  }
  LOG_D("[{}] IO context run successful. [errno = {}, errmsg = {}]",
        moduleName_, ec.value(), ec.message());
}

void SignalHandler::setDefaultSignalHandler(const CBSignalHandler& value) {
  defaultSignalHandler_ = value;
}

void SignalHandler::registSignal() {
#if defined(__linux__)
  signalGroup_.add(SIGUSR1);
  signalGroup_.add(SIGUSR2);
#endif
  signalGroup_.add(SIGINT);
  signalGroup_.add(SIGTERM);
#if defined(SIGQUIT)
  signalGroup_.add(SIGQUIT);
#endif  // defined(SIGQUIT)
#if defined(__linux__)
  signalGroup_.add(SIGCHLD);
#endif
  signalGroup_.add(SIGPIPE);
  signalGroup_.async_wait([this](const boost::system::error_code& e, int s) {
    handleSignal(e, s);
  });
}

void SignalHandler::handleSignal(const boost::system::error_code& e,
                                 int signalNumber) {
  std::string signal_name = fmt::format("{}", signalNumber);
#if defined(__linux__)
  signal_name = strsignal(signalNumber);
#endif
  LOG_D(
      "[{}] Receive signal. "
      "[signalNumber = {}, signal_name = {}, errno = {}, errmsg = {}]",
      moduleName_, signalNumber, signal_name, e.value(), e.message());
  switch (signalNumber) {
    case SIGINT:
    case SIGTERM:
#if defined(SIGQUIT)
    case SIGQUIT:
#endif
      if (exitHandler_) {
        exitHandler_(e, signalNumber);
      }
      io_.stop();
      LOG_D("[{}] Signal svc stop.", moduleName_);
      break;
#if defined(__linux__)
    case SIGCHLD: {
      pid_t pid = 0;
      do {
        pid = waitpid(-1, NULL, WNOHANG);
      } while (pid > 0);
    } break;
#endif
    case SIGPIPE:
      break;
    default:
      if (defaultSignalHandler_) {
        defaultSignalHandler_(e, signalNumber);
      }
      break;
  }

  registSignal();
}

}  // namespace bq
