#include "tatnez/rumble/RumbleSpeakerService.h"

#include <spdlog/logger.h>

#include <stdexcept>
#include <string>
#include <utility>

namespace tatnez::rumble {

RumbleSpeakerService::RumbleSpeakerService(
    std::shared_ptr<controller::IControllerRumbleBackend> controllerRumbleBackend,
    std::shared_ptr<spdlog::logger> applicationLogger,
    std::shared_ptr<playback::IPlaybackTimer> playbackTimer)
    : controllerRumbleBackend_(std::move(controllerRumbleBackend)),
      applicationLogger_(std::move(applicationLogger)), playbackTimer_(std::move(playbackTimer)) {
  if (controllerRumbleBackend_ == nullptr) {
    throw std::invalid_argument("controllerRumbleBackend must not be null.");
  }

  if (applicationLogger_ == nullptr) {
    throw std::invalid_argument("applicationLogger must not be null.");
  }

  if (playbackTimer_ == nullptr) {
    throw std::invalid_argument("playbackTimer must not be null.");
  }
}

auto RumbleSpeakerService::playFrames(std::uint32_t controllerIndex,
                                      const std::vector<RumbleFrame>& rumbleFrames) const
    -> RumblePlaybackStatistics {
  const auto controllerConnectionSummary =
      controllerRumbleBackend_->queryControllerConnectionSummary(controllerIndex);

  if (!controllerConnectionSummary.isConnected) {
    throw std::runtime_error(
        "Controller slot " + std::to_string(controllerIndex) +
        " is not connected or not available: " + controllerConnectionSummary.diagnosticMessage);
  }

  RumblePlaybackStatistics playbackStatistics{};

  try {
    applicationLogger_->info(
        "Beginning rumble playback on controller slot {} using backend {} with {} frames.", controllerIndex,
        controllerConnectionSummary.backendDisplayName, rumbleFrames.size());

    for (std::size_t frameIndex = 0; frameIndex < rumbleFrames.size(); ++frameIndex) {
      // Playback is intentionally explicit: apply one frame, wait for that frame's duration,
      // then move on to the next frame. That simplicity makes the timing model easier to teach
      // and easier to test.
      const auto& rumbleFrame = rumbleFrames.at(frameIndex);
      applicationLogger_->trace("Applying frame {} with largeMotorIntensity={}, smallMotorIntensity={}, "
                                "frameDuration={} microseconds.",
                                frameIndex, rumbleFrame.largeMotorIntensity, rumbleFrame.smallMotorIntensity,
                                rumbleFrame.frameDuration.count());

      controllerRumbleBackend_->setMotorIntensities(controllerIndex, rumbleFrame.largeMotorIntensity,
                                                    rumbleFrame.smallMotorIntensity);
      playbackTimer_->sleepFor(rumbleFrame.frameDuration);

      playbackStatistics.playedFrameCount += 1U;
      playbackStatistics.requestedPlaybackDuration += rumbleFrame.frameDuration;
    }

    controllerRumbleBackend_->stopMotors(controllerIndex);
    applicationLogger_->info("Completed playback on controller slot {} after {} frames and {} microseconds.",
                             controllerIndex, playbackStatistics.playedFrameCount,
                             playbackStatistics.requestedPlaybackDuration.count());
  } catch (...) {
    applicationLogger_->error(
        "Playback failed for controller slot {}. Attempting to stop the motors before rethrowing.",
        controllerIndex);
    controllerRumbleBackend_->stopMotors(controllerIndex);
    throw;
  }

  return playbackStatistics;
}

} // namespace tatnez::rumble
