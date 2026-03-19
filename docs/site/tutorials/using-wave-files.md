# Using WAVE Files

The WAVE-file workflow is useful when you want to convert existing audio material into a rough
rumble envelope.

## Supported file types

The built-in parser currently supports:

- uncompressed PCM integer samples
- 32-bit IEEE floating-point samples

Stereo material is down-mixed to mono by averaging the channels.

## Example

```bash
./build/default/tatnez_cli --dry-run wave --input-wave-file example.wav
```

When you are satisfied with the generated frame summary:

```bash
./build/default/tatnez_cli --controller-index 0 wave --input-wave-file example.wav
```

## What conversion means here

The project does **not** stream full-band audio to the controller. Instead, it measures short
windows of the waveform, extracts simple amplitude statistics, and maps those values onto the two
rumble motors.

That means this workflow is closer to an intensity envelope than to actual speaker playback.
