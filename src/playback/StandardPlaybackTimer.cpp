#include "tatnez/playback/StandardPlaybackTimer.h"

#include <thread>

namespace tatnez::playback {

void StandardPlaybackTimer::sleepFor(std::chrono::microseconds requestedSleepDuration) {
  std::this_thread::sleep_for(requestedSleepDuration);
}

} // namespace tatnez::playback
