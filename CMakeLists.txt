cmake_minimum_required(VERSION 3.27)

project(
  TatnezRumbleSpeaker
  VERSION 0.1.0
  DESCRIPTION "A heavily documented Xbox 360 controller rumble-to-audio experiment"
  LANGUAGES CXX
)

include(CTest)
include(FetchContent)

option(TATNEZ_BUILD_TESTS "Build the automated test suite." ON)
option(TATNEZ_ENABLE_WARNINGS_AS_ERRORS "Treat compiler warnings as build failures." ON)
option(TATNEZ_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer when supported." OFF)
option(TATNEZ_ENABLE_COVERAGE "Enable coverage instrumentation for GCC or Clang." OFF)
option(TATNEZ_ENABLE_CLANG_TIDY "Run clang-tidy during compilation when available." OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CompilerWarnings)
include(ClangTidy)
include(Sanitizers)
include(Coverage)

FetchContent_Declare(
  cli11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG v2.4.2
)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.14.1
)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
)

set(CLI11_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(CLI11_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CLI11_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)
set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(cli11 spdlog googletest)

add_library(
  tatnez_core
  src/audio/WaveFileReader.cpp
  src/controller/PlatformControllerRumbleBackendFactory.cpp
  src/logging/ApplicationLoggerFactory.cpp
  src/playback/StandardPlaybackTimer.cpp
  src/rumble/RumbleSignalGenerator.cpp
  src/rumble/RumbleSpeakerService.cpp
)

add_library(tatnez::core ALIAS tatnez_core)

target_include_directories(
  tatnez_core
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(
  tatnez_core
  PUBLIC
    spdlog::spdlog_header_only
)

if(WIN32)
  target_compile_definitions(tatnez_core PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)
  target_link_libraries(tatnez_core PRIVATE XInput)
endif()

tatnez_enable_project_warnings(tatnez_core ${TATNEZ_ENABLE_WARNINGS_AS_ERRORS})
tatnez_enable_clang_tidy(tatnez_core ${TATNEZ_ENABLE_CLANG_TIDY})
tatnez_enable_sanitizers(tatnez_core ${TATNEZ_ENABLE_SANITIZERS})
tatnez_enable_coverage(tatnez_core ${TATNEZ_ENABLE_COVERAGE})

add_executable(
  tatnez_cli
  src/main.cpp
)

add_executable(tatnez::cli ALIAS tatnez_cli)

target_link_libraries(tatnez_cli PRIVATE tatnez::core)
target_link_libraries(tatnez_cli PRIVATE CLI11::CLI11)

tatnez_enable_project_warnings(tatnez_cli ${TATNEZ_ENABLE_WARNINGS_AS_ERRORS})
tatnez_enable_clang_tidy(tatnez_cli ${TATNEZ_ENABLE_CLANG_TIDY})
tatnez_enable_sanitizers(tatnez_cli ${TATNEZ_ENABLE_SANITIZERS})
tatnez_enable_coverage(tatnez_cli ${TATNEZ_ENABLE_COVERAGE})

if(TATNEZ_BUILD_TESTS)
  enable_testing()

  add_executable(
    tatnez_tests
    tests/TestMain.cpp
    tests/RumbleSignalGeneratorTests.cpp
    tests/RumbleSpeakerServiceTests.cpp
    tests/WaveFileReaderTests.cpp
  )

  target_link_libraries(
    tatnez_tests
    PRIVATE
      GTest::gtest
      tatnez::core
  )

  tatnez_enable_project_warnings(tatnez_tests ${TATNEZ_ENABLE_WARNINGS_AS_ERRORS})
  tatnez_enable_sanitizers(tatnez_tests ${TATNEZ_ENABLE_SANITIZERS})
  tatnez_enable_coverage(tatnez_tests ${TATNEZ_ENABLE_COVERAGE})

  include(GoogleTest)
  gtest_discover_tests(tatnez_tests)
endif()

find_package(Doxygen QUIET)
if(Doxygen_FOUND)
  set(DOXYGEN_INPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src")
  set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doxygen")
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" @ONLY)

  add_custom_target(
    documentation_api_reference
    COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Generating Doxygen API reference"
    VERBATIM
  )
endif()
