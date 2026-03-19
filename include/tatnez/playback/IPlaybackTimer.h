#pragma once

#include <chrono>

namespace tatnez::playback {

/**
 * @brief Small seam that allows tests to observe and control time-based playback behavior.
 */
class IPlaybackTimer {
public:
  virtual ~IPlaybackTimer() = default;

  /**
   * @brief Pause for the requested amount of time.
   */
  virtual void sleepFor(std::chrono::microseconds requestedSleepDuration) = 0;
};

} // namespace tatnez::playback
