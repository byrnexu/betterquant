/*!
 * \file SvcBase.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "util/PchBase.hpp"

namespace boost::system {
class error_code;
}

namespace bq {

enum class InstallSignalHandler { True = 1, False = 2 };

class SignalHandler;
using SignalHandlerSPtr = std::shared_ptr<SignalHandler>;

class SvcBase {
 public:
  SvcBase(const SvcBase&) = delete;
  SvcBase& operator=(const SvcBase&) = delete;
  SvcBase(const SvcBase&&) = delete;
  SvcBase& operator=(const SvcBase&&) = delete;

  SvcBase() = default;
  SvcBase(
      const std::string& configFilename,
      InstallSignalHandler installSignalHandler = InstallSignalHandler::True);

 public:
  int init(
      const std::string& configFilename = "",
      InstallSignalHandler installSignalHandler = InstallSignalHandler::True);

 private:
  virtual int prepareInit() { return 0; }
  virtual int beforeInit() { return 0; }
  virtual int doInit() { return 0; }
  virtual int afterInit() { return 0; }

 public:
  int run();

 private:
  virtual int beforeRun() { return 0; }
  virtual int doRun() = 0;
  virtual int afterRun();

 private:
  void exit(const boost::system::error_code* ec, int signalNum);

 private:
  virtual void beforeExit(const boost::system::error_code* ec, int signalNum) {}
  virtual void doExit(const boost::system::error_code* ec, int signalNum) {}
  virtual void afterExit(const boost::system::error_code* ec, int signalNum) {}

 protected:
  std::string configFilename_;

 private:
  InstallSignalHandler installSignalHandler{InstallSignalHandler::True};
  SignalHandlerSPtr signalHandler_{nullptr};
};

}  // namespace bq
