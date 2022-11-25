/*!
 * \file Logger.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "PchBase.hpp"
#ifndef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVESPDL_TRACE
#endif
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

#define SPDL_T(logger_name, ...)                              \
  {                                                           \
    auto logger = spdlog::get((logger_name));                 \
    if (logger == nullptr) logger = spdlog::default_logger(); \
    SPDLOG_LOGGER_TRACE(logger, __VA_ARGS__);                 \
  }

#define SPDL_D(logger_name, ...)                              \
  {                                                           \
    auto logger = spdlog::get((logger_name));                 \
    if (logger == nullptr) logger = spdlog::default_logger(); \
    SPDLOG_LOGGER_DEBUG(logger, __VA_ARGS__);                 \
  }

#define SPDL_I(logger_name, ...)                              \
  {                                                           \
    auto logger = spdlog::get((logger_name));                 \
    if (logger == nullptr) logger = spdlog::default_logger(); \
    SPDLOG_LOGGER_INFO(logger, __VA_ARGS__);                  \
  }

#define SPDL_W(logger_name, ...)                              \
  {                                                           \
    auto logger = spdlog::get((logger_name));                 \
    if (logger == nullptr) logger = spdlog::default_logger(); \
    SPDLOG_LOGGER_WARN(logger, __VA_ARGS__);                  \
  }

#define SPDL_E(logger_name, ...)                              \
  {                                                           \
    auto logger = spdlog::get((logger_name));                 \
    if (logger == nullptr) logger = spdlog::default_logger(); \
    SPDLOG_LOGGER_ERROR(logger, __VA_ARGS__);                 \
  }

#define SPDL_C(logger_name, ...)                              \
  {                                                           \
    auto logger = spdlog::get((logger_name));                 \
    if (logger == nullptr) logger = spdlog::default_logger(); \
    SPDLOG_LOGGER_CRITICAL(logger, __VA_ARGS__);              \
  }

#define LOG_T(...) SPDL_T("", __VA_ARGS__)
#define LOG_D(...) SPDL_D("", __VA_ARGS__)
#define LOG_I(...) SPDL_I("", __VA_ARGS__)
#define LOG_W(...) SPDL_W("", __VA_ARGS__)
#define LOG_E(...) SPDL_E("", __VA_ARGS__)
#define LOG_C(...) SPDL_C("", __VA_ARGS__)
