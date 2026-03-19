# Getting Started

## What you need

- Windows if you want to drive a real Xbox 360 controller through XInput
- A C++20 compiler
- CMake 3.27 or newer
- Ninja or another CMake generator
- Python if you want to build the documentation site locally

## Build the project

Open a shell that already has your compiler environment configured. On Windows that usually means a
Developer PowerShell for Visual Studio.

```bash
./scripts/build.ps1
```

The PowerShell wrapper configures, builds, and runs the default test suite. If you prefer the
lower-level commands, you can still run:

```bash
cmake --preset default
cmake --build --preset default
ctest --preset default
```

## First safe command

Start with a dry run so you can see how the signal generator behaves without touching real
hardware:

```bash
./build/default/tatnez_cli --dry-run tone --frequency-hz 40 --duration-seconds 2
```

## What the dry run tells you

- how many rumble frames were generated
- the first frame values
- whether the command-line parsing and signal generation path worked

## When you are ready for hardware

List controller slots:

```bash
./build/default/tatnez_cli --list-controllers
```

Then send a short tone to slot `0`:

```bash
./build/default/tatnez_cli --controller-index 0 tone --frequency-hz 35 --duration-seconds 1
```

## Where logs go

On Windows, logs default to:

```text
%LOCALAPPDATA%\TatnezRumbleSpeaker\logs
```

You can override the path with `--log-directory`.
