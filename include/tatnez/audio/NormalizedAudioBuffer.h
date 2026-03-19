#pragma once

#include <cstdint>
#include <vector>

namespace tatnez::audio {

/**
 * @brief Stores mono audio samples that have already been normalized into a floating-point range.
 *
 * The normalization convention used by this project is intentionally simple for beginners:
 * every sample is expected to live within the inclusive range [-1.0, 1.0]. A value of 0.0
 * represents silence, positive values represent one direction of the waveform, and negative
 * values represent the opposite direction of the waveform.
 */
struct NormalizedAudioBuffer final {
  /**
   * @brief The number of audio samples that occur every second.
   *
   * A value such as 44,100 means that the source file contains 44,100 samples for every
   * second of sound.
   */
  std::uint32_t sampleRateInHertz{};

  /**
   * @brief The mono audio samples, each already normalized to the [-1.0, 1.0] range.
   *
   * Stereo files are down-mixed into this mono representation by averaging the channels.
   */
  std::vector<float> monoSamples{};
};

} // namespace tatnez::audio
