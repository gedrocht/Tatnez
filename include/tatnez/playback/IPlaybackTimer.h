#pragma once

#include <chrono>

namespace tatnez::playback {

/**
 * @brief Small seam that allows tests to observe and control time-based playback behavior.
 */
class IPlaybackTimer {
public:
  IPlaybackTimer() = default;
  IPlaybackTimer(const IPlaybackTimer&) = delete;
  auto operator=(const IPlaybackTimer&) -> IPlaybackTimer& = delete;
  IPlaybackTimer(IPlaybackTimer&&) = delete;
  auto operator=(IPlaybackTimer&&) -> IPlaybackTimer& = delete;
  virtual ~IPlaybackTimer() = default;

  /**
   * @brief Pause for the requested amount of time.
   */
  virtual void sleepFor(std::chrono::microseconds requestedSleepDuration) = 0;
};

} // namespace tatnez::playback
