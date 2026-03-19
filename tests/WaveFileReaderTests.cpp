#include "tatnez/audio/WaveFileReader.h"

#include <gtest/gtest.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace {

using ChunkIdentifier = std::array<char, 4>;

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

void writeUnsignedThirtyTwoBitLittleEndianAtOffset(std::vector<std::uint8_t>& fileBytes,
                                                   std::size_t byteOffset, std::uint32_t valueToWrite) {
  fileBytes.at(byteOffset) = static_cast<std::uint8_t>(valueToWrite & 0x000000FFU);
  fileBytes.at(byteOffset + 1U) = static_cast<std::uint8_t>((valueToWrite >> 8U) & 0x000000FFU);
  fileBytes.at(byteOffset + 2U) = static_cast<std::uint8_t>((valueToWrite >> 16U) & 0x000000FFU);
  fileBytes.at(byteOffset + 3U) = static_cast<std::uint8_t>((valueToWrite >> 24U) & 0x000000FFU);
}

void appendSignedTwentyFourBitLittleEndian(std::vector<std::uint8_t>& fileBytes, std::int32_t valueToAppend) {
  const auto rawValue = static_cast<std::uint32_t>(valueToAppend);
  fileBytes.push_back(static_cast<std::uint8_t>(rawValue & 0x000000FFU));
  fileBytes.push_back(static_cast<std::uint8_t>((rawValue >> 8U) & 0x000000FFU));
  fileBytes.push_back(static_cast<std::uint8_t>((rawValue >> 16U) & 0x000000FFU));
}

void appendSignedThirtyTwoBitLittleEndian(std::vector<std::uint8_t>& fileBytes, std::int32_t valueToAppend) {
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, static_cast<std::uint32_t>(valueToAppend));
}

void appendFloatLittleEndian(std::vector<std::uint8_t>& fileBytes, float valueToAppend) {
  std::uint32_t rawValue = 0U;
  std::memcpy(&rawValue, &valueToAppend, sizeof(float));
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, rawValue);
}

void appendChunk(std::vector<std::uint8_t>& fileBytes, const ChunkIdentifier& chunkIdentifier,
                 const std::vector<std::uint8_t>& chunkDataBytes) {
  fileBytes.insert(fileBytes.end(), chunkIdentifier.begin(), chunkIdentifier.end());
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, static_cast<std::uint32_t>(chunkDataBytes.size()));
  fileBytes.insert(fileBytes.end(), chunkDataBytes.begin(), chunkDataBytes.end());

  if ((chunkDataBytes.size() % 2U) != 0U) {
    fileBytes.push_back(0U);
  }
}

auto createFormatChunkBytes(std::uint16_t audioFormatTag, std::uint16_t channelCount,
                            std::uint32_t sampleRateInHertz,
                            std::uint16_t bitsPerSample) -> std::vector<std::uint8_t> {
  const auto blockAlignment = static_cast<std::uint16_t>((bitsPerSample / 8U) * channelCount);
  const auto bytesPerSecond = sampleRateInHertz * static_cast<std::uint32_t>(blockAlignment);

  std::vector<std::uint8_t> formatChunkBytes;
  appendUnsignedSixteenBitLittleEndian(formatChunkBytes, audioFormatTag);
  appendUnsignedSixteenBitLittleEndian(formatChunkBytes, channelCount);
  appendUnsignedThirtyTwoBitLittleEndian(formatChunkBytes, sampleRateInHertz);
  appendUnsignedThirtyTwoBitLittleEndian(formatChunkBytes, bytesPerSecond);
  appendUnsignedSixteenBitLittleEndian(formatChunkBytes, blockAlignment);
  appendUnsignedSixteenBitLittleEndian(formatChunkBytes, bitsPerSample);
  return formatChunkBytes;
}

auto createRiffWaveFileBytes(const std::vector<std::pair<ChunkIdentifier, std::vector<std::uint8_t>>>&
                                 chunkDefinitions) -> std::vector<std::uint8_t> {
  std::vector<std::uint8_t> fileBytes{'R', 'I', 'F', 'F', 0U, 0U, 0U, 0U, 'W', 'A', 'V', 'E'};

  for (const auto& [chunkIdentifier, chunkDataBytes] : chunkDefinitions) {
    appendChunk(fileBytes, chunkIdentifier, chunkDataBytes);
  }

  writeUnsignedThirtyTwoBitLittleEndianAtOffset(fileBytes, 4U,
                                                static_cast<std::uint32_t>(fileBytes.size() - 8U));
  return fileBytes;
}

auto createWaveFileBytes(std::uint16_t audioFormatTag, std::uint16_t channelCount,
                         std::uint32_t sampleRateInHertz, std::uint16_t bitsPerSample,
                         const std::vector<std::uint8_t>& dataChunkBytes) -> std::vector<std::uint8_t> {
  return createRiffWaveFileBytes(std::vector<std::pair<ChunkIdentifier, std::vector<std::uint8_t>>>{
      {ChunkIdentifier{'f', 'm', 't', ' '},
       createFormatChunkBytes(audioFormatTag, channelCount, sampleRateInHertz, bitsPerSample)},
      {ChunkIdentifier{'d', 'a', 't', 'a'}, dataChunkBytes}});
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

auto createMonoTwentyFourBitWaveFileBytes() -> std::vector<std::uint8_t> {
  std::vector<std::uint8_t> dataChunkBytes;
  appendSignedTwentyFourBitLittleEndian(dataChunkBytes, 8'388'607);
  appendSignedTwentyFourBitLittleEndian(dataChunkBytes, -8'388'608);
  return createWaveFileBytes(1U, 1U, 22050U, 24U, dataChunkBytes);
}

auto createMonoThirtyTwoBitIntegerWaveFileBytes() -> std::vector<std::uint8_t> {
  std::vector<std::uint8_t> dataChunkBytes;
  appendSignedThirtyTwoBitLittleEndian(dataChunkBytes, std::numeric_limits<std::int32_t>::max());
  appendSignedThirtyTwoBitLittleEndian(dataChunkBytes, std::numeric_limits<std::int32_t>::min());
  return createWaveFileBytes(1U, 1U, 48000U, 32U, dataChunkBytes);
}

auto createMonoFloatingPointWaveFileBytes() -> std::vector<std::uint8_t> {
  std::vector<std::uint8_t> dataChunkBytes;
  appendFloatLittleEndian(dataChunkBytes, 0.25F);
  appendFloatLittleEndian(dataChunkBytes, -0.5F);
  return createWaveFileBytes(3U, 1U, 16000U, 32U, dataChunkBytes);
}

auto createFloatingPointWaveFileBytesThatNeedClamping() -> std::vector<std::uint8_t> {
  std::vector<std::uint8_t> dataChunkBytes;
  appendFloatLittleEndian(dataChunkBytes, 1.5F);
  appendFloatLittleEndian(dataChunkBytes, -1.5F);
  return createWaveFileBytes(3U, 1U, 16000U, 32U, dataChunkBytes);
}

auto createWaveFileBytesWithoutFormatChunk() -> std::vector<std::uint8_t> {
  return createRiffWaveFileBytes(std::vector<std::pair<ChunkIdentifier, std::vector<std::uint8_t>>>{});
}

auto createWaveFileBytesWithoutDataChunk() -> std::vector<std::uint8_t> {
  return createRiffWaveFileBytes(std::vector<std::pair<ChunkIdentifier, std::vector<std::uint8_t>>>{
      {ChunkIdentifier{'f', 'm', 't', ' '}, createFormatChunkBytes(1U, 1U, 44100U, 16U)}});
}

auto createUnsupportedEncodingWaveFileBytes() -> std::vector<std::uint8_t> {
  return createWaveFileBytes(6U, 1U, 44100U, 16U, std::vector<std::uint8_t>{0U, 0U});
}

auto createUnsupportedPcmBitDepthWaveFileBytes() -> std::vector<std::uint8_t> {
  return createWaveFileBytes(1U, 1U, 44100U, 12U, std::vector<std::uint8_t>{0U, 0U});
}

auto createZeroChannelWaveFileBytes() -> std::vector<std::uint8_t> {
  return createWaveFileBytes(1U, 0U, 44100U, 16U, std::vector<std::uint8_t>{0U, 0U});
}

auto createZeroBitDepthWaveFileBytes() -> std::vector<std::uint8_t> {
  return createWaveFileBytes(1U, 1U, 44100U, 0U, std::vector<std::uint8_t>{});
}

auto createWaveFileBytesWithWrongIdentifiers() -> std::vector<std::uint8_t> {
  return std::vector<std::uint8_t>{'N', 'O', 'P', 'E', 0U, 0U, 0U, 0U, 'W', 'A', 'V', 'E'};
}

auto createWaveFileBytesWithShortFormatChunk() -> std::vector<std::uint8_t> {
  return createRiffWaveFileBytes(std::vector<std::pair<ChunkIdentifier, std::vector<std::uint8_t>>>{
      {ChunkIdentifier{'f', 'm', 't', ' '}, std::vector<std::uint8_t>{1U, 0U, 1U, 0U}},
      {ChunkIdentifier{'d', 'a', 't', 'a'}, std::vector<std::uint8_t>{0U, 0U}}});
}

auto createWaveFileBytesWithTruncatedChunkPayload() -> std::vector<std::uint8_t> {
  std::vector<std::uint8_t> fileBytes{'R', 'I', 'F', 'F'};
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, 13U);
  fileBytes.insert(fileBytes.end(), {'W', 'A', 'V', 'E'});
  fileBytes.insert(fileBytes.end(), {'f', 'm', 't', ' '});
  appendUnsignedThirtyTwoBitLittleEndian(fileBytes, 16U);
  fileBytes.push_back(1U);
  return fileBytes;
}

auto createWaveFileBytesWithOddSizedPaddedChunk() -> std::vector<std::uint8_t> {
  return createRiffWaveFileBytes(std::vector<std::pair<ChunkIdentifier, std::vector<std::uint8_t>>>{
      {ChunkIdentifier{'f', 'm', 't', ' '}, createFormatChunkBytes(1U, 1U, 8000U, 8U)},
      {ChunkIdentifier{'J', 'U', 'N', 'K'}, std::vector<std::uint8_t>{1U, 2U, 3U}},
      {ChunkIdentifier{'d', 'a', 't', 'a'}, std::vector<std::uint8_t>{0U, 255U}}});
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

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileRejectsMissingFiles) {
  const tatnez::audio::WaveFileReader waveFileReader;
  const auto missingWaveFilePath = createUniqueTemporaryPath();

  EXPECT_FALSE(std::filesystem::exists(missingWaveFilePath));
  EXPECT_THROW(static_cast<void>(waveFileReader.readNormalizedMonoSamplesFromFile(missingWaveFilePath)),
               std::runtime_error);
}

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileRejectsFilesThatAreTooSmallToBeWaveFiles) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, std::vector<std::uint8_t>{'N', 'O', 'P', 'E'});

  const tatnez::audio::WaveFileReader waveFileReader;

  EXPECT_THROW(static_cast<void>(waveFileReader.readNormalizedMonoSamplesFromFile(temporaryWaveFilePath)),
               std::runtime_error);

  std::filesystem::remove(temporaryWaveFilePath);
}

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileRejectsHeadersWithWrongIdentifiers) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, createWaveFileBytesWithWrongIdentifiers());

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

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileReadsSignedTwentyFourBitSamples) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, createMonoTwentyFourBitWaveFileBytes());

  const tatnez::audio::WaveFileReader waveFileReader;
  const auto normalizedAudioBuffer = waveFileReader.readNormalizedMonoSamplesFromFile(temporaryWaveFilePath);

  std::filesystem::remove(temporaryWaveFilePath);

  ASSERT_EQ(normalizedAudioBuffer.sampleRateInHertz, 22050U);
  ASSERT_EQ(normalizedAudioBuffer.monoSamples.size(), 2U);
  EXPECT_NEAR(normalizedAudioBuffer.monoSamples.at(0), 1.0F, 0.001F);
  EXPECT_NEAR(normalizedAudioBuffer.monoSamples.at(1), -1.0F, 0.001F);
}

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileReadsSignedThirtyTwoBitIntegerSamples) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, createMonoThirtyTwoBitIntegerWaveFileBytes());

  const tatnez::audio::WaveFileReader waveFileReader;
  const auto normalizedAudioBuffer = waveFileReader.readNormalizedMonoSamplesFromFile(temporaryWaveFilePath);

  std::filesystem::remove(temporaryWaveFilePath);

  ASSERT_EQ(normalizedAudioBuffer.sampleRateInHertz, 48000U);
  ASSERT_EQ(normalizedAudioBuffer.monoSamples.size(), 2U);
  EXPECT_NEAR(normalizedAudioBuffer.monoSamples.at(0), 1.0F, 0.001F);
  EXPECT_NEAR(normalizedAudioBuffer.monoSamples.at(1), -1.0F, 0.001F);
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

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileClampsFloatingPointSamplesToNormalizedRange) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, createFloatingPointWaveFileBytesThatNeedClamping());

  const tatnez::audio::WaveFileReader waveFileReader;
  const auto normalizedAudioBuffer = waveFileReader.readNormalizedMonoSamplesFromFile(temporaryWaveFilePath);

  std::filesystem::remove(temporaryWaveFilePath);

  ASSERT_EQ(normalizedAudioBuffer.monoSamples.size(), 2U);
  EXPECT_FLOAT_EQ(normalizedAudioBuffer.monoSamples.at(0), 1.0F);
  EXPECT_FLOAT_EQ(normalizedAudioBuffer.monoSamples.at(1), -1.0F);
}

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileRejectsMissingFormatChunk) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, createWaveFileBytesWithoutFormatChunk());

  const tatnez::audio::WaveFileReader waveFileReader;

  EXPECT_THROW(static_cast<void>(waveFileReader.readNormalizedMonoSamplesFromFile(temporaryWaveFilePath)),
               std::runtime_error);

  std::filesystem::remove(temporaryWaveFilePath);
}

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileRejectsFormatChunksThatAreTooSmall) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, createWaveFileBytesWithShortFormatChunk());

  const tatnez::audio::WaveFileReader waveFileReader;

  EXPECT_THROW(static_cast<void>(waveFileReader.readNormalizedMonoSamplesFromFile(temporaryWaveFilePath)),
               std::runtime_error);

  std::filesystem::remove(temporaryWaveFilePath);
}

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileRejectsTruncatedChunkPayloads) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, createWaveFileBytesWithTruncatedChunkPayload());

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

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileSupportsOddSizedPaddedChunksBeforeAudioData) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, createWaveFileBytesWithOddSizedPaddedChunk());

  const tatnez::audio::WaveFileReader waveFileReader;
  const auto normalizedAudioBuffer = waveFileReader.readNormalizedMonoSamplesFromFile(temporaryWaveFilePath);

  std::filesystem::remove(temporaryWaveFilePath);

  ASSERT_EQ(normalizedAudioBuffer.sampleRateInHertz, 8000U);
  ASSERT_EQ(normalizedAudioBuffer.monoSamples.size(), 2U);
  EXPECT_NEAR(normalizedAudioBuffer.monoSamples.at(0), -1.0F, 0.001F);
  EXPECT_NEAR(normalizedAudioBuffer.monoSamples.at(1), 0.9921875F, 0.001F);
}

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileRejectsUnsupportedEncoding) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, createUnsupportedEncodingWaveFileBytes());

  const tatnez::audio::WaveFileReader waveFileReader;

  EXPECT_THROW(static_cast<void>(waveFileReader.readNormalizedMonoSamplesFromFile(temporaryWaveFilePath)),
               std::runtime_error);

  std::filesystem::remove(temporaryWaveFilePath);
}

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileRejectsUnsupportedPcmBitDepths) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, createUnsupportedPcmBitDepthWaveFileBytes());

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

TEST(WaveFileReaderTests, ReadNormalizedMonoSamplesFromFileRejectsZeroBitDepthFiles) {
  const auto temporaryWaveFilePath = createUniqueTemporaryPath();
  writeFileBytes(temporaryWaveFilePath, createZeroBitDepthWaveFileBytes());

  const tatnez::audio::WaveFileReader waveFileReader;

  EXPECT_THROW(static_cast<void>(waveFileReader.readNormalizedMonoSamplesFromFile(temporaryWaveFilePath)),
               std::runtime_error);

  std::filesystem::remove(temporaryWaveFilePath);
}

} // namespace
