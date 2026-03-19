#include "tatnez/rumble/RumbleSignalGenerator.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr auto twoPi = 6.28318530717958647692;

void validateStrictlyPositiveValue(double valueToValidate, const char* valueDescription) {
  if (valueToValidate <= 0.0) {
    throw std::invalid_argument(std::string(valueDescription) + " must be greater than zero.");
  }
}

void validateNormalizedScale(double scaleToValidate, const char* scaleDescription) {
  if (scaleToValidate < 0.0 || scaleToValidate > 1.0) {
    throw std::invalid_argument(std::string(scaleDescription) + " must be between 0.0 and 1.0.");
  }
}

auto convertNormalizedIntensityToMotorValue(double normalizedIntensity) -> std::uint16_t {
  const auto clampedIntensity = std::clamp(normalizedIntensity, 0.0, 1.0);
  const auto maximumMotorValue = static_cast<double>(std::numeric_limits<std::uint16_t>::max());
  return static_cast<std::uint16_t>(std::llround(clampedIntensity * maximumMotorValue));
}

auto computeFrameDuration(double updateRateInHertz) -> std::chrono::microseconds {
  return std::chrono::microseconds(
      static_cast<std::chrono::microseconds::rep>(std::llround(1'000'000.0 / updateRateInHertz)));
}

struct WindowStatistics final {
  double rootMeanSquareAmplitude{};
  double peakAbsoluteAmplitude{};
};

auto analyzeSampleWindow(const std::vector<float>& monoSamples, std::size_t windowStartIndex,
                         std::size_t windowEndIndexExclusive) -> WindowStatistics {
  if (windowStartIndex >= windowEndIndexExclusive || windowEndIndexExclusive > monoSamples.size()) {
    return WindowStatistics{};
  }

  double accumulatedSquaredAmplitude = 0.0;
  double peakAbsoluteAmplitude = 0.0;

  for (std::size_t sampleIndex = windowStartIndex; sampleIndex < windowEndIndexExclusive; ++sampleIndex) {
    const auto absoluteAmplitude = std::abs(static_cast<double>(monoSamples.at(sampleIndex)));
    accumulatedSquaredAmplitude += absoluteAmplitude * absoluteAmplitude;
    peakAbsoluteAmplitude = std::max(peakAbsoluteAmplitude, absoluteAmplitude);
  }

  const auto sampleCount = static_cast<double>(windowEndIndexExclusive - windowStartIndex);
  return WindowStatistics{std::sqrt(accumulatedSquaredAmplitude / sampleCount), peakAbsoluteAmplitude};
}

} // namespace

namespace tatnez::rumble {

auto RumbleSignalGenerator::generateToneFrames(const TonePlaybackRequest& tonePlaybackRequest) const
    -> std::vector<RumbleFrame> {
  validateStrictlyPositiveValue(tonePlaybackRequest.toneFrequencyInHertz, "toneFrequencyInHertz");
  validateStrictlyPositiveValue(tonePlaybackRequest.durationInSeconds, "durationInSeconds");
  validateStrictlyPositiveValue(tonePlaybackRequest.updateRateInHertz, "updateRateInHertz");
  validateNormalizedScale(tonePlaybackRequest.masterIntensityScale, "masterIntensityScale");

  const auto requestedFrameCount = static_cast<std::size_t>(
      std::ceil(tonePlaybackRequest.durationInSeconds * tonePlaybackRequest.updateRateInHertz));
  const auto frameDuration = computeFrameDuration(tonePlaybackRequest.updateRateInHertz);

  // The Xbox 360 controller has one larger low-frequency motor and one smaller higher-frequency
  // motor. These simple preference weights bias lower requested tones toward the large motor and
  // higher requested tones toward the small motor.
  const auto lowMotorPreference =
      std::clamp(1.0 - ((tonePlaybackRequest.toneFrequencyInHertz - 35.0) / 120.0), 0.15, 1.0);
  const auto highMotorPreference =
      std::clamp(((tonePlaybackRequest.toneFrequencyInHertz - 20.0) / 160.0), 0.25, 1.0);

  std::vector<RumbleFrame> rumbleFrames;
  rumbleFrames.reserve(requestedFrameCount);

  for (std::size_t frameIndex = 0; frameIndex < requestedFrameCount; ++frameIndex) {
    // The generator samples an idealized sine wave at the requested update rate. The absolute
    // value is used because the motors only know "how strong" to spin, not "which direction"
    // the original waveform was moving.
    const auto playbackTimeInSeconds =
        static_cast<double>(frameIndex) / tonePlaybackRequest.updateRateInHertz;
    const auto oscillationMagnitude =
        std::abs(std::sin(twoPi * tonePlaybackRequest.toneFrequencyInHertz * playbackTimeInSeconds));
    const auto scaledOscillationMagnitude = oscillationMagnitude * tonePlaybackRequest.masterIntensityScale;

    rumbleFrames.push_back(
        RumbleFrame{convertNormalizedIntensityToMotorValue(scaledOscillationMagnitude * lowMotorPreference),
                    convertNormalizedIntensityToMotorValue(scaledOscillationMagnitude * highMotorPreference),
                    frameDuration});
  }

  return rumbleFrames;
}

auto RumbleSignalGenerator::generateWaveFrames(const audio::NormalizedAudioBuffer& normalizedAudioBuffer,
                                               const WavePlaybackRequest& wavePlaybackRequest) const
    -> std::vector<RumbleFrame> {
  validateStrictlyPositiveValue(wavePlaybackRequest.updateRateInHertz, "updateRateInHertz");
  validateNormalizedScale(wavePlaybackRequest.masterIntensityScale, "masterIntensityScale");
  validateNormalizedScale(wavePlaybackRequest.largeMotorContributionScale, "largeMotorContributionScale");
  validateNormalizedScale(wavePlaybackRequest.smallMotorContributionScale, "smallMotorContributionScale");

  if (normalizedAudioBuffer.sampleRateInHertz == 0U) {
    throw std::invalid_argument("normalizedAudioBuffer.sampleRateInHertz must be greater than zero.");
  }

  if (normalizedAudioBuffer.monoSamples.empty()) {
    return {};
  }

  const auto samplesPerFrame =
      static_cast<double>(normalizedAudioBuffer.sampleRateInHertz) / wavePlaybackRequest.updateRateInHertz;
  const auto totalFrameCount = static_cast<std::size_t>(
      std::ceil(static_cast<double>(normalizedAudioBuffer.monoSamples.size()) / samplesPerFrame));
  const auto frameDuration = computeFrameDuration(wavePlaybackRequest.updateRateInHertz);

  std::vector<RumbleFrame> rumbleFrames;
  rumbleFrames.reserve(totalFrameCount);

  for (std::size_t frameIndex = 0; frameIndex < totalFrameCount; ++frameIndex) {
    // Each rumble frame represents a short window of the original audio. Instead of trying to
    // reproduce the full waveform, the code extracts simple energy measures that are much more
    // realistic for motors.
    const auto windowStartIndex =
        static_cast<std::size_t>(std::floor(static_cast<double>(frameIndex) * samplesPerFrame));
    const auto windowEndIndexExclusive =
        std::min(normalizedAudioBuffer.monoSamples.size(),
                 static_cast<std::size_t>(std::ceil(static_cast<double>(frameIndex + 1U) * samplesPerFrame)));

    const auto windowStatistics =
        analyzeSampleWindow(normalizedAudioBuffer.monoSamples, windowStartIndex, windowEndIndexExclusive);

    // The larger motor receives a smoother RMS-style value, while the smaller motor receives a
    // more reactive peak-oriented value. This is not psychoacoustically accurate, but it is easy
    // to understand and gives the two motors distinct jobs.
    const auto scaledLargeMotorIntensity = windowStatistics.rootMeanSquareAmplitude *
                                           wavePlaybackRequest.masterIntensityScale *
                                           wavePlaybackRequest.largeMotorContributionScale;
    const auto scaledSmallMotorIntensity = windowStatistics.peakAbsoluteAmplitude *
                                           wavePlaybackRequest.masterIntensityScale *
                                           wavePlaybackRequest.smallMotorContributionScale;

    rumbleFrames.push_back(RumbleFrame{convertNormalizedIntensityToMotorValue(scaledLargeMotorIntensity),
                                       convertNormalizedIntensityToMotorValue(scaledSmallMotorIntensity),
                                       frameDuration});
  }

  return rumbleFrames;
}

} // namespace tatnez::rumble
