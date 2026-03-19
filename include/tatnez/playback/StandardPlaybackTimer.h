#pragma once

#include "tatnez/playback/IPlaybackTimer.h"

namespace tatnez::playback {

/**
 * @brief Production timer implementation that simply sleeps the current thread.
 */
class StandardPlaybackTimer final : public IPlaybackTimer {
public:
  void sleepFor(std::chrono::microseconds requestedSleepDuration) override;
};

} // namespace tatnez::playback
