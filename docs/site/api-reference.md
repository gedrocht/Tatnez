# API Reference

The generated API reference is built with Doxygen from the comments in the public headers and
selected implementation files.

## Local build

If Doxygen is installed:

```bash
cmake --preset default
cmake --build build/default --target documentation_api_reference
```

The generated HTML will appear in the build directory under `doxygen/html`.

## On GitHub Pages

The documentation workflow copies the generated Doxygen output into the published site so the API
reference can live beside the beginner-focused documentation. After deployment, the entry page will
be available at `api/reference/index.html` under the published site root.
