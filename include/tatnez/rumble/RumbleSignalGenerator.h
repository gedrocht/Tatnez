#pragma once

#include "tatnez/audio/NormalizedAudioBuffer.h"
#include "tatnez/rumble/RumbleFrame.h"

#include <vector>

namespace tatnez::rumble {

inline constexpr auto defaultToneFrequencyInHertz = 40.0;
inline constexpr auto defaultToneDurationInSeconds = 2.0;
inline constexpr auto defaultRumbleUpdateRateInHertz = 200.0;
inline constexpr auto defaultMasterIntensityScale = 1.0;
inline constexpr auto defaultLargeMotorContributionScale = 0.65;

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
  double toneFrequencyInHertz{defaultToneFrequencyInHertz};

  /**
   * @brief Playback duration in seconds.
   */
  double durationInSeconds{defaultToneDurationInSeconds};

  /**
   * @brief How many rumble updates should be produced every second.
   */
  double updateRateInHertz{defaultRumbleUpdateRateInHertz};

  /**
   * @brief Master gain applied to both motors, expected in the range [0.0, 1.0].
   */
  double masterIntensityScale{defaultMasterIntensityScale};
};

/**
 * @brief Input configuration for WAVE-to-rumble conversion.
 */
struct WavePlaybackRequest final {
  /**
   * @brief How many rumble frames should be produced for every second of source audio.
   */
  double updateRateInHertz{defaultRumbleUpdateRateInHertz};

  /**
   * @brief Overall gain applied to the generated motor intensities.
   */
  double masterIntensityScale{defaultMasterIntensityScale};

  /**
   * @brief Scaling factor for the large low-frequency motor.
   */
  double largeMotorContributionScale{defaultLargeMotorContributionScale};

  /**
   * @brief Scaling factor for the small high-frequency motor.
   */
  double smallMotorContributionScale{defaultMasterIntensityScale};
};

/**
 * @brief Converts tones and normalized audio into controller rumble frames.
 */
class RumbleSignalGenerator final {
public:
  /**
   * @brief Generate rumble frames for a synthetic tone.
   */
  [[nodiscard]] static auto
  generateToneFrames(const TonePlaybackRequest& tonePlaybackRequest) -> std::vector<RumbleFrame>;

  /**
   * @brief Generate rumble frames from normalized mono audio.
   */
  [[nodiscard]] static auto
  generateWaveFrames(const audio::NormalizedAudioBuffer& normalizedAudioBuffer,
                     const WavePlaybackRequest& wavePlaybackRequest) -> std::vector<RumbleFrame>;
};

} // namespace tatnez::rumble
