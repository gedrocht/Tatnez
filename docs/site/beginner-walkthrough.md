# Beginner Walkthrough

This page is written for somebody who wants a literal recipe instead of a conceptual overview.

## Step 1: Open PowerShell in the repository

Make sure you are in the project folder before you run any commands.

## Step 2: Print the roadmap

```powershell
./scripts/start-here.ps1
```

## Step 3: Install the prerequisites

```powershell
./scripts/install-prerequisites.ps1
```

If you want the optional documentation toolchain too:

```powershell
./scripts/install-prerequisites.ps1 -IncludeDocumentationTools
```

If you want the local wiki too:

```powershell
./scripts/install-prerequisites.ps1 -IncludeWikiTools
```

If you want both optional layers in one step:

```powershell
./scripts/install-prerequisites.ps1 -IncludeDocumentationTools -IncludeWikiTools
```

## Step 4: Reopen Developer PowerShell for Visual Studio

This step is easy to skip, but it matters. The Microsoft C++ compiler usually becomes available
through the Developer PowerShell environment.

## Step 5: Check that the tools are visible

```powershell
./scripts/check-prerequisites.ps1
```

If the script says `cl.exe` is missing, you are probably not in Developer PowerShell yet.

## Step 6: Build the project

```powershell
./scripts/build.ps1
```

## Step 7: Run the safest possible demo

```powershell
./scripts/run.ps1
```

That command is intentionally safe by default. It performs a dry run instead of powering the
controller motors.

## Step 8: Run the test suite yourself

```powershell
./scripts/test.ps1
```

## Step 9: Build the beginner documentation site

```powershell
./scripts/build-docs.ps1
```

## Step 10: Serve the documentation site locally

```powershell
./scripts/serve-docs.ps1
```

That script serves the built site at `http://localhost:8000`.

## Step 11: Start the local wiki

```powershell
./scripts/start-wiki.ps1
```

That script starts the wiki at `http://localhost:4567`.

## Step 12: Try a real controller

List controller slots first:

```powershell
./scripts/run.ps1 -ListControllers
```

Then try a short live tone:

```powershell
./scripts/run.ps1 -LiveHardware -ControllerIndex 0
```
