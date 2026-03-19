param(
  [switch]$IncludeDocumentationTools,
  [switch]$IncludeWikiTools
)

. (Join-Path $PSScriptRoot "common.ps1")

function Install-WingetPackageIfNeeded {
  param(
    [Parameter(Mandatory = $true)]
    [string]$PackageIdentifier,

    [Parameter(Mandatory = $true)]
    [string]$FriendlyPackageName,

    [Parameter(Mandatory = $false)]
    [string[]]$AdditionalInstallArguments = @()
  )

  $existingInstallationOutput = winget list --id $PackageIdentifier --exact --accept-source-agreements 2>$null

  if($existingInstallationOutput -match [regex]::Escape($PackageIdentifier)) {
    Write-Host "$FriendlyPackageName is already installed."
    return
  }

  $installArguments = @(
    "install",
    "--id", $PackageIdentifier,
    "--exact",
    "--accept-source-agreements",
    "--accept-package-agreements"
  ) + $AdditionalInstallArguments

  Invoke-ExternalCommand -ExecutablePath "winget" -ExecutableArguments $installArguments
}

if(-not (Test-CommandAvailability -CommandName "winget")) {
  throw "winget is not available on PATH. Install App Installer from Microsoft and try again."
}

Write-Host "Installing the core Tatnez prerequisites for Windows development..."

Install-WingetPackageIfNeeded -PackageIdentifier "Kitware.CMake" -FriendlyPackageName "CMake"
Install-WingetPackageIfNeeded -PackageIdentifier "Python.Python.3.12" -FriendlyPackageName "Python 3.12"
Install-WingetPackageIfNeeded -PackageIdentifier "Ninja-build.Ninja" -FriendlyPackageName "Ninja"

if(-not (Test-CommandAvailability -CommandName "cl")) {
  Write-Host ""
  Write-Host "The Visual C++ compiler is not currently available in this shell."
  Write-Host "Launching the Visual Studio Build Tools installer interactively."
  Write-Host "When the installer opens, select the 'Desktop development with C++' workload."

  Install-WingetPackageIfNeeded `
    -PackageIdentifier "Microsoft.VisualStudio.2022.BuildTools" `
    -FriendlyPackageName "Visual Studio Build Tools 2022" `
    -AdditionalInstallArguments @("--interactive")
}

if($IncludeDocumentationTools) {
  Write-Host ""
  Write-Host "Installing optional documentation tools..."
  Install-WingetPackageIfNeeded -PackageIdentifier "DimitriVanHeesch.Doxygen" -FriendlyPackageName "Doxygen"
  Install-WingetPackageIfNeeded -PackageIdentifier "Graphviz.Graphviz" -FriendlyPackageName "Graphviz"

  Assert-CommandAvailability -CommandName "python" -FailureMessage "Python must be available before documentation dependencies can be installed."
  Invoke-ExternalCommand -ExecutablePath "python" -ExecutableArguments @("-m", "pip", "install", "-r", "requirements-docs.txt")
}

if($IncludeWikiTools) {
  Write-Host ""
  Write-Host "Installing optional wiki tools..."
  Install-WingetPackageIfNeeded -PackageIdentifier "Docker.DockerDesktop" -FriendlyPackageName "Docker Desktop"
}

Write-Host ""
Write-Host "Prerequisite installation finished."
Write-Host "Next steps:"
Write-Host "1. Close this shell."
Write-Host "2. Reopen a Developer PowerShell for Visual Studio."
Write-Host "3. Run .\scripts\check-prerequisites.ps1"
Write-Host "4. Run .\scripts\build.ps1"
