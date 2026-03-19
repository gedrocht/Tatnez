param(
  [string]$PresetName = "default",
  [switch]$ConfigureOnly,
  [switch]$SkipTests,
  [switch]$CleanFirst
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Invoke-ExternalCommand {
  param(
    [Parameter(Mandatory = $true)]
    [string]$ExecutablePath,

    [Parameter(Mandatory = $false)]
    [string[]]$ExecutableArguments = @()
  )

  # Echo the exact command so a beginner can see which lower-level tools the wrapper is using.
  Write-Host "Running:" $ExecutablePath ($ExecutableArguments -join " ")
  & $ExecutablePath @ExecutableArguments

  if($LASTEXITCODE -ne 0) {
    throw "The command failed with exit code $LASTEXITCODE: $ExecutablePath $($ExecutableArguments -join ' ')"
  }
}

function Test-CommandAvailability {
  param(
    [Parameter(Mandatory = $true)]
    [string]$CommandName
  )

  return $null -ne (Get-Command $CommandName -ErrorAction SilentlyContinue)
}

if(-not (Test-CommandAvailability -CommandName "cmake")) {
  throw "CMake is not available on PATH. Install CMake and reopen the shell."
}

# Resolve the repository root from the script location so the wrapper works no matter where the
# user launches it from.
$repositoryRootPath = Split-Path -Parent $PSScriptRoot

if($CleanFirst) {
  $buildDirectoryPath = Join-Path $repositoryRootPath ("build\" + $PresetName)
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
