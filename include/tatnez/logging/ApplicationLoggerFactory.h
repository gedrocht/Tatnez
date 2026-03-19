#pragma once

#include <filesystem>
#include <memory>
#include <string>

namespace spdlog {
class logger;
}

namespace tatnez::logging {

/**
 * @brief Creates the application logger and resolves the log directory.
 *
 * Logging is intentionally a first-class feature in this project because hardware experiments
 * are much easier to debug when every decision is written to a searchable log file.
 */
class ApplicationLoggerFactory final {
public:
  /**
   * @brief Create a logger that writes both to the console and to a rotating file.
   *
   * @param loggerName A stable logger name used by spdlog.
   * @param requestedLogDirectory An optional explicit directory chosen by the user.
   */
  [[nodiscard]] static auto
  createLogger(const std::string& loggerName,
               const std::filesystem::path& requestedLogDirectory = {}) -> std::shared_ptr<spdlog::logger>;

  /**
   * @brief Determine where logs should be stored when the user did not provide a directory.
   */
  [[nodiscard]] static auto determineDefaultLogDirectory() -> std::filesystem::path;
};

} // namespace tatnez::logging
