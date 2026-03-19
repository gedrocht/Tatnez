param(
  [switch]$CleanFirst,
  [switch]$SkipApiReference
)

. (Join-Path $PSScriptRoot "common.ps1")

function Assert-MkDocsAvailability {
  Assert-CommandAvailability -CommandName "python" -FailureMessage "Python is not available on PATH. Run .\scripts\install-prerequisites.ps1 first."

  try {
    Invoke-ExternalCommand -ExecutablePath "python" -ExecutableArguments @("-c", "import mkdocs")
  }
  catch {
    throw "MkDocs is not installed. Run .\scripts\install-prerequisites.ps1 -IncludeDocumentationTools and try again."
  }
}

Assert-MkDocsAvailability

$documentationBuildDirectoryPath = Get-DocumentationBuildDirectoryPath
$siteDirectoryPath = Get-SiteDirectoryPath

if($CleanFirst) {
  if(Test-Path $documentationBuildDirectoryPath) {
    Write-Host "Removing existing documentation build directory:" $documentationBuildDirectoryPath
    Remove-Item -Recurse -Force $documentationBuildDirectoryPath
  }

  if(Test-Path $siteDirectoryPath) {
    Write-Host "Removing existing site directory:" $siteDirectoryPath
    Remove-Item -Recurse -Force $siteDirectoryPath
  }
}

Push-Location (Get-RepositoryRootPath)

try {
  $canBuildApiReference = (-not $SkipApiReference) -and (Test-CommandAvailability -CommandName "doxygen")

  if($canBuildApiReference) {
    Assert-CommandAvailability -CommandName "cmake" -FailureMessage "CMake is not available on PATH. Run .\scripts\install-prerequisites.ps1 first."

    # Configure a dedicated documentation build tree so the generated API reference does not mix
    # with the normal application build output.
    Invoke-ExternalCommand -ExecutablePath "cmake" -ExecutableArguments @("-S", ".", "-B", $documentationBuildDirectoryPath, "-G", "Ninja")
    Invoke-ExternalCommand -ExecutablePath "cmake" -ExecutableArguments @("--build", $documentationBuildDirectoryPath, "--target", "documentation_api_reference")
  }
  elseif(-not $SkipApiReference) {
    Write-Host "Doxygen is not currently available, so the API reference will be skipped."
    Write-Host "Run .\scripts\install-prerequisites.ps1 -IncludeDocumentationTools if you want the full documentation stack."
  }

  # Build the beginner-facing static site. Using `python -m mkdocs` keeps the script tied to the
  # same Python environment that installed the documentation dependencies.
  Invoke-ExternalCommand -ExecutablePath "python" -ExecutableArguments @("-m", "mkdocs", "build", "--strict")

  if($canBuildApiReference) {
    Invoke-ExternalCommand -ExecutablePath "python" -ExecutableArguments @("scripts/publish_api_docs.py", "build/docs/doxygen/html", "site/api/reference")
  }

  Write-Host ""
  Write-Host "Documentation build completed successfully."
  Write-Host "Open" (Join-Path $siteDirectoryPath "index.html") "to browse the generated site."
}
finally {
  Pop-Location
}
