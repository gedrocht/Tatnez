. (Join-Path $PSScriptRoot "common.ps1")

$prerequisiteStatusRows = @(
  [pscustomobject]@{
    Prerequisite = "CMake"
    Installed = Test-CommandAvailability -CommandName "cmake"
    WhyYouNeedIt = "Configures and builds the project"
    Fix = ".\scripts\install-prerequisites.ps1"
  }
  [pscustomobject]@{
    Prerequisite = "CTest"
    Installed = Test-CommandAvailability -CommandName "ctest"
    WhyYouNeedIt = "Runs the automated test suite"
    Fix = "Install CMake, then reopen the shell"
  }
  [pscustomobject]@{
    Prerequisite = "Python"
    Installed = Test-CommandAvailability -CommandName "python"
    WhyYouNeedIt = "Supports docs and helper tooling"
    Fix = ".\scripts\install-prerequisites.ps1"
  }
  [pscustomobject]@{
    Prerequisite = "Ninja"
    Installed = Test-CommandAvailability -CommandName "ninja"
    WhyYouNeedIt = "Builds the default CMake preset"
    Fix = ".\scripts\install-prerequisites.ps1"
  }
  [pscustomobject]@{
    Prerequisite = "Visual C++ compiler (cl.exe)"
    Installed = Test-CommandAvailability -CommandName "cl"
    WhyYouNeedIt = "Compiles the Windows application"
    Fix = "Install Visual Studio Build Tools and reopen Developer PowerShell"
  }
)

$optionalDocumentationStatusRows = @(
  [pscustomobject]@{
    OptionalTool = "MkDocs"
    Installed = Test-CommandAvailability -CommandName "mkdocs"
    WhyYouNeedIt = "Builds and serves the beginner documentation site"
    Fix = ".\scripts\install-prerequisites.ps1 -IncludeDocumentationTools"
  }
  [pscustomobject]@{
    OptionalTool = "Doxygen"
    Installed = Test-CommandAvailability -CommandName "doxygen"
    WhyYouNeedIt = "Builds API documentation"
    Fix = ".\scripts\install-prerequisites.ps1 -IncludeDocumentationTools"
  }
  [pscustomobject]@{
    OptionalTool = "Graphviz"
    Installed = Test-CommandAvailability -CommandName "dot"
    WhyYouNeedIt = "Improves documentation diagrams"
    Fix = ".\scripts\install-prerequisites.ps1 -IncludeDocumentationTools"
  }
  [pscustomobject]@{
    OptionalTool = "Docker Desktop"
    Installed = Test-CommandAvailability -CommandName "docker"
    WhyYouNeedIt = "Runs the local Gollum wiki"
    Fix = ".\scripts\install-prerequisites.ps1 -IncludeWikiTools"
  }
)

Write-Host "Required prerequisites"
$prerequisiteStatusRows | Format-Table -Auto

Write-Host ""
Write-Host "Optional documentation tools"
$optionalDocumentationStatusRows | Format-Table -Auto

$missingRequiredPrerequisites = @($prerequisiteStatusRows | Where-Object { -not $_.Installed })

if($missingRequiredPrerequisites.Count -gt 0) {
  Write-Host ""
  Write-Host "One or more required prerequisites are missing."
  Write-Host "Run .\scripts\install-prerequisites.ps1 and then reopen a Developer PowerShell for Visual Studio."
  exit 1
}

Write-Host ""
Write-Host "All required prerequisites look available in this shell."
Write-Host "You can now run .\scripts\build.ps1"
