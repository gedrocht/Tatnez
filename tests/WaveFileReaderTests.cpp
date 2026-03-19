#include "tatnez/audio/WaveFileReader.h"

#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

void appendUnsignedSixteenBitLittleEndian(std::vector<std::uint8_t>& fileBytes, std::uint16_t valueToAppend) {
  fileBytes.push_back(static_cast<std::uint8_t>(valueToAppend & 0x00FFU));
  fileBytes.push_back(static_cast<std::uint8_t>((valueToAppend >> 8U) & 0x00FFU));
}

void appendUnsignedThirtyTwoBitLittleEndian(std::vector<std::uint8_t>& fileBytes,
                                            std::uint32_t valueToAppend) {
  fileBytes.push_back(static_cast<std::uint8_t>(valueToAppend & 0x000000FFU));
  fileBytes.push_back(static_cast<std::uint8_t>((valueToAppend >> 8U) & 0x000000FFU));
  fileBytes.push_back(static_cast<std::uint8_t>((valueToAppend >> 16U) & 0x000000FFU));
  fileBytes.push_back(static_cast<std::uint8_t>((valueToAppend >> 24U) & 0x000000FFU));
}

auto createStereoSixteenBitWaveFileBytes() -> std::vector<std::uint8_t> {
  const std::vector<std::int16_t> interleavedSamples{32767, -32768, -32768, 32767};
  const auto dataChunkSizeInBytes =
      static_cast<std::uint32_t>(interleavedSamples.size() * sizeof(std::int16_t));

  std::vector<std::uint8_t> fileBytes;
  fileBytes.reserve(44U + dataChunkSizeInBytes);

  fileBytes.insert(fileBytes.end(), {'R', 'I', 'F', 'F'});
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, 36U + dataChunkSizeInBytes);
  fileBytes.insert(fileBytes.end(), {'W', 'A', 'V', 'E'});

  fileBytes.insert(fileBytes.end(), {'f', 'm', 't', ' '});
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, 16U);
  appendUnsignedSixteenBitLittleEndian(fileBytes, 1U);
  appendUnsignedSixteenBitLittleEndian(fileBytes, 2U);
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, 44100U);
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, 44100U * 2U * 2U);
  appendUnsignedSixteenBitLittleEndian(fileBytes, 4U);
  appendUnsignedSixteenBitLittleEndian(fileBytes, 16U);

  fileBytes.insert(fileBytes.end(), {'d', 'a', 't', 'a'});
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, dataChunkSizeInBytes);

  for (const auto interleavedSample : interleavedSamples) {
    appendUnsignedSixteenBitLittleEndian(fileBytes, static_cast<std::uint16_t>(interleavedSample));
  }

  return fileBytes;
}

auto createUniqueTemporaryPath() -> std::filesystem::path {
  const auto uniqueSuffix = std::chrono::steady_clock::now().time_since_epoch().count();
  return std::filesystem::temp_directory_path() /
         ("tatnez-wave-reader-test-" + std::to_string(uniqueSuffix) + ".wav");
}

void writeFileBytes(const std::filesystem::path& filePath, const std::vector<std::uint8_t>& fileBytes) {
  std::ofstream outputFileStream(filePath, std::ios::binary);
  ASSERT_TRUE(static_cast<bool>(outputFileStream));
  outputFileStream.write(reinterpret_cast<const char*>(fileBytes.data()),
                         static_cast<std::streamsize>(fileBytes.size()));
}

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileDownMixesStereoCorrectly) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, createStereoSixteenBitWaveFileBytes());

  const tatnez::audio::WaveFileReader waveFileReader;
  const auto normalizedAudioBuffer = waveFileReader.readNormalizedMonoSamplesFromFile(temporaryWaveFilePath);

  std::filesystem::remove(temporaryWaveFilePath);

  ASSERT_EQ(normalizedAudioBuffer.sampleRateInHertz, 44100U);
  ASSERT_EQ(normalizedAudioBuffer.monoSamples.size(), 2U);
  EXPECT_NEAR(normalizedAudioBuffer.monoSamples.at(0), -0.0000152587F, 0.001F);
  EXPECT_NEAR(normalizedAudioBuffer.monoSamples.at(1), -0.0000152587F, 0.001F);
}

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileRejectsInvalidHeader) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, std::vector<std::uint8_t>{'N', 'O', 'P', 'E'});

  const tatnez::audio::WaveFileReader waveFileReader;

  EXPECT_THROW(static_cast<void>(waveFileReader.readNormalizedMonoSamplesFromFile(temporaryWaveFilePath)),
               std::runtime_error);

  std::filesystem::remove(temporaryWaveFilePath);
}

} // namespace
