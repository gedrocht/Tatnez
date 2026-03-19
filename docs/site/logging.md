# Logging

Hardware experiments are hard to debug without logs, so this project treats logging as a normal part
of the user experience rather than an afterthought.

## What gets logged

- application startup
- selected playback mode
- file-loading activity
- playback summaries
- per-frame trace logs
- failure and cleanup paths

## Where logs go

- console output for immediate feedback
- a persistent log file for later inspection

On Windows the default directory is:

```text
%LOCALAPPDATA%\TatnezRumbleSpeaker\logs
```

## Helpful tools for viewing logs

- Windows Terminal
- Visual Studio Code
- PowerShell with `Get-Content -Wait`
- any text editor that can auto-refresh

## Why both console and file logging exist

Console output is good for immediate feedback while you are experimenting. File logging is good for
comparing runs, attaching evidence to bug reports, and understanding failures that happen too fast
to read live.
