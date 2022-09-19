#pragma once

#include "util/Pch.hpp"

#define L_T(...) \
  { SPDLOG_LOGGER_TRACE(logger_, __VA_ARGS__); }

#define L_D(...) \
  { SPDLOG_LOGGER_DEBUG(logger_, __VA_ARGS__); }

#define L_I(...) \
  { SPDLOG_LOGGER_INFO(logger_, __VA_ARGS__); }

#define L_W(...) \
  { SPDLOG_LOGGER_WARN(logger_, __VA_ARGS__); }

#define L_E(...) \
  { SPDLOG_LOGGER_ERROR(logger_, __VA_ARGS__); }

#define L_C(...) \
  { SPDLOG_LOGGER_CRITICAL(logger_, __VA_ARGS__); }
