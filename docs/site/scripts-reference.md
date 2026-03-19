# Scripts Reference

This page exists so that a beginner can answer the question, "Which script should I run next?"

## `./scripts/start-here.ps1`

Prints the recommended beginner workflow.

## `./scripts/install-prerequisites.ps1`

Installs the Windows prerequisites with `winget`.

### Install example

```powershell
./scripts/install-prerequisites.ps1
```

### Optional docs example

```powershell
./scripts/install-prerequisites.ps1 -IncludeDocumentationTools
```

### Optional wiki example

```powershell
./scripts/install-prerequisites.ps1 -IncludeWikiTools
```

## `./scripts/check-prerequisites.ps1`

Checks whether the current shell can see the required tools.

### Check example

```powershell
./scripts/check-prerequisites.ps1
```

## `./scripts/build.ps1`

Configures, builds, and tests a CMake preset.

### Build examples

```powershell
./scripts/build.ps1
./scripts/build.ps1 -PresetName coverage
./scripts/build.ps1 -ConfigureOnly
./scripts/build.ps1 -SkipTests
./scripts/build.ps1 -CleanFirst
```

## `./scripts/run.ps1`

Runs the application. The default command is a safe dry run.

### Run examples

```powershell
./scripts/run.ps1
./scripts/run.ps1 -ListControllers
./scripts/run.ps1 -LiveHardware -ControllerIndex 0
./scripts/run.ps1 -WaveFilePath .\example.wav
./scripts/run.ps1 -LiveHardware -ControllerIndex 0 -WaveFilePath .\example.wav
```

## `./scripts/test.ps1`

Runs the automated tests.

### Test examples

```powershell
./scripts/test.ps1
./scripts/test.ps1 -BuildFirst
./scripts/test.ps1 -PresetName coverage -BuildFirst
```

## `./scripts/build-docs.ps1`

Builds the static documentation site and, when Doxygen is available, the API reference.

### Documentation build examples

```powershell
./scripts/build-docs.ps1
./scripts/build-docs.ps1 -CleanFirst
./scripts/build-docs.ps1 -SkipApiReference
```

## `./scripts/serve-docs.ps1`

Builds the documentation site and serves it locally.

### Documentation serve examples

```powershell
./scripts/serve-docs.ps1
./scripts/serve-docs.ps1 -PortNumber 9000
./scripts/serve-docs.ps1 -SkipApiReference
```

## `./scripts/start-wiki.ps1`

Starts the local Gollum wiki with Docker.

### Wiki examples

```powershell
./scripts/start-wiki.ps1
./scripts/start-wiki.ps1 -Detached
```
