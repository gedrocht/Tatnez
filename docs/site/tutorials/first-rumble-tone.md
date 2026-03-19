# First Rumble Tone

This tutorial walks through the simplest path in the application: generating a synthetic tone and
turning it into rumble frames.

## Step 1: Ask the application for a tone

```bash
./build/default/tatnez_cli --dry-run tone --frequency-hz 40 --duration-seconds 2
```

## Step 2: Understand what happens internally

1. The command line is parsed.
2. A `TonePlaybackRequest` object is filled in.
3. `RumbleSignalGenerator::generateToneFrames` computes one `RumbleFrame` per update slice.
4. In dry-run mode the frames are summarized instead of being sent to a controller.

## Step 3: Understand the important limitation

The requested frequency is a *target modulation rate*, not a promise of clean audio output. The
controller motors have inertia, friction, and firmware behavior that make them dramatically less
responsive than a real speaker cone.

## Step 4: Try a real playback

```bash
./build/default/tatnez_cli --controller-index 0 tone --frequency-hz 35 --duration-seconds 1
```

Keep the duration short at first. Rumble motors were not designed for continuous full-power
operation as loudspeakers.
