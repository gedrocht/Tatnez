#include "tatnez/audio/NormalizedAudioBuffer.h"
#include "tatnez/rumble/RumbleSignalGenerator.h"

#include <gtest/gtest.h>

namespace {

TEST(RumbleSignalGeneratorTests, GenerateToneFramesReturnsExpectedFrameCountAndPositiveIntensity) {
  const tatnez::rumble::TonePlaybackRequest tonePlaybackRequest{40.0, 0.5, 200.0, 0.8};

  const auto generatedRumbleFrames =
      tatnez::rumble::RumbleSignalGenerator::generateToneFrames(tonePlaybackRequest);

  ASSERT_EQ(generatedRumbleFrames.size(), 100U);
  EXPECT_EQ(generatedRumbleFrames.front().frameDuration, std::chrono::microseconds(5000));
  EXPECT_GT(generatedRumbleFrames.at(1).largeMotorIntensity, 0U);
  EXPECT_GT(generatedRumbleFrames.at(1).smallMotorIntensity, 0U);
}

TEST(RumbleSignalGeneratorTests, GenerateToneFramesRejectsInvalidArguments) {
  tatnez::rumble::TonePlaybackRequest invalidTonePlaybackRequest;
  invalidTonePlaybackRequest.toneFrequencyInHertz = 0.0;

  EXPECT_THROW(static_cast<void>(
                   tatnez::rumble::RumbleSignalGenerator::generateToneFrames(invalidTonePlaybackRequest)),
               std::invalid_argument);
}

TEST(RumbleSignalGeneratorTests, GenerateToneFramesRejectsZeroDurationInSeconds) {
  tatnez::rumble::TonePlaybackRequest invalidTonePlaybackRequest;
  invalidTonePlaybackRequest.durationInSeconds = 0.0;

  EXPECT_THROW(static_cast<void>(
                   tatnez::rumble::RumbleSignalGenerator::generateToneFrames(invalidTonePlaybackRequest)),
               std::invalid_argument);
}

TEST(RumbleSignalGeneratorTests, GenerateToneFramesRejectsZeroUpdateRateInHertz) {
  tatnez::rumble::TonePlaybackRequest invalidTonePlaybackRequest;
  invalidTonePlaybackRequest.updateRateInHertz = 0.0;

  EXPECT_THROW(static_cast<void>(
                   tatnez::rumble::RumbleSignalGenerator::generateToneFrames(invalidTonePlaybackRequest)),
               std::invalid_argument);
}

TEST(RumbleSignalGeneratorTests, GenerateToneFramesRejectsOutOfRangeMasterIntensityScale) {
  tatnez::rumble::TonePlaybackRequest invalidTonePlaybackRequest;
  invalidTonePlaybackRequest.masterIntensityScale = 1.5;

  EXPECT_THROW(static_cast<void>(
                   tatnez::rumble::RumbleSignalGenerator::generateToneFrames(invalidTonePlaybackRequest)),
               std::invalid_argument);
}

TEST(RumbleSignalGeneratorTests, GenerateWaveFramesProducesStableMotorValues) {
  const tatnez::audio::NormalizedAudioBuffer normalizedAudioBuffer{4U, {0.0F, 1.0F, -1.0F, 0.0F}};
  const tatnez::rumble::WavePlaybackRequest wavePlaybackRequest{2.0, 1.0, 0.5, 1.0};

  const auto generatedRumbleFrames =
      tatnez::rumble::RumbleSignalGenerator::generateWaveFrames(normalizedAudioBuffer, wavePlaybackRequest);

  ASSERT_EQ(generatedRumbleFrames.size(), 2U);
  EXPECT_EQ(generatedRumbleFrames.front().frameDuration, std::chrono::microseconds(500000));
  EXPECT_GT(generatedRumbleFrames.front().largeMotorIntensity, 0U);
  EXPECT_GE(generatedRumbleFrames.front().smallMotorIntensity,
            generatedRumbleFrames.front().largeMotorIntensity);
}

TEST(RumbleSignalGeneratorTests, GenerateWaveFramesReturnsEmptyVectorForEmptyInput) {
  const tatnez::audio::NormalizedAudioBuffer normalizedAudioBuffer{44100U, {}};

  const auto generatedRumbleFrames = tatnez::rumble::RumbleSignalGenerator::generateWaveFrames(
      normalizedAudioBuffer, tatnez::rumble::WavePlaybackRequest{});

  EXPECT_TRUE(generatedRumbleFrames.empty());
}

TEST(RumbleSignalGeneratorTests, GenerateWaveFramesRejectsZeroSampleRate) {
  const tatnez::audio::NormalizedAudioBuffer normalizedAudioBuffer{0U, {0.1F, 0.2F}};

  EXPECT_THROW(static_cast<void>(tatnez::rumble::RumbleSignalGenerator::generateWaveFrames(
                   normalizedAudioBuffer, tatnez::rumble::WavePlaybackRequest{})),
               std::invalid_argument);
}

TEST(RumbleSignalGeneratorTests, GenerateWaveFramesRejectsZeroUpdateRateInHertz) {
  const tatnez::audio::NormalizedAudioBuffer normalizedAudioBuffer{44100U, {0.1F, 0.2F}};
  tatnez::rumble::WavePlaybackRequest invalidWavePlaybackRequest;
  invalidWavePlaybackRequest.updateRateInHertz = 0.0;

  EXPECT_THROW(static_cast<void>(tatnez::rumble::RumbleSignalGenerator::generateWaveFrames(
                   normalizedAudioBuffer, invalidWavePlaybackRequest)),
               std::invalid_argument);
}

TEST(RumbleSignalGeneratorTests, GenerateWaveFramesRejectsOutOfRangeMasterIntensityScale) {
  const tatnez::audio::NormalizedAudioBuffer normalizedAudioBuffer{44100U, {0.1F, 0.2F}};
  tatnez::rumble::WavePlaybackRequest invalidWavePlaybackRequest;
  invalidWavePlaybackRequest.masterIntensityScale = -0.1;

  EXPECT_THROW(static_cast<void>(tatnez::rumble::RumbleSignalGenerator::generateWaveFrames(
                   normalizedAudioBuffer, invalidWavePlaybackRequest)),
               std::invalid_argument);
}

TEST(RumbleSignalGeneratorTests, GenerateWaveFramesRejectsInvalidLargeMotorContributionScale) {
  const tatnez::audio::NormalizedAudioBuffer normalizedAudioBuffer{44100U, {0.1F, 0.2F}};
  tatnez::rumble::WavePlaybackRequest invalidWavePlaybackRequest;
  invalidWavePlaybackRequest.largeMotorContributionScale = 1.5;

  EXPECT_THROW(static_cast<void>(tatnez::rumble::RumbleSignalGenerator::generateWaveFrames(
                   normalizedAudioBuffer, invalidWavePlaybackRequest)),
               std::invalid_argument);
}

TEST(RumbleSignalGeneratorTests, GenerateWaveFramesRejectsInvalidContributionScale) {
  const tatnez::audio::NormalizedAudioBuffer normalizedAudioBuffer{44100U, {0.1F, 0.2F}};
  tatnez::rumble::WavePlaybackRequest invalidWavePlaybackRequest;
  invalidWavePlaybackRequest.smallMotorContributionScale = 1.5;

  EXPECT_THROW(static_cast<void>(tatnez::rumble::RumbleSignalGenerator::generateWaveFrames(
                   normalizedAudioBuffer, invalidWavePlaybackRequest)),
               std::invalid_argument);
}

TEST(RumbleSignalGeneratorTests, GenerateWaveFramesRoundsUpWhenTheLastFrameContainsPartialAudio) {
  const tatnez::audio::NormalizedAudioBuffer normalizedAudioBuffer{6U, {0.0F, 0.25F, 0.5F, 0.75F, 1.0F}};
  const tatnez::rumble::WavePlaybackRequest wavePlaybackRequest{4.0, 1.0, 0.5, 1.0};

  const auto generatedRumbleFrames =
      tatnez::rumble::RumbleSignalGenerator::generateWaveFrames(normalizedAudioBuffer, wavePlaybackRequest);

  ASSERT_EQ(generatedRumbleFrames.size(), 4U);
  EXPECT_EQ(generatedRumbleFrames.back().frameDuration, std::chrono::microseconds(250000));
  EXPECT_GT(generatedRumbleFrames.back().smallMotorIntensity, 0U);
}

} // namespace
