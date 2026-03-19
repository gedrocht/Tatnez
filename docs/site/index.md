# Tatnez Rumble Speaker

Tatnez Rumble Speaker is an experiment in turning Xbox 360 controller rumble motors into very crude
haptic transducers. The project is intentionally educational: the code is written to be readable,
the names are explicit, and the documentation tries to explain both *what* the code does and *why*
it does it that way.

## The short version

The application can:

- generate a synthetic rumble pattern from a requested frequency
- read a WAVE file and convert its amplitude envelope into rumble frames
- play those frames through an Xbox 360 controller on Windows via XInput
- log what it is doing in detail
- verify the important logic on GitHub without needing physical controller hardware

The application cannot:

- make Windows recognize the controller as a native speaker or headphone device
- bypass XInput hardware limits
- produce high-fidelity sound like a real audio driver and speaker chain

## Why this project exists

There are two interesting engineering lessons here:

1. Hardware interfaces define hard limits. XInput gives us rumble control, not an audio device.
2. Even when hardware is unusual, we can still build it with professional engineering discipline:
   tests, static analysis, coverage, documentation, logs, and deployment automation.

## Recommended reading order for beginners

1. Read [Getting Started](getting-started.md).
2. Read [Beginner Walkthrough](beginner-walkthrough.md).
3. Read [Scripts Reference](scripts-reference.md).
4. Build the docs locally with `./scripts/build-docs.ps1` if you want a local copy.
5. Read [First Rumble Tone](tutorials/first-rumble-tone.md).
6. Read [Architecture](architecture.md).
7. Read [Limitations and Roadmap](limitations-and-roadmap.md).
8. Explore the API reference after the higher-level ideas make sense.

## Documentation layers

- This site is the beginner-first explanation layer.
- The Doxygen-generated API reference is the detailed code layer.
- The local Gollum wiki is the browseable notebook-style layer for tutorials and glossaries.
