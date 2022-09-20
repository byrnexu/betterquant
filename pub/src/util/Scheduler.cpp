/*!
 * \file Scheduler.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "util/Scheduler.hpp"

#include "util/Logger.hpp"

namespace bq {

Scheduler::Scheduler(const std::string& moduleName, const Callback& callback,
                     std::uint32_t interval, bool avgInterval,
                     std::uint32_t countMax)
    : moduleName_(moduleName),
      callback_(callback),
      interval_(interval),
      avgInterval_(avgInterval),
      count_(0),
      countMax_(countMax),
      timer_(io_) {
  io_.stop();
}

int Scheduler::start(const std::string& startTime) {
  if (interval_ == 0) {
    LOG_I("[{}] Start scheduler failed because of interval equal to 0.",
          moduleName_);
    return -1;
  }

  bool ioServiceIsReady = false;
  io_.post([&ioServiceIsReady]() { ioServiceIsReady = true; });

  auto ret = initTimer(startTime);
  if (ret != 0) {
    LOG_W("[{}] Start scheduler failed because of init timer failed.",
          moduleName_);
    return ret;
  }
  threadIO_ =
      std::unique_ptr<std::thread>(new std::thread([this]() { startIO(); }));

  while (ioServiceIsReady == false) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  };

  return 0;
}

int Scheduler::stop() {
  io_.stop();
  if (threadIO_ == nullptr) {
    LOG_I("[{}] IO context running thread is a null pointer.", moduleName_);
    return 0;
  }
  if (threadIO_->joinable()) {
    threadIO_->join();
  }
  LOG_D("[{}] IO context stop.", moduleName_);
  return 0;
}

inline boost::asio::io_context& Scheduler::io() { return io_; }

inline void Scheduler::resetInterval(std::uint32_t interval) {
  interval_ = interval;
}

int Scheduler::initTimer(const std::string& startTime) {
  count_ = 0;

  boost::system::error_code ec;
  if (startTime.empty()) {
    startDatetime_ = boost::posix_time::microsec_clock::local_time();
    timer_.expires_from_now(boost::posix_time::milliseconds(interval_), ec);
    if (ec) {
      LOG_E(
          "[{}] The timer set the trigger interval failed. "
          "[errno = {}, errmsg = {}]",
          moduleName_, ec.value(), ec.message());
      return -1;
    }
    timer_.async_wait(
        [this](const boost::system::error_code& e) { return callFunc(e); });
    return 0;
  }

  const auto now = boost::posix_time::microsec_clock::local_time();
  calcStartDatetime(startTime, now);

  const auto timeDur = startDatetime_ - now;
  timer_.expires_from_now(timeDur, ec);
  if (ec) {
    LOG_E(
        "[{}] The timer set the trigger interval failed. "
        "[errno = {}, errmsg = {}]",
        moduleName_, ec.value(), ec.message());
    return -1;
  }

  timer_.async_wait(
      [this](const boost::system::error_code& e) { return callFunc(e); });

  return 0;
}

void Scheduler::calcStartDatetime(const std::string& startTime,
                                  const boost::posix_time::ptime& now) {
  const auto today = boost::gregorian::day_clock::local_day();
  startDatetime_ = boost::posix_time::from_iso_string(
      boost::gregorian::to_iso_string(today) + "T" + startTime);
  if (now >= startDatetime_) {
    const auto tomorrow = boost::gregorian::day_clock::local_day() +
                          boost::gregorian::date_duration(1);
    startDatetime_ = boost::posix_time::from_iso_string(
        boost::gregorian::to_iso_string(tomorrow) + "T" + startTime);
  }
}

void Scheduler::startIO() {
  io_.reset();
  boost::system::error_code ec;
  const auto numOfHandles = io_.run(ec);
  if (!ec) {
    LOG_D("[{}] IO context run succcessful. [numOfHandles = {}]", moduleName_,
          numOfHandles);
  } else {
    LOG_E(
        "[{}] IO context run failed. [numOfHandles = {}, "
        "errno = {}, errmsg = {}]",
        moduleName_, numOfHandles, ec.value(), ec.message());
  }
}

void Scheduler::callFunc(const boost::system::error_code& e) {
  if (e) {
    LOG_D("[{}] Call the timing function failed. [errno = {}, errmsg = {}]",
          moduleName_, e.value(), e.message());
    return;
  }

  callback_();
  count_ += 1;

  if (countMax_ != 0 && count_ == countMax_) {
    LOG_D(
        "[{}] Because the timer function is called the specified number of "
        "times, the timer function is no longer executed. "
        "[count = {}, countMax = {}]",
        moduleName_, count_, countMax_);
    return;
  }

  const auto timeDur =
      boost::posix_time::microsec_clock::local_time() - startDatetime_;
  auto elapseTime = timeDur.total_milliseconds() % interval_;
  auto newInterval = avgInterval_ ? interval_ : interval_ - elapseTime;

  boost::system::error_code ec;
  timer_.expires_from_now(boost::posix_time::milliseconds(newInterval), ec);

  if (ec) {
    LOG_E(
        "[{}] The timer set the trigger interval failed. "
        "[errno = {}, errmsg = {}]",
        moduleName_, ec.value(), ec.message());
    return;
  }

  timer_.async_wait(
      [this](const boost::system::error_code& e) { return callFunc(e); });
}

}  // namespace bq
