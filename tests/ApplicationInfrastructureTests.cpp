#include "tatnez/controller/PlatformControllerRumbleBackendFactory.h"
#include "tatnez/logging/ApplicationLoggerFactory.h"
#include "tatnez/playback/StandardPlaybackTimer.h"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

auto createUniqueTemporaryDirectoryPath(const std::string& directoryPrefix) -> std::filesystem::path {
  const auto uniqueSuffix = std::chrono::steady_clock::now().time_since_epoch().count();
  return std::filesystem::temp_directory_path() / (directoryPrefix + "-" + std::to_string(uniqueSuffix));
}

#ifndef _WIN32
class ScopedEnvironmentVariableOverride final {
public:
  ScopedEnvironmentVariableOverride(std::string environmentVariableName, std::string replacementValue)
      : environmentVariableName_(std::move(environmentVariableName)) {
    if (const auto* originalEnvironmentVariableValue = std::getenv(environmentVariableName_.c_str());
        originalEnvironmentVariableValue != nullptr) {
      originalEnvironmentVariableValue_ = std::string(originalEnvironmentVariableValue);
    }

    if (setenv(environmentVariableName_.c_str(), replacementValue.c_str(), 1) != 0) {
      throw std::runtime_error("Failed to set the test environment variable override.");
    }
  }

  ~ScopedEnvironmentVariableOverride() {
    if (originalEnvironmentVariableValue_.has_value()) {
      static_cast<void>(
          setenv(environmentVariableName_.c_str(), originalEnvironmentVariableValue_->c_str(), 1));
      return;
    }

    static_cast<void>(unsetenv(environmentVariableName_.c_str()));
  }

private:
  std::string environmentVariableName_;
  std::optional<std::string> originalEnvironmentVariableValue_;
};
#endif

TEST(ApplicationInfrastructureTests, CreateLoggerCreatesTheRequestedLogFileAndReusesTheLoggerName) {
  const auto temporaryLogDirectoryPath = createUniqueTemporaryDirectoryPath("tatnez-logger-test");
  const auto loggerName =
      "tatnez-test-logger-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());

  auto firstLogger =
      tatnez::logging::ApplicationLoggerFactory::createLogger(loggerName, temporaryLogDirectoryPath);
  firstLogger->info("Logger creation test message.");
  firstLogger->flush();

  auto secondLogger =
      tatnez::logging::ApplicationLoggerFactory::createLogger(loggerName, temporaryLogDirectoryPath);

  EXPECT_EQ(firstLogger.get(), secondLogger.get());
  EXPECT_TRUE(std::filesystem::exists(temporaryLogDirectoryPath / "tatnez-rumble-speaker.log"));

  spdlog::drop(loggerName);
  firstLogger.reset();
  secondLogger.reset();
  std::filesystem::remove_all(temporaryLogDirectoryPath);
}

TEST(ApplicationInfrastructureTests, DetermineDefaultLogDirectoryFallsBackToCurrentPathOutsideWindows) {
#ifdef _WIN32
  GTEST_SKIP() << "This fallback path is only expected on non-Windows platforms.";
#else
  EXPECT_EQ(tatnez::logging::ApplicationLoggerFactory::determineDefaultLogDirectory(),
            std::filesystem::current_path() / "logs");
#endif
}

TEST(ApplicationInfrastructureTests,
     DetermineDefaultLogDirectoryUsesLocalApplicationDataWhenItIsAvailableOutsideWindows) {
#ifdef _WIN32
  GTEST_SKIP() << "This environment-variable branch is only exercised in the non-Windows coverage job.";
#else
  const auto temporaryLocalApplicationDataDirectoryPath =
      createUniqueTemporaryDirectoryPath("tatnez-local-application-data");
  [[maybe_unused]] const ScopedEnvironmentVariableOverride scopedEnvironmentVariableOverride(
      "LOCALAPPDATA", temporaryLocalApplicationDataDirectoryPath.string());

  EXPECT_EQ(tatnez::logging::ApplicationLoggerFactory::determineDefaultLogDirectory(),
            temporaryLocalApplicationDataDirectoryPath / "TatnezRumbleSpeaker" / "logs");
#endif
}

TEST(ApplicationInfrastructureTests, CreateLoggerUsesTheDefaultLogDirectoryWhenNoDirectoryIsRequested) {
#ifdef _WIN32
  GTEST_SKIP()
      << "The non-Windows coverage job exercises this branch with a controlled environment variable.";
#else
  const auto temporaryLocalApplicationDataDirectoryPath =
      createUniqueTemporaryDirectoryPath("tatnez-default-logger");
  [[maybe_unused]] const ScopedEnvironmentVariableOverride scopedEnvironmentVariableOverride(
      "LOCALAPPDATA", temporaryLocalApplicationDataDirectoryPath.string());
  const auto loggerName = "tatnez-default-log-directory-test-" +
                          std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());

  auto logger = tatnez::logging::ApplicationLoggerFactory::createLogger(loggerName);
  logger->info("Logger default-directory test message.");
  logger->flush();

  EXPECT_TRUE(std::filesystem::exists(temporaryLocalApplicationDataDirectoryPath / "TatnezRumbleSpeaker" /
                                      "logs" / "tatnez-rumble-speaker.log"));

  spdlog::drop(loggerName);
  logger.reset();
  std::filesystem::remove_all(temporaryLocalApplicationDataDirectoryPath);
#endif
}

TEST(ApplicationInfrastructureTests, PlatformControllerBackendExplainsUnsupportedPlatforms) {
#ifdef _WIN32
  GTEST_SKIP() << "This test targets the non-Windows fallback backend.";
#else
  const auto controllerRumbleBackend = tatnez::controller::createPlatformControllerRumbleBackend();
  const auto controllerConnectionSummary = controllerRumbleBackend->queryControllerConnectionSummary(0U);

  EXPECT_FALSE(controllerConnectionSummary.isConnected);
  EXPECT_EQ(controllerConnectionSummary.backendDisplayName, "Unsupported platform");
  EXPECT_THROW(controllerRumbleBackend->setMotorIntensities(0U, 100U, 100U), std::runtime_error);
  EXPECT_NO_THROW(controllerRumbleBackend->stopMotors(0U));
#endif
}

TEST(ApplicationInfrastructureTests, StandardPlaybackTimerAcceptsZeroDurationSleeps) {
  tatnez::playback::StandardPlaybackTimer standardPlaybackTimer;
  EXPECT_NO_THROW(standardPlaybackTimer.sleepFor(std::chrono::microseconds::zero()));
}

} // namespace
