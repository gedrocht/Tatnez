#!/usr/bin/env bash
set -euo pipefail

# Default to the same preset the README uses so the script is convenient out of the box.
preset_name="default"
configure_only="false"
skip_tests="false"
clean_first="false"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --preset)
      preset_name="$2"
      shift 2
      ;;
    --configure-only)
      configure_only="true"
      shift
      ;;
    --skip-tests)
      skip_tests="true"
      shift
      ;;
    --clean-first)
      clean_first="true"
      shift
      ;;
    *)
      echo "Unknown argument: $1" >&2
      exit 2
      ;;
  esac
done

# Resolve the repository root from the script location so the wrapper works from any current
# working directory.
script_directory_path="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repository_root_path="$(cd "${script_directory_path}/.." && pwd)"

if ! command -v cmake >/dev/null 2>&1; then
  echo "CMake is not available on PATH. Install CMake and reopen the shell." >&2
  exit 1
fi

if [[ "${clean_first}" == "true" ]]; then
  build_directory_path="${repository_root_path}/build/${preset_name}"
  if [[ -d "${build_directory_path}" ]]; then
    echo "Removing existing build directory: ${build_directory_path}"
    rm -rf "${build_directory_path}"
  fi
fi

cd "${repository_root_path}"

# The configure step creates the build tree for the selected preset.
echo "Running: cmake --preset ${preset_name}"
cmake --preset "${preset_name}"

if [[ "${configure_only}" != "true" ]]; then
  # The build step compiles the preset's targets.
  echo "Running: cmake --build --preset ${preset_name}"
  cmake --build --preset "${preset_name}"
fi

if [[ "${configure_only}" != "true" && "${skip_tests}" != "true" ]]; then
  # Running tests here keeps the wrapper aligned with the repository's normal quality workflow.
  echo "Running: ctest --preset ${preset_name}"
  ctest --preset "${preset_name}"
fi

echo "Build workflow completed successfully for preset '${preset_name}'."
