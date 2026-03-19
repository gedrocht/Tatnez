#pragma once

#include <chrono>
#include <cstddef>

namespace tatnez::rumble {

/**
 * @brief Summarizes what happened during a playback run.
 */
struct RumblePlaybackStatistics final {
  /**
   * @brief The number of frames that were sent to the backend.
   */
  std::size_t playedFrameCount{};

  /**
   * @brief The total requested frame duration across the playback run.
   */
  std::chrono::microseconds requestedPlaybackDuration{};
};

} // namespace tatnez::rumble
