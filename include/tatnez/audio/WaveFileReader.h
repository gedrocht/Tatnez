#pragma once

#include "tatnez/audio/NormalizedAudioBuffer.h"

#include <filesystem>

namespace tatnez::audio {

/**
 * @brief Reads uncompressed WAVE files and converts them into normalized mono audio data.
 *
 * The reader intentionally supports only a practical subset of the WAVE format because the
 * project is trying to stay easy to understand. Specifically, the implementation handles
 * PCM integer samples and IEEE floating-point samples. Unsupported encodings produce a clear
 * exception instead of silently corrupting the data.
 */
class WaveFileReader final {
public:
  /**
   * @brief Read a WAVE file from disk and return a normalized mono buffer.
   *
   * @param waveFilePath The path to the `.wav` file that should be parsed.
   * @return NormalizedAudioBuffer A normalized mono buffer ready for rumble synthesis.
   * @throws std::runtime_error Thrown when the file is missing, malformed, or unsupported.
   */
  [[nodiscard]] auto
  readNormalizedMonoSamplesFromFile(const std::filesystem::path& waveFilePath) const -> NormalizedAudioBuffer;
};

} // namespace tatnez::audio
