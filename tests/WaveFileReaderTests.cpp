#include "tatnez/audio/WaveFileReader.h"

#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <cstring>
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

void appendFloatLittleEndian(std::vector<std::uint8_t>& fileBytes, float valueToAppend) {
  std::uint32_t rawValue = 0U;
  std::memcpy(&rawValue, &valueToAppend, sizeof(float));
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, rawValue);
}

auto createWaveFileBytes(std::uint16_t audioFormatTag, std::uint16_t channelCount,
                         std::uint32_t sampleRateInHertz, std::uint16_t bitsPerSample,
                         const std::vector<std::uint8_t>& dataChunkBytes) -> std::vector<std::uint8_t> {
  const auto blockAlignment = static_cast<std::uint16_t>((bitsPerSample / 8U) * channelCount);
  const auto bytesPerSecond = sampleRateInHertz * static_cast<std::uint32_t>(blockAlignment);

  std::vector<std::uint8_t> fileBytes;
  fileBytes.reserve(44U + dataChunkBytes.size());

  fileBytes.insert(fileBytes.end(), {'R', 'I', 'F', 'F'});
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, 36U + static_cast<std::uint32_t>(dataChunkBytes.size()));
  fileBytes.insert(fileBytes.end(), {'W', 'A', 'V', 'E'});

  fileBytes.insert(fileBytes.end(), {'f', 'm', 't', ' '});
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, 16U);
  appendUnsignedSixteenBitLittleEndian(fileBytes, audioFormatTag);
  appendUnsignedSixteenBitLittleEndian(fileBytes, channelCount);
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, sampleRateInHertz);
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, bytesPerSecond);
  appendUnsignedSixteenBitLittleEndian(fileBytes, blockAlignment);
  appendUnsignedSixteenBitLittleEndian(fileBytes, bitsPerSample);

  fileBytes.insert(fileBytes.end(), {'d', 'a', 't', 'a'});
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, static_cast<std::uint32_t>(dataChunkBytes.size()));
  fileBytes.insert(fileBytes.end(), dataChunkBytes.begin(), dataChunkBytes.end());

  return fileBytes;
}

auto createStereoSixteenBitWaveFileBytes() -> std::vector<std::uint8_t> {
  const std::vector<std::int16_t> interleavedSamples{32767, -32768, -32768, 32767};
  std::vector<std::uint8_t> dataChunkBytes;
  for (const auto interleavedSample : interleavedSamples) {
    appendUnsignedSixteenBitLittleEndian(dataChunkBytes, static_cast<std::uint16_t>(interleavedSample));
  }

  return createWaveFileBytes(1U, 2U, 44100U, 16U, dataChunkBytes);
}

auto createMonoEightBitWaveFileBytes() -> std::vector<std::uint8_t> {
  return createWaveFileBytes(1U, 1U, 8000U, 8U, std::vector<std::uint8_t>{0U, 255U});
}

auto createMonoFloatingPointWaveFileBytes() -> std::vector<std::uint8_t> {
  std::vector<std::uint8_t> dataChunkBytes;
  appendFloatLittleEndian(dataChunkBytes, 0.25F);
  appendFloatLittleEndian(dataChunkBytes, -0.5F);
  return createWaveFileBytes(3U, 1U, 16000U, 32U, dataChunkBytes);
}

auto createWaveFileBytesWithoutFormatChunk() -> std::vector<std::uint8_t> {
  std::vector<std::uint8_t> fileBytes;
  fileBytes.insert(fileBytes.end(), {'R', 'I', 'F', 'F'});
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, 12U);
  fileBytes.insert(fileBytes.end(), {'W', 'A', 'V', 'E'});
  return fileBytes;
}

auto createWaveFileBytesWithoutDataChunk() -> std::vector<std::uint8_t> {
  std::vector<std::uint8_t> fileBytes;
  fileBytes.insert(fileBytes.end(), {'R', 'I', 'F', 'F'});
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, 28U);
  fileBytes.insert(fileBytes.end(), {'W', 'A', 'V', 'E'});
  fileBytes.insert(fileBytes.end(), {'f', 'm', 't', ' '});
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, 16U);
  appendUnsignedSixteenBitLittleEndian(fileBytes, 1U);
  appendUnsignedSixteenBitLittleEndian(fileBytes, 1U);
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, 44100U);
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, 44100U * 2U);
  appendUnsignedSixteenBitLittleEndian(fileBytes, 2U);
  appendUnsignedSixteenBitLittleEndian(fileBytes, 16U);
  return fileBytes;
}

auto createUnsupportedEncodingWaveFileBytes() -> std::vector<std::uint8_t> {
  return createWaveFileBytes(6U, 1U, 44100U, 16U, std::vector<std::uint8_t>{0U, 0U});
}

auto createZeroChannelWaveFileBytes() -> std::vector<std::uint8_t> {
  return createWaveFileBytes(1U, 0U, 44100U, 16U, std::vector<std::uint8_t>{0U, 0U});
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

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileReadsUnsignedEightBitSamples) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, createMonoEightBitWaveFileBytes());

  const tatnez::audio::WaveFileReader waveFileReader;
  const auto normalizedAudioBuffer = waveFileReader.readNormalizedMonoSamplesFromFile(temporaryWaveFilePath);

  std::filesystem::remove(temporaryWaveFilePath);

  ASSERT_EQ(normalizedAudioBuffer.sampleRateInHertz, 8000U);
  ASSERT_EQ(normalizedAudioBuffer.monoSamples.size(), 2U);
  EXPECT_NEAR(normalizedAudioBuffer.monoSamples.at(0), -1.0F, 0.001F);
  EXPECT_NEAR(normalizedAudioBuffer.monoSamples.at(1), 0.9921875F, 0.001F);
}

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileReadsFloatingPointSamples) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, createMonoFloatingPointWaveFileBytes());

  const tatnez::audio::WaveFileReader waveFileReader;
  const auto normalizedAudioBuffer = waveFileReader.readNormalizedMonoSamplesFromFile(temporaryWaveFilePath);

  std::filesystem::remove(temporaryWaveFilePath);

  ASSERT_EQ(normalizedAudioBuffer.sampleRateInHertz, 16000U);
  ASSERT_EQ(normalizedAudioBuffer.monoSamples.size(), 2U);
  EXPECT_NEAR(normalizedAudioBuffer.monoSamples.at(0), 0.25F, 0.001F);
  EXPECT_NEAR(normalizedAudioBuffer.monoSamples.at(1), -0.5F, 0.001F);
}

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileRejectsMissingFormatChunk) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, createWaveFileBytesWithoutFormatChunk());

  const tatnez::audio::WaveFileReader waveFileReader;

  EXPECT_THROW(static_cast<void>(waveFileReader.readNormalizedMonoSamplesFromFile(temporaryWaveFilePath)),
               std::runtime_error);

  std::filesystem::remove(temporaryWaveFilePath);
}

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileRejectsMissingDataChunk) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, createWaveFileBytesWithoutDataChunk());

  const tatnez::audio::WaveFileReader waveFileReader;

  EXPECT_THROW(static_cast<void>(waveFileReader.readNormalizedMonoSamplesFromFile(temporaryWaveFilePath)),
               std::runtime_error);

  std::filesystem::remove(temporaryWaveFilePath);
}

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileRejectsUnsupportedEncoding) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, createUnsupportedEncodingWaveFileBytes());

  const tatnez::audio::WaveFileReader waveFileReader;

  EXPECT_THROW(static_cast<void>(waveFileReader.readNormalizedMonoSamplesFromFile(temporaryWaveFilePath)),
               std::runtime_error);

  std::filesystem::remove(temporaryWaveFilePath);
}

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileRejectsZeroChannelFiles) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, createZeroChannelWaveFileBytes());

  const tatnez::audio::WaveFileReader waveFileReader;

  EXPECT_THROW(static_cast<void>(waveFileReader.readNormalizedMonoSamplesFromFile(temporaryWaveFilePath)),
               std::runtime_error);

  std::filesystem::remove(temporaryWaveFilePath);
}

} // namespace
