#include "tatnez/audio/WaveFileReader.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct WaveFormatChunk final {
  std::uint16_t audioFormatTag{};
  std::uint16_t channelCount{};
  std::uint32_t sampleRateInHertz{};
  std::uint16_t bitsPerSample{};
};

struct ChunkLocation final {
  std::size_t chunkDataOffset{};
  std::uint32_t chunkDataSizeInBytes{};
};

auto readBinaryFileBytes(const std::filesystem::path& filePath) -> std::vector<std::uint8_t> {
  std::ifstream inputFileStream(filePath, std::ios::binary);
  if (!inputFileStream) {
    throw std::runtime_error("The WAVE file could not be opened: " + filePath.string());
  }

  std::vector<std::uint8_t> fileBytes(std::istreambuf_iterator<char>(inputFileStream),
                                      std::istreambuf_iterator<char>());
  return fileBytes;
}

void requireReadableRange(const std::vector<std::uint8_t>& fileBytes, std::size_t startingOffset,
                          std::size_t requiredByteCount, std::string_view errorContext) {
  if (startingOffset + requiredByteCount > fileBytes.size()) {
    throw std::runtime_error("The WAVE file ended unexpectedly while reading " + std::string(errorContext) +
                             ".");
  }
}

auto readLittleEndianUnsignedInteger16(const std::vector<std::uint8_t>& fileBytes,
                                       std::size_t byteOffset) -> std::uint16_t {
  requireReadableRange(fileBytes, byteOffset, 2U, "a 16-bit integer");

  return static_cast<std::uint16_t>(
      static_cast<std::uint16_t>(fileBytes.at(byteOffset)) |
      static_cast<std::uint16_t>(static_cast<std::uint16_t>(fileBytes.at(byteOffset + 1U)) << 8U));
}

auto readLittleEndianUnsignedInteger32(const std::vector<std::uint8_t>& fileBytes,
                                       std::size_t byteOffset) -> std::uint32_t {
  requireReadableRange(fileBytes, byteOffset, 4U, "a 32-bit integer");

  return static_cast<std::uint32_t>(
      static_cast<std::uint32_t>(fileBytes.at(byteOffset)) |
      static_cast<std::uint32_t>(static_cast<std::uint32_t>(fileBytes.at(byteOffset + 1U)) << 8U) |
      static_cast<std::uint32_t>(static_cast<std::uint32_t>(fileBytes.at(byteOffset + 2U)) << 16U) |
      static_cast<std::uint32_t>(static_cast<std::uint32_t>(fileBytes.at(byteOffset + 3U)) << 24U));
}

auto readChunkIdentifier(const std::vector<std::uint8_t>& fileBytes, std::size_t byteOffset) -> std::string {
  requireReadableRange(fileBytes, byteOffset, 4U, "a chunk identifier");
  return {fileBytes.begin() + static_cast<std::ptrdiff_t>(byteOffset),
          fileBytes.begin() + static_cast<std::ptrdiff_t>(byteOffset + 4U)};
}

auto findChunk(const std::vector<std::uint8_t>& fileBytes,
               std::string_view desiredChunkIdentifier) -> std::optional<ChunkLocation> {
  // The first 12 bytes of a RIFF/WAVE file are the container header. After that come the
  // variable-length chunks such as `fmt ` and `data`.
  std::size_t chunkHeaderOffset = 12U;

  while (chunkHeaderOffset + 8U <= fileBytes.size()) {
    const auto currentChunkIdentifier = readChunkIdentifier(fileBytes, chunkHeaderOffset);
    const auto currentChunkSizeInBytes = readLittleEndianUnsignedInteger32(fileBytes, chunkHeaderOffset + 4U);
    const auto currentChunkDataOffset = chunkHeaderOffset + 8U;

    requireReadableRange(fileBytes, currentChunkDataOffset, static_cast<std::size_t>(currentChunkSizeInBytes),
                         "chunk data");

    if (currentChunkIdentifier == desiredChunkIdentifier) {
      return ChunkLocation{currentChunkDataOffset, currentChunkSizeInBytes};
    }

    // RIFF chunks are padded to an even byte boundary. If the chunk payload has odd length,
    // one extra padding byte must be skipped before the next chunk begins.
    const auto paddedChunkSizeInBytes = static_cast<std::size_t>(currentChunkSizeInBytes) +
                                        static_cast<std::size_t>(currentChunkSizeInBytes % 2U);
    chunkHeaderOffset = currentChunkDataOffset + paddedChunkSizeInBytes;
  }

  return std::nullopt;
}

auto parseWaveFormatChunk(const std::vector<std::uint8_t>& fileBytes,
                          const ChunkLocation& formatChunkLocation) -> WaveFormatChunk {
  requireReadableRange(fileBytes, formatChunkLocation.chunkDataOffset, 16U, "the format chunk");

  return WaveFormatChunk{
      readLittleEndianUnsignedInteger16(fileBytes, formatChunkLocation.chunkDataOffset),
      readLittleEndianUnsignedInteger16(fileBytes, formatChunkLocation.chunkDataOffset + 2U),
      readLittleEndianUnsignedInteger32(fileBytes, formatChunkLocation.chunkDataOffset + 4U),
      readLittleEndianUnsignedInteger16(fileBytes, formatChunkLocation.chunkDataOffset + 14U)};
}

auto convertUnsignedEightBitSampleToNormalizedFloat(std::uint8_t rawSampleValue) -> float {
  return static_cast<float>((static_cast<double>(rawSampleValue) - 128.0) / 128.0);
}

auto convertSignedIntegerSampleToNormalizedFloat(std::int32_t rawSampleValue,
                                                 const WaveFormatChunk& waveFormatChunk) -> float {
  const auto magnitudeScale = std::pow(2.0, static_cast<double>(waveFormatChunk.bitsPerSample - 1U));
  return static_cast<float>(static_cast<double>(rawSampleValue) / magnitudeScale);
}

auto readSignedTwentyFourBitInteger(const std::vector<std::uint8_t>& fileBytes,
                                    std::size_t sampleOffset) -> std::int32_t {
  requireReadableRange(fileBytes, sampleOffset, 3U, "a 24-bit PCM sample");

  auto signedSampleValue = static_cast<std::int32_t>(
      static_cast<std::uint32_t>(fileBytes.at(sampleOffset)) |
      static_cast<std::uint32_t>(static_cast<std::uint32_t>(fileBytes.at(sampleOffset + 1U)) << 8U) |
      static_cast<std::uint32_t>(static_cast<std::uint32_t>(fileBytes.at(sampleOffset + 2U)) << 16U));

  if ((signedSampleValue & 0x00800000) != 0) {
    signedSampleValue |= static_cast<std::int32_t>(0xFF000000);
  }

  return signedSampleValue;
}

auto readNormalizedSample(const std::vector<std::uint8_t>& fileBytes, std::size_t sampleOffset,
                          const WaveFormatChunk& waveFormatChunk) -> float {
  if (waveFormatChunk.audioFormatTag == 1U) {
    switch (waveFormatChunk.bitsPerSample) {
    case 8U:
      return convertUnsignedEightBitSampleToNormalizedFloat(fileBytes.at(sampleOffset));
    case 16U:
      return convertSignedIntegerSampleToNormalizedFloat(
          static_cast<std::int16_t>(readLittleEndianUnsignedInteger16(fileBytes, sampleOffset)),
          waveFormatChunk);
    case 24U:
      return convertSignedIntegerSampleToNormalizedFloat(
          readSignedTwentyFourBitInteger(fileBytes, sampleOffset), waveFormatChunk);
    case 32U:
      return convertSignedIntegerSampleToNormalizedFloat(
          static_cast<std::int32_t>(readLittleEndianUnsignedInteger32(fileBytes, sampleOffset)),
          waveFormatChunk);
    default:
      break;
    }
  }

  if (waveFormatChunk.audioFormatTag == 3U && waveFormatChunk.bitsPerSample == 32U) {
    requireReadableRange(fileBytes, sampleOffset, 4U, "a 32-bit floating-point sample");

    const std::array<std::uint8_t, 4U> floatingPointSampleBytes{
        fileBytes.at(sampleOffset), fileBytes.at(sampleOffset + 1U), fileBytes.at(sampleOffset + 2U),
        fileBytes.at(sampleOffset + 3U)};
    float floatingPointSampleValue{};
    std::memcpy(&floatingPointSampleValue, floatingPointSampleBytes.data(), floatingPointSampleBytes.size());
    return floatingPointSampleValue;
  }

  throw std::runtime_error("Unsupported WAVE encoding. Supported encodings are PCM integer and IEEE float.");
}

auto clampNormalizedSample(float normalizedSampleValue) -> float {
  return std::clamp(normalizedSampleValue, -1.0F, 1.0F);
}

} // namespace

namespace tatnez::audio {

auto WaveFileReader::readNormalizedMonoSamplesFromFile(const std::filesystem::path& waveFilePath)
    -> NormalizedAudioBuffer {
  const auto fileBytes = readBinaryFileBytes(waveFilePath);
  if (fileBytes.size() < 12U) {
    throw std::runtime_error("The file is too small to be a valid WAVE file.");
  }

  if (readChunkIdentifier(fileBytes, 0U) != "RIFF" || readChunkIdentifier(fileBytes, 8U) != "WAVE") {
    throw std::runtime_error("The file is not a RIFF/WAVE file.");
  }

  const auto formatChunkLocation = findChunk(fileBytes, "fmt ");
  const auto dataChunkLocation = findChunk(fileBytes, "data");

  if (!formatChunkLocation.has_value()) {
    throw std::runtime_error("The WAVE file does not contain a required fmt chunk.");
  }

  if (!dataChunkLocation.has_value()) {
    throw std::runtime_error("The WAVE file does not contain a required data chunk.");
  }

  const auto waveFormatChunk = parseWaveFormatChunk(fileBytes, formatChunkLocation.value());

  if (waveFormatChunk.channelCount == 0U) {
    throw std::runtime_error("The WAVE file reports zero channels, which is invalid.");
  }

  const auto bytesPerSamplePerChannel = static_cast<std::size_t>(waveFormatChunk.bitsPerSample / 8U);
  if (bytesPerSamplePerChannel == 0U) {
    throw std::runtime_error("The WAVE file reports an invalid bit depth.");
  }

  const auto bytesPerInterleavedFrame =
      bytesPerSamplePerChannel * static_cast<std::size_t>(waveFormatChunk.channelCount);
  const auto totalSampleFrameCount =
      static_cast<std::size_t>(dataChunkLocation->chunkDataSizeInBytes) / bytesPerInterleavedFrame;

  std::vector<float> monoSamples;
  monoSamples.reserve(totalSampleFrameCount);

  for (std::size_t sampleFrameIndex = 0; sampleFrameIndex < totalSampleFrameCount; ++sampleFrameIndex) {
    // A "sample frame" means one moment in time across all channels. For stereo data that means
    // one left sample and one right sample that should be interpreted together.
    const auto sampleFrameOffset =
        dataChunkLocation->chunkDataOffset + (sampleFrameIndex * bytesPerInterleavedFrame);
    double accumulatedChannelValue = 0.0;

    for (std::uint16_t channelIndex = 0; channelIndex < waveFormatChunk.channelCount; ++channelIndex) {
      const auto channelOffset =
          sampleFrameOffset + (static_cast<std::size_t>(channelIndex) * bytesPerSamplePerChannel);
      const auto normalizedChannelValue = readNormalizedSample(fileBytes, channelOffset, waveFormatChunk);

      accumulatedChannelValue += static_cast<double>(clampNormalizedSample(normalizedChannelValue));
    }

    // The project keeps the downstream rumble conversion intentionally simple, so stereo is
    // reduced to mono by averaging the channels rather than preserving any spatial information.
    const auto averagedMonoValue =
        static_cast<float>(accumulatedChannelValue / static_cast<double>(waveFormatChunk.channelCount));
    monoSamples.push_back(clampNormalizedSample(averagedMonoValue));
  }

  return NormalizedAudioBuffer{waveFormatChunk.sampleRateInHertz, std::move(monoSamples)};
}

} // namespace tatnez::audio
