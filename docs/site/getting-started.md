# Getting Started

## The shortest possible beginner path

Run these commands in order:

```powershell
./scripts/start-here.ps1
./scripts/install-prerequisites.ps1
```

If you also want local documentation tooling and the wiki:

```powershell
./scripts/install-prerequisites.ps1 -IncludeDocumentationTools -IncludeWikiTools
```

Close the shell, reopen **Developer PowerShell for Visual Studio**, then run:

```powershell
./scripts/check-prerequisites.ps1
./scripts/build.ps1
./scripts/run.ps1
./scripts/test.ps1
```

If you want to build and browse the documentation locally too:

```powershell
./scripts/build-docs.ps1
./scripts/serve-docs.ps1
./scripts/start-wiki.ps1
```

## What each command does

### `./scripts/start-here.ps1`

Prints the beginner workflow in the exact order we recommend.

### `./scripts/install-prerequisites.ps1`

Installs the Windows prerequisites through `winget`:

- CMake
- Python 3.12
- Ninja
- Visual Studio Build Tools 2022

When the Visual Studio Build Tools installer opens, select **Desktop development with C++**.

### `./scripts/check-prerequisites.ps1`

Confirms whether the current shell can see:

- `cmake`
- `ctest`
- `python`
- `ninja`
- `cl.exe`

### `./scripts/build.ps1`

Configures, builds, and tests the default preset.

### `./scripts/run.ps1`

Runs the application. By default it performs a safe dry run.

### `./scripts/test.ps1`

Runs the automated tests for the selected preset.

### `./scripts/build-docs.ps1`

Builds the static documentation site. If Doxygen is installed, it also builds the API reference.

### `./scripts/serve-docs.ps1`

Builds the documentation site and serves it locally at `http://localhost:8000`.

### `./scripts/start-wiki.ps1`

Starts the local Gollum wiki at `http://localhost:4567`.

## Safe first run

This is the safest beginner command because it does not vibrate real hardware:

```powershell
./scripts/run.ps1
```

## First hardware check

```powershell
./scripts/run.ps1 -ListControllers
```

## First live tone

```powershell
./scripts/run.ps1 -LiveHardware -ControllerIndex 0
```

## Where logs go

On Windows, logs default to:

```text
%LOCALAPPDATA%\TatnezRumbleSpeaker\logs
```

You can override the path with `-LogDirectory` when you use `./scripts/run.ps1`.
