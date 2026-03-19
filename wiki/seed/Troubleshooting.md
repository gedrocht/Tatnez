# Troubleshooting

## The app says no controller is connected

- Make sure the controller is connected by USB or the correct wireless adapter.
- Run `.\scripts\run.ps1 -ListControllers`.
- Confirm that Windows itself can see the controller.

## The app runs but Windows does not show a new speaker device

That is expected. This repository does not create a virtual audio driver.

## Nothing appears in the log directory

- Check whether you passed `--log-directory`.
- Check the default `%LOCALAPPDATA%\TatnezRumbleSpeaker\logs` location.
- Make sure the process had permission to create files there.

## The build script says `cl.exe` is missing

- Reopen **Developer PowerShell for Visual Studio**.
- Run `.\scripts\check-prerequisites.ps1`.
- If it still fails, rerun `.\scripts\install-prerequisites.ps1`.

## The documentation scripts say MkDocs or Doxygen is missing

- Run `.\scripts\install-prerequisites.ps1 -IncludeDocumentationTools`.
- Reopen the shell after the install finishes.
- Run `.\scripts\check-prerequisites.ps1` again.

## The wiki script says Docker is missing

- Run `.\scripts\install-prerequisites.ps1 -IncludeWikiTools`.
- Start Docker Desktop once so the daemon is running.
- Rerun `.\scripts\start-wiki.ps1`.
