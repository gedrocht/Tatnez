param(
  [string]$PresetName = "default",
  [switch]$ConfigureOnly,
  [switch]$SkipTests,
  [switch]$CleanFirst
)

. (Join-Path $PSScriptRoot "common.ps1")

if(-not (Test-CommandAvailability -CommandName "cmake")) {
  throw "CMake is not available on PATH. Install CMake and reopen the shell."
}

$repositoryRootPath = Get-RepositoryRootPath

if($CleanFirst) {
  $buildDirectoryPath = Get-BuildDirectoryPath -PresetName $PresetName
  if(Test-Path $buildDirectoryPath) {
    Write-Host "Removing existing build directory:" $buildDirectoryPath
    Remove-Item -Recurse -Force $buildDirectoryPath
  }
}

Push-Location $repositoryRootPath

try {
  # The configure step creates the build tree for the selected preset.
  Invoke-ExternalCommand -ExecutablePath "cmake" -ExecutableArguments @("--preset", $PresetName)

  if(-not $ConfigureOnly) {
    # The build step compiles every target that belongs to the preset.
    Invoke-ExternalCommand -ExecutablePath "cmake" -ExecutableArguments @("--build", "--preset", $PresetName)
  }

  if((-not $ConfigureOnly) -and (-not $SkipTests)) {
    Assert-CommandAvailability -CommandName "ctest" -FailureMessage "CTest is not available on PATH. Install CMake and reopen the shell."

    # Running tests here keeps the wrapper aligned with the repository's normal quality workflow.
    Invoke-ExternalCommand -ExecutablePath "ctest" -ExecutableArguments @("--preset", $PresetName)
  }

  Write-Host "Build workflow completed successfully for preset '$PresetName'."
}
catch {
  Write-Host ""
  Write-Host "Build workflow failed."
  Write-Host "If you are on Windows, make sure you are using a Developer PowerShell for Visual Studio"
  Write-Host "or another shell that already has a working C++ compiler toolchain on PATH."
  throw
}
finally {
  Pop-Location
}
