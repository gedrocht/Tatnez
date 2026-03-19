# Troubleshooting

## The app says no controller is connected

- Make sure the controller is connected by USB or the correct wireless adapter.
- Run `--list-controllers`.
- Confirm that Windows itself can see the controller.

## The app runs but Windows does not show a new speaker device

That is expected. This repository does not create a virtual audio driver.

## Nothing appears in the log directory

- Check whether you passed `--log-directory`.
- Check the default `%LOCALAPPDATA%\TatnezRumbleSpeaker\logs` location.
- Make sure the process had permission to create files there.
