#pragma once

#include <chrono>
#include <cstdint>

namespace tatnez::rumble {

/**
 * @brief One time slice of controller motor output.
 *
 * Think of this structure as the rumble equivalent of one audio frame. The application computes
 * a sequence of these frames and then sends them to the controller one after another.
 */
struct RumbleFrame final {
  /**
   * @brief Low-frequency motor intensity in the range [0, 65535].
   */
  std::uint16_t largeMotorIntensity{};

  /**
   * @brief High-frequency motor intensity in the range [0, 65535].
   */
  std::uint16_t smallMotorIntensity{};

  /**
   * @brief How long this frame should be held before the next frame is applied.
   */
  std::chrono::microseconds frameDuration{};
};

} // namespace tatnez::rumble
