#include "tatnez/controller/IControllerRumbleBackend.h"
#include "tatnez/playback/IPlaybackTimer.h"
#include "tatnez/rumble/RumbleSpeakerService.h"

#include <gtest/gtest.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/null_sink.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

struct RecordedMotorCommand final {
  std::uint32_t controllerIndex{};
  std::uint16_t largeMotorIntensity{};
  std::uint16_t smallMotorIntensity{};
};

class RecordingControllerRumbleBackend final : public tatnez::controller::IControllerRumbleBackend {
public:
  bool shouldReportConnectedController = true;
  bool shouldThrowWhenSettingMotorIntensities = false;
  std::vector<RecordedMotorCommand> recordedMotorCommands;
  std::uint32_t stopInvocationCount = 0U;

  [[nodiscard]] auto getBackendDisplayName() const -> std::string override {
    return "recording test backend";
  }

  [[nodiscard]] auto queryControllerConnectionSummary(std::uint32_t controllerIndex) const
      -> tatnez::controller::ControllerConnectionSummary override {
    return tatnez::controller::ControllerConnectionSummary{
        controllerIndex, shouldReportConnectedController, getBackendDisplayName(),
        shouldReportConnectedController ? "connected" : "disconnected for test"};
  }

  void setMotorIntensities(std::uint32_t controllerIndex, std::uint16_t largeMotorIntensity,
                           std::uint16_t smallMotorIntensity) override {
    if (shouldThrowWhenSettingMotorIntensities) {
      throw std::runtime_error("Simulated backend failure.");
    }

    recordedMotorCommands.push_back(
        RecordedMotorCommand{controllerIndex, largeMotorIntensity, smallMotorIntensity});
  }

  void stopMotors(std::uint32_t /* controllerIndex */) override { stopInvocationCount += 1U; }
};

class RecordingPlaybackTimer final : public tatnez::playback::IPlaybackTimer {
public:
  std::vector<std::chrono::microseconds> recordedSleepDurations;

  void sleepFor(std::chrono::microseconds requestedSleepDuration) override {
    recordedSleepDurations.push_back(requestedSleepDuration);
  }
};

auto createNullLogger() -> std::shared_ptr<spdlog::logger> {
  return std::make_shared<spdlog::logger>("test-logger", std::make_shared<spdlog::sinks::null_sink_mt>());
}

TEST(RumbleSpeakerServiceTests, PlayFramesSendsAllFramesAndStopsMotors) {
  auto recordingControllerRumbleBackend = std::make_shared<RecordingControllerRumbleBackend>();
  auto recordingPlaybackTimer = std::make_shared<RecordingPlaybackTimer>();

  const tatnez::rumble::RumbleSpeakerService rumbleSpeakerService(recordingControllerRumbleBackend,
                                                                  createNullLogger(), recordingPlaybackTimer);

  const std::vector<tatnez::rumble::RumbleFrame> rumbleFrames{{100U, 200U, std::chrono::microseconds(1000)},
                                                              {300U, 400U, std::chrono::microseconds(2000)}};

  const auto playbackStatistics = rumbleSpeakerService.playFrames(1U, rumbleFrames);

  ASSERT_EQ(recordingControllerRumbleBackend->recordedMotorCommands.size(), 2U);
  EXPECT_EQ(recordingControllerRumbleBackend->recordedMotorCommands.front().controllerIndex, 1U);
  EXPECT_EQ(recordingPlaybackTimer->recordedSleepDurations.size(), 2U);
  EXPECT_EQ(recordingControllerRumbleBackend->stopInvocationCount, 1U);
  EXPECT_EQ(playbackStatistics.playedFrameCount, 2U);
  EXPECT_EQ(playbackStatistics.requestedPlaybackDuration, std::chrono::microseconds(3000));
}

TEST(RumbleSpeakerServiceTests, PlayFramesRejectsDisconnectedController) {
  auto recordingControllerRumbleBackend = std::make_shared<RecordingControllerRumbleBackend>();
  recordingControllerRumbleBackend->shouldReportConnectedController = false;

  const tatnez::rumble::RumbleSpeakerService rumbleSpeakerService(
      recordingControllerRumbleBackend, createNullLogger(), std::make_shared<RecordingPlaybackTimer>());

  EXPECT_THROW(static_cast<void>(rumbleSpeakerService.playFrames(0U, {})), std::runtime_error);
  EXPECT_TRUE(recordingControllerRumbleBackend->recordedMotorCommands.empty());
}

TEST(RumbleSpeakerServiceTests, PlayFramesStopsMotorsWhenTheBackendThrows) {
  auto recordingControllerRumbleBackend = std::make_shared<RecordingControllerRumbleBackend>();
  recordingControllerRumbleBackend->shouldThrowWhenSettingMotorIntensities = true;

  const tatnez::rumble::RumbleSpeakerService rumbleSpeakerService(
      recordingControllerRumbleBackend, createNullLogger(), std::make_shared<RecordingPlaybackTimer>());

  const std::vector<tatnez::rumble::RumbleFrame> rumbleFrames{{100U, 200U, std::chrono::microseconds(1000)}};

  EXPECT_THROW(static_cast<void>(rumbleSpeakerService.playFrames(0U, rumbleFrames)), std::runtime_error);
  EXPECT_EQ(recordingControllerRumbleBackend->stopInvocationCount, 1U);
}

} // namespace
