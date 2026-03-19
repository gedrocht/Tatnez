param(
  [string]$PresetName = "default",
  [switch]$BuildFirst,
  [switch]$CleanFirst
)

. (Join-Path $PSScriptRoot "common.ps1")

Assert-CommandAvailability -CommandName "ctest" -FailureMessage "CTest is not available on PATH. Install CMake and reopen the shell."

$buildDirectoryPath = Get-BuildDirectoryPath -PresetName $PresetName

if($BuildFirst -or (-not (Test-Path $buildDirectoryPath))) {
  # Reuse the normal build wrapper so beginners do not have to remember multiple configure/build
  # command sequences.
  & (Join-Path $PSScriptRoot "build.ps1") -PresetName $PresetName -SkipTests -CleanFirst:$CleanFirst

  if($LASTEXITCODE -ne 0) {
    throw "The build step failed, so the test step cannot continue."
  }
}

Push-Location (Get-RepositoryRootPath)

try {
  Invoke-ExternalCommand -ExecutablePath "ctest" -ExecutableArguments @("--preset", $PresetName)
}
finally {
  Pop-Location
}
