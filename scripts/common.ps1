Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Invoke-ExternalCommand {
  param(
    [Parameter(Mandatory = $true)]
    [string]$ExecutablePath,

    [Parameter(Mandatory = $false)]
    [string[]]$ExecutableArguments = @()
  )

  # Show the exact command before it runs so a beginner can map each script to the lower-level
  # tools that actually do the work.
  Write-Host "Running:" $ExecutablePath ($ExecutableArguments -join " ")
  & $ExecutablePath @ExecutableArguments

  if($LASTEXITCODE -ne 0) {
    throw "The command failed with exit code ${LASTEXITCODE}: $ExecutablePath $($ExecutableArguments -join ' ')"
  }
}

function Test-CommandAvailability {
  param(
    [Parameter(Mandatory = $true)]
    [string]$CommandName
  )

  return $null -ne (Get-Command $CommandName -ErrorAction SilentlyContinue)
}

function Assert-CommandAvailability {
  param(
    [Parameter(Mandatory = $true)]
    [string]$CommandName,

    [Parameter(Mandatory = $true)]
    [string]$FailureMessage
  )

  if(-not (Test-CommandAvailability -CommandName $CommandName)) {
    throw $FailureMessage
  }
}

function Get-RepositoryRootPath {
  # Every PowerShell script in this folder can calculate the repository root the same way:
  # move up one directory from the script directory.
  return Split-Path -Parent $PSScriptRoot
}

function Get-BuildDirectoryPath {
  param(
    [Parameter(Mandatory = $true)]
    [string]$PresetName
  )

  return Join-Path (Get-RepositoryRootPath) ("build\" + $PresetName)
}

function Get-DocumentationBuildDirectoryPath {
  return Join-Path (Get-RepositoryRootPath) "build\docs"
}

function Get-SiteDirectoryPath {
  return Join-Path (Get-RepositoryRootPath) "site"
}

function Get-WikiComposeFilePath {
  return Join-Path (Get-RepositoryRootPath) "wiki\docker-compose.yml"
}

function Get-ExecutablePath {
  param(
    [Parameter(Mandatory = $true)]
    [string]$PresetName
  )

  $candidateExecutablePath = Join-Path (Get-BuildDirectoryPath -PresetName $PresetName) "tatnez_cli.exe"
  return $candidateExecutablePath
}
