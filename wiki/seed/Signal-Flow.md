# Signal Flow

## End-to-end path

1. The user chooses `tone` or `wave`.
2. The application produces a sequence of `RumbleFrame` values.
3. The playback service sends those values to the controller backend.
4. The Windows XInput backend forwards the values to the controller motors.

## Why the application does not stream raw audio

The motors cannot realistically reproduce full-band audio. The project therefore uses reduced
control signals that fit the hardware better.
