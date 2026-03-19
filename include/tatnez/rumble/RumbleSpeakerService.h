#pragma once

#include "tatnez/controller/IControllerRumbleBackend.h"
#include "tatnez/playback/IPlaybackTimer.h"
#include "tatnez/rumble/RumbleFrame.h"
#include "tatnez/rumble/RumblePlaybackStatistics.h"

#include <memory>
#include <vector>

namespace spdlog {
class logger;
}

namespace tatnez::rumble {

/**
 * @brief Plays precomputed rumble frames through a controller backend.
 */
class RumbleSpeakerService final {
public:
  /**
   * @brief Create the playback service.
   *
   * @param controllerRumbleBackend The backend that talks to real or simulated controllers.
   * @param applicationLogger The logger that should receive playback diagnostics.
   * @param playbackTimer The timer used between frames.
   */
  RumbleSpeakerService(std::shared_ptr<controller::IControllerRumbleBackend> controllerRumbleBackend,
                       std::shared_ptr<spdlog::logger> applicationLogger,
                       std::shared_ptr<playback::IPlaybackTimer> playbackTimer);

  /**
   * @brief Play a sequence of rumble frames through one controller slot.
   */
  [[nodiscard]] auto
  playFrames(std::uint32_t controllerIndex,
             const std::vector<RumbleFrame>& rumbleFrames) const -> RumblePlaybackStatistics;

private:
  std::shared_ptr<controller::IControllerRumbleBackend> controllerRumbleBackend_;
  std::shared_ptr<spdlog::logger> applicationLogger_;
  std::shared_ptr<playback::IPlaybackTimer> playbackTimer_;
};

} // namespace tatnez::rumble
