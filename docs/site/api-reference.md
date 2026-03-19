# API Reference

The generated API reference is built with Doxygen from the comments in the public headers and
selected implementation files.

## Local build

The simplest beginner command is:

```powershell
./scripts/build-docs.ps1
```

If Doxygen is installed, that script builds the API reference and copies it into the generated
static documentation site.

If you want only the raw lower-level commands, they are:

```powershell
cmake -S . -B build/docs -G Ninja
cmake --build build/docs --target documentation_api_reference
python scripts/publish_api_docs.py build/docs/doxygen/html site/api/reference
```

The raw generated HTML appears under `build/docs/doxygen/html`.

## On GitHub Pages

The documentation workflow copies the generated Doxygen output into the published site so the API
reference can live beside the beginner-focused documentation. After deployment, the entry page will
be available at `api/reference/index.html` under the published site root.
