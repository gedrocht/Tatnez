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

## Quick start

Open a Developer PowerShell for Visual Studio or another shell that already has a C++ compiler on
`PATH`, then run:

```bash
./scripts/build.ps1
```

If you prefer the raw CMake commands:

```bash
cmake --preset default
cmake --build --preset default
ctest --preset default
```

For Unix-like shells:

```bash
./scripts/build.sh
```

Generate a synthetic tone without touching real hardware:

```bash
./build/default/tatnez_cli --dry-run tone --frequency-hz 40 --duration-seconds 2
```

List controller slots:

```bash
./build/default/tatnez_cli --list-controllers
```

Play a WAVE file through controller rumble on Windows:

```bash
./build/default/tatnez_cli --controller-index 0 wave --input-wave-file example.wav
```

## Documentation layers

- Beginner-friendly documentation site: see [docs/site/index.md](docs/site/index.md)
- API reference source for Doxygen: public headers under [include](include)
- Local serveable wiki using Gollum: see [wiki/README.md](wiki/README.md)

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
