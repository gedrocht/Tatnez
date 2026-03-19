# Home

Welcome to the Tatnez Rumble Speaker wiki.

This wiki is the beginner-first walkthrough layer for the repository. If you are completely new to
XInput, haptics, or signal flow, start here before diving into the source code.

## Suggested reading order

1. Beginner-Glossary
2. Signal-Flow
3. Troubleshooting

## Core idea

The project measures or synthesizes a signal, reduces it into rumble-friendly control values, and
then sends those values to the two motors inside an Xbox 360 controller.

## Hard limit to remember

The controller does not become a real Windows speaker device. The wiki repeats this point because
beginners can lose a lot of time chasing a capability that XInput does not expose.
