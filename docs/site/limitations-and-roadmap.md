# Limitations and Roadmap

## Current limitations

### No native Windows audio endpoint

This repository does not create a Windows speaker device. XInput exposes controller state and
rumble, not a WASAPI-compatible render endpoint. Creating a real audio endpoint would require a
separate virtual or hardware audio driver.

### Very limited frequency response

Controller motors are mechanical devices with inertia. They are not designed to reproduce the full
audio spectrum.

### Signal conversion is intentionally simple

The current WAVE conversion uses amplitude windows, not full spectral analysis or perceptual
modelling.

## Sensible future work

- optional spectral splitting between the two motors
- WASAPI capture integration for real-time analysis
- a separate research spike around a virtual audio driver
- richer telemetry export formats
