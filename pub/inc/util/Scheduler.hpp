#pragma once

#include "util/Pch.hpp"

namespace bq {

class Scheduler;
using SchedulerSPtr = std::shared_ptr<Scheduler>;

class Scheduler {
  using Callback = std::function<void()>;

 public:
  Scheduler(const std::string& moduleName, const Callback& callback,
            std::uint32_t interval, bool avgInterval = false,
            std::uint32_t countMax = 0);

 public:
  int start(const std::string& startTime = "");
  int stop();

 public:
  inline boost::asio::io_context& io();
  inline void resetInterval(std::uint32_t interval);

 private:
  int initTimer(const std::string& startTime);

  void calcStartDatetime(const std::string& startTime,
                         const boost::posix_time::ptime& now);

  void startIO();

  void callFunc(const boost::system::error_code& e);

 private:
  const std::string moduleName_;
  const Callback callback_;

  std::uint32_t interval_;
  const bool avgInterval_;

  std::uint32_t count_;
  const std::uint32_t countMax_;

  boost::asio::io_context io_;
  boost::asio::deadline_timer timer_;
  std::unique_ptr<std::thread> threadIO_{nullptr};

  boost::posix_time::ptime startDatetime_;
};

}  // namespace bq
