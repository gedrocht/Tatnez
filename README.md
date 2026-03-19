# Tatnez Rumble Speaker

Tatnez Rumble Speaker is a Windows-first C++ application that converts synthetic tones or WAVE file
amplitude envelopes into Xbox 360 controller rumble patterns through XInput.

## Important technical limitation

This project can make the motors behave like extremely crude haptic transducers, but it **cannot**
make Windows 10 enumerate an Xbox 360 controller as a native audio output endpoint through XInput
alone. A true audio device in Windows requires a separate virtual or hardware audio driver. This
repository therefore focuses on the part that is technically honest and achievable in user-space:
generating and playing rumble sequences.

## What the application does

- Generates synthetic rumble tones at a requested modulation frequency
- Reads uncompressed `.wav` files and converts their amplitude envelope into rumble frames
- Plays those frames through a connected Xbox 360 controller on Windows
- Logs every important step to both the console and a persistent log file
- Keeps the signal-generation code testable independently of physical hardware

## Complete beginner path

If you are new to C++, CMake, or Windows build tools, use these commands in this exact order.

### Step 1: See the roadmap

```powershell
./scripts/start-here.ps1
```

### Step 2: Install prerequisites

```powershell
./scripts/install-prerequisites.ps1
```

If you also want local documentation tools:

```powershell
./scripts/install-prerequisites.ps1 -IncludeDocumentationTools
```

If you also want to run the local wiki:

```powershell
./scripts/install-prerequisites.ps1 -IncludeWikiTools
```

If you want both documentation extras at once:

```powershell
./scripts/install-prerequisites.ps1 -IncludeDocumentationTools -IncludeWikiTools
```

### Step 3: Close the shell and reopen Developer PowerShell for Visual Studio

This matters because the Visual C++ compiler usually becomes available through the Developer
PowerShell environment.

### Step 4: Confirm the tools are available

```powershell
./scripts/check-prerequisites.ps1
```

### Step 5: Build the project

```powershell
./scripts/build.ps1
```

### Step 6: Run the safest possible demo

```powershell
./scripts/run.ps1
```

That default command performs a dry run, so it does not energize the controller motors.

### Step 7: Run the test suite

```powershell
./scripts/test.ps1
```

### Step 8: Build the documentation site

```powershell
./scripts/build-docs.ps1
```

### Step 9: Serve the documentation site locally

```powershell
./scripts/serve-docs.ps1
```

### Step 10: Start the local wiki

```powershell
./scripts/start-wiki.ps1
```

## Script map

- `./scripts/start-here.ps1`: prints the beginner workflow
- `./scripts/install-prerequisites.ps1`: installs the required Windows tools with `winget`
- `./scripts/check-prerequisites.ps1`: confirms the current shell can see the needed tools
- `./scripts/build.ps1`: configures, builds, and tests the default preset
- `./scripts/run.ps1`: runs the application in a safe dry-run mode by default
- `./scripts/test.ps1`: runs the test suite for a chosen preset
- `./scripts/build-docs.ps1`: builds the static documentation site and, when available, the API reference
- `./scripts/serve-docs.ps1`: serves the generated documentation site on `http://localhost:8000`
- `./scripts/start-wiki.ps1`: starts the local Gollum wiki on `http://localhost:4567`

## Common next commands

List controller slots:

```powershell
./scripts/run.ps1 -ListControllers
```

Run a live tone on controller slot `0`:

```powershell
./scripts/run.ps1 -LiveHardware -ControllerIndex 0
```

Run a WAVE file safely in dry-run mode:

```powershell
./scripts/run.ps1 -WaveFilePath .\example.wav
```

Run a WAVE file on real hardware:

```powershell
./scripts/run.ps1 -LiveHardware -ControllerIndex 0 -WaveFilePath .\example.wav
```

## Raw commands if you want them

If you prefer the lower-level commands behind the wrapper scripts:

```powershell
cmake --preset default
cmake --build --preset default
ctest --preset default
```

For Unix-like shells, the build wrapper is:

```bash
./scripts/build.sh
```

## Documentation layers

- Beginner-friendly documentation site: see [docs/site/index.md](docs/site/index.md)
- API reference source for Doxygen: public headers under [include](include)
- Local serveable wiki using Gollum: see [wiki/README.md](wiki/README.md)

## Documentation commands

Build the static documentation site:

```powershell
./scripts/build-docs.ps1
```

Serve the built site locally:

```powershell
./scripts/serve-docs.ps1
```

Start the local wiki:

```powershell
./scripts/start-wiki.ps1
```

## Logging

By default the application stores logs under `%LOCALAPPDATA%\TatnezRumbleSpeaker\logs` on Windows.
You can override that location with `--log-directory`.

## Quality gates

The repository is set up with strict automation for:

- formatting and repository hygiene
- unit and orchestration tests
- static analysis with `clang-tidy` and `cppcheck`
- sanitizer builds
- coverage thresholds
- secret scanning
- dependency review
- CodeQL
- GitHub Pages documentation publishing
- OpenSSF Scorecard checks
