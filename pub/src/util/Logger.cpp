/*!
 * \file Logger.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "util/Logger.hpp"

namespace bq {

int InitLogger(const std::string& configFilename) {
  YAML::Node config;
  try {
    config = YAML::LoadFile(configFilename);
  } catch (const std::exception& e) {
    std::cerr << fmt::format("Init logger by config file {} failed. [{}]",
                             configFilename, e.what())
              << std::endl;
    return -1;
  }
  return InitLogger(config);
}

int InitLogger(const YAML::Node& config) {
  try {
    const auto queueSize = config["logger"]["queueSize"].as<std::uint32_t>();
    const auto backingThreadsCount =
        config["logger"]["backingThreadsCount"].as<std::uint32_t>();
    spdlog::init_thread_pool(queueSize, backingThreadsCount);

    for (std::size_t i = 0; i < config["logger"]["loggerGroup"].size(); ++i) {
      const auto outputDir =
          config["logger"]["loggerGroup"][i]["outputDir"].as<std::string>();
      const auto outputFilename =
          config["logger"]["loggerGroup"][i]["outputFilename"]
              .as<std::string>();
      const auto maxSize =
          config["logger"]["loggerGroup"][i]["maxSize"].as<std::uint32_t>();
      const auto maxFiles =
          config["logger"]["loggerGroup"][i]["maxFiles"].as<std::uint32_t>();
      const auto rotatingSinkPattern =
          config["logger"]["loggerGroup"][i]["rotatingSinkPattern"]
              .as<std::string>();

      const auto loggerFilenameOfDebugLevel =
          fmt::format("{}/{}.log.debug", outputDir, outputFilename);
      auto rotatingSinkOfDebugLevel =
          std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
              loggerFilenameOfDebugLevel, maxSize, maxFiles);
      rotatingSinkOfDebugLevel->set_level(spdlog::level::debug);
      rotatingSinkOfDebugLevel->set_pattern(rotatingSinkPattern);

      const auto loggerFilenameOfInfoLevel =
          fmt::format("{}/{}.log.info", outputDir, outputFilename);
      auto rotatingSinkOfInfoLevel =
          std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
              loggerFilenameOfInfoLevel, maxSize, maxFiles);
      rotatingSinkOfInfoLevel->set_level(spdlog::level::info);
      rotatingSinkOfInfoLevel->set_pattern(rotatingSinkPattern);

      const auto loggerFilenameOfWarnLevel =
          fmt::format("{}/{}.log.warn", outputDir, outputFilename);
      auto rotatingSinkOfWarnLevel =
          std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
              loggerFilenameOfWarnLevel, maxSize, maxFiles);
      rotatingSinkOfWarnLevel->set_level(spdlog::level::warn);
      rotatingSinkOfWarnLevel->set_pattern(rotatingSinkPattern);

      const auto stdoutSinkPattern =
          config["logger"]["loggerGroup"][i]["stdoutSinkPattern"]
              .as<std::string>();
      auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      stdoutSink->set_level(spdlog::level::info);
      stdoutSink->set_pattern(stdoutSinkPattern);

      std::vector<spdlog::sink_ptr> sinks{rotatingSinkOfDebugLevel,
                                          rotatingSinkOfInfoLevel,
                                          rotatingSinkOfWarnLevel, stdoutSink};

      const auto loggerName =
          config["logger"]["loggerGroup"][i]["loggerName"].as<std::string>();
      auto asyncLogger = std::make_shared<spdlog::async_logger>(
          loggerName, sinks.begin(), sinks.end(), spdlog::thread_pool(),
          spdlog::async_overflow_policy::overrun_oldest);
      spdlog::register_logger(asyncLogger);
      spdlog::set_level(spdlog::level::trace);
    }

    const auto defaultLoggerName =
        config["logger"]["defaultLoggerName"].as<std::string>();
    const auto defaultLogger = spdlog::get(defaultLoggerName);
    spdlog::set_default_logger(defaultLogger);

  } catch (const std::exception& e) {
    std::cerr << fmt::format("Init logger by config failed. [{}]", e.what())
              << std::endl;
    return -1;
  }

  return 0;
}

std::shared_ptr<spdlog::async_logger> makeLogger(
    const std::string& configFilename) {
  YAML::Node config;
  try {
    config = YAML::LoadFile(configFilename);
  } catch (const std::exception& e) {
    std::cerr << fmt::format("Init logger by config file {} failed. [{}]",
                             configFilename, e.what())
              << std::endl;
    return nullptr;
  }
  return makeLogger(config);
}

std::shared_ptr<spdlog::async_logger> makeLogger(const YAML::Node& config) {
  try {
    const auto outputDir = config["logger"]["outputDir"].as<std::string>();
    const auto outputFilename =
        config["logger"]["outputFilename"].as<std::string>();
    const auto maxSize = config["logger"]["maxSize"].as<std::uint32_t>();
    const auto maxFiles = config["logger"]["maxFiles"].as<std::uint32_t>();
    const auto rotatingSinkPattern =
        config["logger"]["rotatingSinkPattern"].as<std::string>();

    const auto loggerFilenameOfDebugLevel =
        fmt::format("{}/{}.log.debug", outputDir, outputFilename);
    auto rotatingSinkOfDebugLevel =
        std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            loggerFilenameOfDebugLevel, maxSize, maxFiles);
    rotatingSinkOfDebugLevel->set_level(spdlog::level::debug);
    rotatingSinkOfDebugLevel->set_pattern(rotatingSinkPattern);

    const auto loggerFilenameOfInfoLevel =
        fmt::format("{}/{}.log.info", outputDir, outputFilename);
    auto rotatingSinkOfInfoLevel =
        std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            loggerFilenameOfInfoLevel, maxSize, maxFiles);
    rotatingSinkOfInfoLevel->set_level(spdlog::level::info);
    rotatingSinkOfInfoLevel->set_pattern(rotatingSinkPattern);

    const auto loggerFilenameOfWarnLevel =
        fmt::format("{}/{}.log.warn", outputDir, outputFilename);
    auto rotatingSinkOfWarnLevel =
        std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            loggerFilenameOfWarnLevel, maxSize, maxFiles);
    rotatingSinkOfWarnLevel->set_level(spdlog::level::warn);
    rotatingSinkOfWarnLevel->set_pattern(rotatingSinkPattern);

    const auto stdoutSinkPattern =
        config["logger"]["stdoutSinkPattern"].as<std::string>();
    auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    stdoutSink->set_level(spdlog::level::info);
    stdoutSink->set_pattern(stdoutSinkPattern);

    std::vector<spdlog::sink_ptr> sinks{rotatingSinkOfDebugLevel,
                                        rotatingSinkOfInfoLevel,
                                        rotatingSinkOfWarnLevel, stdoutSink};

    const auto loggerName = config["logger"]["loggerName"].as<std::string>();
    auto asyncLogger = std::make_shared<spdlog::async_logger>(
        loggerName, sinks.begin(), sinks.end(), spdlog::thread_pool(),
        spdlog::async_overflow_policy::overrun_oldest);
    spdlog::set_level(spdlog::level::trace);
    return asyncLogger;

  } catch (const std::exception& e) {
    std::cerr << fmt::format("Init logger by config failed. [{}]", e.what())
              << std::endl;
    return nullptr;
  }

  return nullptr;
}

}  // namespace bq
