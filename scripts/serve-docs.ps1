param(
  [int]$PortNumber = 8000,
  [switch]$CleanFirst,
  [switch]$SkipApiReference
)

. (Join-Path $PSScriptRoot "common.ps1")

# Build the static site first so the served copy includes the optional Doxygen API reference.
& (Join-Path $PSScriptRoot "build-docs.ps1") -CleanFirst:$CleanFirst -SkipApiReference:$SkipApiReference

$siteDirectoryPath = Get-SiteDirectoryPath

if(-not (Test-Path $siteDirectoryPath)) {
  throw "The site directory was not created. Run .\scripts\build-docs.ps1 and check for earlier errors."
}

Write-Host ""
Write-Host "Serving the generated documentation site at http://localhost:$PortNumber"
Write-Host "Press Ctrl+C when you want to stop the local server."

Push-Location $siteDirectoryPath

try {
  Invoke-ExternalCommand -ExecutablePath "python" -ExecutableArguments @("-m", "http.server", $PortNumber.ToString())
}
finally {
  Pop-Location
}
