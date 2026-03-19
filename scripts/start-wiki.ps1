param(
  [switch]$Detached
)

. (Join-Path $PSScriptRoot "common.ps1")

Assert-CommandAvailability -CommandName "docker" -FailureMessage "Docker is not available on PATH. Install Docker Desktop or run .\scripts\install-prerequisites.ps1 -IncludeWikiTools."

$wikiComposeArguments = @(
  "compose",
  "-f", (Get-WikiComposeFilePath),
  "up",
  "--build"
)

if($Detached) {
  $wikiComposeArguments += "-d"
}

Write-Host "The wiki will be available at http://localhost:4567 after the container starts."

Push-Location (Get-RepositoryRootPath)

try {
  Invoke-ExternalCommand -ExecutablePath "docker" -ExecutableArguments $wikiComposeArguments
}
finally {
  Pop-Location
}
