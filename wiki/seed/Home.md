# Home

Welcome to the Tatnez Rumble Speaker wiki.

This wiki is the beginner-first walkthrough layer for the repository. If you are completely new to
XInput, haptics, or signal flow, start here before diving into the source code.

## Suggested reading order

1. Beginner-Glossary
2. Signal-Flow
3. Troubleshooting
4. The script sequence in the repository root README

## Core idea

The project measures or synthesizes a signal, reduces it into rumble-friendly control values, and
then sends those values to the two motors inside an Xbox 360 controller.

## Fastest beginner script path

1. `.\scripts\install-prerequisites.ps1`
2. Reopen Developer PowerShell for Visual Studio
3. `.\scripts\check-prerequisites.ps1`
4. `.\scripts\build.ps1`
5. `.\scripts\run.ps1`
6. `.\scripts\test.ps1`

## Optional documentation path

1. `.\scripts\install-prerequisites.ps1 -IncludeDocumentationTools -IncludeWikiTools`
2. `.\scripts\build-docs.ps1`
3. `.\scripts\serve-docs.ps1`
4. `.\scripts\start-wiki.ps1`

## Hard limit to remember

The controller does not become a real Windows speaker device. The wiki repeats this point because
beginners can lose a lot of time chasing a capability that XInput does not expose.
