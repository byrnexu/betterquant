#pragma once

#include "PchBase.hpp"
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#define SPDLOG_FMT_EXTERNAL
#include <fmt/format.h>
#include <yaml-cpp/yaml.h>

namespace bq {

int InitLogger(const std::string& configFilename);
int InitLogger(const YAML::Node& config);

std::shared_ptr<spdlog::async_logger> makeLogger(
    const std::string& configFilename);
std::shared_ptr<spdlog::async_logger> makeLogger(const YAML::Node& config);

}  // namespace bq

#define LOG_TRACE(logger_name, ...)                           \
  {                                                           \
    auto logger = spdlog::get((logger_name));                 \
    if (logger == nullptr) logger = spdlog::default_logger(); \
    SPDLOG_LOGGER_TRACE(logger, __VA_ARGS__);                 \
  }

#define LOG_DEBUG(logger_name, ...)                           \
  {                                                           \
    auto logger = spdlog::get((logger_name));                 \
    if (logger == nullptr) logger = spdlog::default_logger(); \
    SPDLOG_LOGGER_DEBUG(logger, __VA_ARGS__);                 \
  }

#define LOG_INFO(logger_name, ...)                            \
  {                                                           \
    auto logger = spdlog::get((logger_name));                 \
    if (logger == nullptr) logger = spdlog::default_logger(); \
    SPDLOG_LOGGER_INFO(logger, __VA_ARGS__);                  \
  }

#define LOG_WARN(logger_name, ...)                            \
  {                                                           \
    auto logger = spdlog::get((logger_name));                 \
    if (logger == nullptr) logger = spdlog::default_logger(); \
    SPDLOG_LOGGER_WARN(logger, __VA_ARGS__);                  \
  }

#define LOG_ERROR(logger_name, ...)                           \
  {                                                           \
    auto logger = spdlog::get((logger_name));                 \
    if (logger == nullptr) logger = spdlog::default_logger(); \
    SPDLOG_LOGGER_ERROR(logger, __VA_ARGS__);                 \
  }

#define LOG_CRITICAL(logger_name, ...)                        \
  {                                                           \
    auto logger = spdlog::get((logger_name));                 \
    if (logger == nullptr) logger = spdlog::default_logger(); \
    SPDLOG_LOGGER_CRITICAL(logger, __VA_ARGS__);              \
  }

#define LOG_T(...) LOG_TRACE("", __VA_ARGS__)
#define LOG_D(...) LOG_DEBUG("", __VA_ARGS__)
#define LOG_I(...) LOG_INFO("", __VA_ARGS__)
#define LOG_W(...) LOG_WARN("", __VA_ARGS__)
#define LOG_E(...) LOG_ERROR("", __VA_ARGS__)
#define LOG_C(...) LOG_CRITICAL("", __VA_ARGS__)
