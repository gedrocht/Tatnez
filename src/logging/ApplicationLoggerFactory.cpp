#include "tatnez/logging/ApplicationLoggerFactory.h"

#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace {

auto tryGetEnvironmentVariableValue(const char* environmentVariableName) -> std::optional<std::string> {
#ifdef _WIN32
  char* duplicatedEnvironmentVariableValue = nullptr;
  std::size_t duplicatedEnvironmentVariableLength = 0U;
  const auto duplicationResultCode = _dupenv_s(&duplicatedEnvironmentVariableValue,
                                               &duplicatedEnvironmentVariableLength, environmentVariableName);

  if (duplicationResultCode != 0 || duplicatedEnvironmentVariableValue == nullptr ||
      duplicatedEnvironmentVariableLength == 0U) {
    return std::nullopt;
  }

  std::string environmentVariableValue{duplicatedEnvironmentVariableValue};
  std::free(duplicatedEnvironmentVariableValue);
  return environmentVariableValue;
#else
  const auto* environmentVariableValue = std::getenv(environmentVariableName);
  if (environmentVariableValue == nullptr || *environmentVariableValue == '\0') {
    return std::nullopt;
  }

  return std::string{environmentVariableValue};
#endif
}

} // namespace

namespace tatnez::logging {

auto ApplicationLoggerFactory::determineDefaultLogDirectory() -> std::filesystem::path {
  if (const auto localApplicationDataDirectory = tryGetEnvironmentVariableValue("LOCALAPPDATA");
      localApplicationDataDirectory.has_value()) {
    return std::filesystem::path(localApplicationDataDirectory.value()) / "TatnezRumbleSpeaker" / "logs";
  }

  return std::filesystem::current_path() / "logs";
}

auto ApplicationLoggerFactory::createLogger(const std::string& loggerName,
                                            const std::filesystem::path& requestedLogDirectory)
    -> std::shared_ptr<spdlog::logger> {
  if (const auto existingLogger = spdlog::get(loggerName)) {
    return existingLogger;
  }

  const auto resolvedLogDirectory =
      requestedLogDirectory.empty() ? determineDefaultLogDirectory() : requestedLogDirectory;
  std::filesystem::create_directories(resolvedLogDirectory);

  const auto logFilePath = resolvedLogDirectory / "tatnez-rumble-speaker.log";

  auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);

  consoleSink->set_level(spdlog::level::info);
  fileSink->set_level(spdlog::level::trace);

  auto createdLogger =
      std::make_shared<spdlog::logger>(loggerName, spdlog::sinks_init_list{consoleSink, fileSink});

  createdLogger->set_level(spdlog::level::trace);
  createdLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
  createdLogger->flush_on(spdlog::level::info);

  spdlog::register_logger(createdLogger);
  return createdLogger;
}

} // namespace tatnez::logging
