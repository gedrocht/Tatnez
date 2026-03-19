param(
  [string]$PresetName = "default",
  [switch]$ListControllers,
  [switch]$LiveHardware,
  [int]$ControllerIndex = 0,
  [double]$FrequencyInHertz = 40.0,
  [double]$DurationInSeconds = 2.0,
  [string]$WaveFilePath = "",
  [string]$LogDirectory = ""
)

. (Join-Path $PSScriptRoot "common.ps1")

$executablePath = Get-ExecutablePath -PresetName $PresetName
if(-not (Test-Path $executablePath)) {
  throw "The application executable was not found at '$executablePath'. Run .\scripts\build.ps1 first."
}

$applicationArguments = @()

if($ListControllers) {
  $applicationArguments += "--list-controllers"
}
elseif($WaveFilePath -ne "") {
  if(-not (Test-Path $WaveFilePath)) {
    throw "The WAVE file '$WaveFilePath' does not exist."
  }

  if(-not $LiveHardware) {
    # Dry-run mode is the safest beginner default because it proves the conversion path works
    # without energizing any controller motors.
    $applicationArguments += "--dry-run"
  }

  $applicationArguments += @("--controller-index", $ControllerIndex.ToString())
  $applicationArguments += @("wave", "--input-wave-file", (Resolve-Path $WaveFilePath).Path)
}
else {
  if(-not $LiveHardware) {
    $applicationArguments += "--dry-run"
  }

  $applicationArguments += @("--controller-index", $ControllerIndex.ToString())
  $applicationArguments += @("tone", "--frequency-hz", $FrequencyInHertz.ToString(), "--duration-seconds", $DurationInSeconds.ToString())
}

if($LogDirectory -ne "") {
  $applicationArguments = @("--log-directory", $LogDirectory) + $applicationArguments
}

Invoke-ExternalCommand -ExecutablePath $executablePath -ExecutableArguments $applicationArguments
