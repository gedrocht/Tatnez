#pragma once

#include "tatnez/audio/NormalizedAudioBuffer.h"
#include "tatnez/rumble/RumbleFrame.h"

#include <vector>

namespace tatnez::rumble {

/**
 * @brief Input configuration for synthetic tone playback.
 */
struct TonePlaybackRequest final {
  /**
   * @brief Requested oscillation frequency in hertz.
   *
   * This does not mean that the controller will behave like a real loudspeaker at this exact
   * acoustic frequency. The motors have significant physical limitations, so this value is best
   * thought of as a target modulation rate for a haptic approximation.
   */
  double toneFrequencyInHertz{40.0};

  /**
   * @brief Playback duration in seconds.
   */
  double durationInSeconds{2.0};

  /**
   * @brief How many rumble updates should be produced every second.
   */
  double updateRateInHertz{200.0};

  /**
   * @brief Master gain applied to both motors, expected in the range [0.0, 1.0].
   */
  double masterIntensityScale{1.0};
};

/**
 * @brief Input configuration for WAVE-to-rumble conversion.
 */
struct WavePlaybackRequest final {
  /**
   * @brief How many rumble frames should be produced for every second of source audio.
   */
  double updateRateInHertz{200.0};

  /**
   * @brief Overall gain applied to the generated motor intensities.
   */
  double masterIntensityScale{1.0};

  /**
   * @brief Scaling factor for the large low-frequency motor.
   */
  double largeMotorContributionScale{0.65};

  /**
   * @brief Scaling factor for the small high-frequency motor.
   */
  double smallMotorContributionScale{1.0};
};

/**
 * @brief Converts tones and normalized audio into controller rumble frames.
 */
class RumbleSignalGenerator final {
public:
  /**
   * @brief Generate rumble frames for a synthetic tone.
   */
  [[nodiscard]] auto generateToneFrames(const TonePlaybackRequest& tonePlaybackRequest) const
      -> std::vector<RumbleFrame>;

  /**
   * @brief Generate rumble frames from normalized mono audio.
   */
  [[nodiscard]] auto generateWaveFrames(const audio::NormalizedAudioBuffer& normalizedAudioBuffer,
                                        const WavePlaybackRequest& wavePlaybackRequest) const
      -> std::vector<RumbleFrame>;
};

} // namespace tatnez::rumble
