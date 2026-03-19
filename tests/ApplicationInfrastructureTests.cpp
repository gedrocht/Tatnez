#include "tatnez/controller/PlatformControllerRumbleBackendFactory.h"
#include "tatnez/logging/ApplicationLoggerFactory.h"
#include "tatnez/playback/StandardPlaybackTimer.h"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <filesystem>
#include <stdexcept>
#include <string>

namespace
{

auto createUniqueTemporaryDirectoryPath(const std::string& directoryPrefix) -> std::filesystem::path
{
  const auto uniqueSuffix = std::chrono::steady_clock::now().time_since_epoch().count();
  return std::filesystem::temp_directory_path()
    / (directoryPrefix + "-" + std::to_string(uniqueSuffix));
}

TEST(ApplicationInfrastructureTests, CreateLoggerCreatesTheRequestedLogFileAndReusesTheLoggerName)
{
  const auto temporaryLogDirectoryPath = createUniqueTemporaryDirectoryPath("tatnez-logger-test");
  const auto loggerName = "tatnez-test-logger-" +
    std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());

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

TEST(ApplicationInfrastructureTests, DetermineDefaultLogDirectoryFallsBackToCurrentPathOutsideWindows)
{
#ifdef _WIN32
  GTEST_SKIP() << "This fallback path is only expected on non-Windows platforms.";
#else
  EXPECT_EQ(
    tatnez::logging::ApplicationLoggerFactory::determineDefaultLogDirectory(),
    std::filesystem::current_path() / "logs");
#endif
}

TEST(ApplicationInfrastructureTests, PlatformControllerBackendExplainsUnsupportedPlatforms)
{
#ifdef _WIN32
  GTEST_SKIP() << "This test targets the non-Windows fallback backend.";
#else
  const auto controllerRumbleBackend = tatnez::controller::createPlatformControllerRumbleBackend();
  const auto controllerConnectionSummary =
    controllerRumbleBackend->queryControllerConnectionSummary(0U);

  EXPECT_FALSE(controllerConnectionSummary.isConnected);
  EXPECT_EQ(controllerConnectionSummary.backendDisplayName, "Unsupported platform");
  EXPECT_THROW(
    controllerRumbleBackend->setMotorIntensities(0U, 100U, 100U),
    std::runtime_error);
  EXPECT_NO_THROW(controllerRumbleBackend->stopMotors(0U));
#endif
}

TEST(ApplicationInfrastructureTests, StandardPlaybackTimerAcceptsZeroDurationSleeps)
{
  tatnez::playback::StandardPlaybackTimer standardPlaybackTimer;
  EXPECT_NO_THROW(standardPlaybackTimer.sleepFor(std::chrono::microseconds::zero()));
}

}  // namespace
