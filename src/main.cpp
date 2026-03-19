#include "tatnez/audio/WaveFileReader.h"
#include "tatnez/controller/PlatformControllerRumbleBackendFactory.h"
#include "tatnez/logging/ApplicationLoggerFactory.h"
#include "tatnez/playback/StandardPlaybackTimer.h"
#include "tatnez/rumble/RumbleSignalGenerator.h"
#include "tatnez/rumble/RumbleSpeakerService.h"

#include <CLI/CLI.hpp>
#include <spdlog/logger.h>

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace {

void printControllerSummaries(const tatnez::controller::IControllerRumbleBackend& controllerRumbleBackend,
                              std::ostream& outputStream) {
  outputStream << "Controller connection summary:\n";

  for (std::uint32_t controllerIndex = 0; controllerIndex < 4U; ++controllerIndex) {
    const auto controllerConnectionSummary =
        controllerRumbleBackend.queryControllerConnectionSummary(controllerIndex);

    outputStream << "  Slot " << controllerConnectionSummary.controllerIndex << ": "
                 << (controllerConnectionSummary.isConnected ? "connected" : "not connected") << " via "
                 << controllerConnectionSummary.backendDisplayName << " ("
                 << controllerConnectionSummary.diagnosticMessage << ")\n";
  }
}

void printDryRunSummary(const std::vector<tatnez::rumble::RumbleFrame>& rumbleFrames,
                        std::ostream& outputStream) {
  outputStream << "Dry run summary:\n";
  outputStream << "  Generated frame count: " << rumbleFrames.size() << '\n';

  if (!rumbleFrames.empty()) {
    const auto& firstRumbleFrame = rumbleFrames.front();
    outputStream << "  First frame large motor intensity: " << firstRumbleFrame.largeMotorIntensity << '\n';
    outputStream << "  First frame small motor intensity: " << firstRumbleFrame.smallMotorIntensity << '\n';
    outputStream << "  First frame duration in microseconds: " << firstRumbleFrame.frameDuration.count()
                 << '\n';
  }
}

} // namespace

/**
 * @brief Command-line entry point for the Tatnez Rumble Speaker application.
 *
 * The executable offers a safe dry-run mode, a synthetic tone mode, and a WAVE file mode. The
 * code keeps the user-facing messaging explicit about the fact that controller rumble is not the
 * same thing as a native Windows audio endpoint.
 */
auto main(int argumentCount, char** argumentValues) -> int {
  try {
    CLI::App commandLineApplication{
        "Tatnez Rumble Speaker converts tones or WAVE files into Xbox 360 controller rumble.\n"
        "Important limitation: this does not make Windows identify the controller as a native audio "
        "output endpoint."};
    commandLineApplication.require_subcommand(0, 1);

    std::uint32_t controllerIndex = 0U;
    std::filesystem::path requestedLogDirectory;
    bool shouldListControllers = false;
    bool shouldPerformDryRunOnly = false;

    commandLineApplication
        .add_option("--controller-index", controllerIndex,
                    "The XInput controller slot to use, typically between 0 and 3.")
        ->check(CLI::Range(0U, 3U));
    commandLineApplication.add_option("--log-directory", requestedLogDirectory,
                                      "Optional directory that should receive the application log file.");
    commandLineApplication.add_flag(
        "--list-controllers", shouldListControllers,
        "List the four XInput controller slots and report whether each one is connected.");
    commandLineApplication.add_flag(
        "--dry-run", shouldPerformDryRunOnly,
        "Generate rumble frames and print a summary without sending anything to a controller.");

    tatnez::rumble::TonePlaybackRequest tonePlaybackRequest;
    auto* toneSubcommand = commandLineApplication.add_subcommand(
        "tone", "Generate a synthetic haptic tone directly in software.");
    toneSubcommand
        ->add_option("--frequency-hz", tonePlaybackRequest.toneFrequencyInHertz,
                     "Requested tone frequency in hertz.")
        ->required();
    toneSubcommand
        ->add_option("--duration-seconds", tonePlaybackRequest.durationInSeconds,
                     "Playback duration in seconds.")
        ->default_val(tonePlaybackRequest.durationInSeconds);
    toneSubcommand
        ->add_option("--update-rate-hz", tonePlaybackRequest.updateRateInHertz,
                     "How many rumble updates should be produced each second.")
        ->default_val(tonePlaybackRequest.updateRateInHertz);
    toneSubcommand
        ->add_option("--intensity-scale", tonePlaybackRequest.masterIntensityScale,
                     "Master intensity scale between 0.0 and 1.0.")
        ->default_val(tonePlaybackRequest.masterIntensityScale);

    std::filesystem::path waveFilePath;
    tatnez::rumble::WavePlaybackRequest wavePlaybackRequest;
    auto* waveSubcommand = commandLineApplication.add_subcommand(
        "wave", "Read a WAVE file and convert its amplitude envelope into controller rumble.");
    waveSubcommand
        ->add_option("--input-wave-file", waveFilePath, "The path to a .wav file that should be converted.")
        ->required()
        ->check(CLI::ExistingFile);
    waveSubcommand
        ->add_option("--update-rate-hz", wavePlaybackRequest.updateRateInHertz,
                     "How many rumble updates should be produced each second.")
        ->default_val(wavePlaybackRequest.updateRateInHertz);
    waveSubcommand
        ->add_option("--intensity-scale", wavePlaybackRequest.masterIntensityScale,
                     "Master intensity scale between 0.0 and 1.0.")
        ->default_val(wavePlaybackRequest.masterIntensityScale);

    CLI11_PARSE(commandLineApplication, argumentCount, argumentValues);

    if (argumentCount == 1) {
      std::cout << commandLineApplication.help() << '\n';
      return 0;
    }

    auto applicationLogger = tatnez::logging::ApplicationLoggerFactory::createLogger("tatnez-rumble-speaker",
                                                                                     requestedLogDirectory);
    applicationLogger->info("Tatnez Rumble Speaker is starting.");

    auto controllerRumbleBackend = std::shared_ptr<tatnez::controller::IControllerRumbleBackend>(
        tatnez::controller::createPlatformControllerRumbleBackend().release());

    if (shouldListControllers) {
      // Listing controller slots is intentionally available independently of playback. This gives
      // beginners a safe first diagnostic step before any rumble command is attempted.
      printControllerSummaries(*controllerRumbleBackend, std::cout);
    }

    std::vector<tatnez::rumble::RumbleFrame> rumbleFrames;
    tatnez::rumble::RumbleSignalGenerator rumbleSignalGenerator;

    if (*toneSubcommand) {
      applicationLogger->info("Generating a synthetic tone at {} hertz for {} seconds.",
                              tonePlaybackRequest.toneFrequencyInHertz,
                              tonePlaybackRequest.durationInSeconds);
      rumbleFrames = rumbleSignalGenerator.generateToneFrames(tonePlaybackRequest);
    } else if (*waveSubcommand) {
      applicationLogger->info("Reading WAVE file {} and converting it into rumble frames.",
                              waveFilePath.string());
      tatnez::audio::WaveFileReader waveFileReader;
      const auto normalizedAudioBuffer = waveFileReader.readNormalizedMonoSamplesFromFile(waveFilePath);
      rumbleFrames = rumbleSignalGenerator.generateWaveFrames(normalizedAudioBuffer, wavePlaybackRequest);
    } else if (!shouldListControllers) {
      std::cout << commandLineApplication.help() << '\n';
      return 0;
    }

    if (shouldPerformDryRunOnly) {
      // Dry-run mode exists so that signal generation can be explored on any machine, including
      // Linux CI runners and development environments without a connected controller.
      printDryRunSummary(rumbleFrames, std::cout);
      return 0;
    }

    if (!rumbleFrames.empty()) {
      auto playbackTimer = std::make_shared<tatnez::playback::StandardPlaybackTimer>();
      tatnez::rumble::RumbleSpeakerService rumbleSpeakerService(controllerRumbleBackend, applicationLogger,
                                                                playbackTimer);

      const auto playbackStatistics = rumbleSpeakerService.playFrames(controllerIndex, rumbleFrames);
      applicationLogger->info("Playback finished after {} frames and {} microseconds.",
                              playbackStatistics.playedFrameCount,
                              playbackStatistics.requestedPlaybackDuration.count());
    }

    return 0;
  } catch (const std::exception& thrownException) {
    std::cerr << "Fatal error: " << thrownException.what() << '\n';
    return 1;
  }
}
