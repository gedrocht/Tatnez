# Contributing

## Philosophy

This repository is deliberately strict. The point is not to make contribution difficult; the point
is to protect a hardware-facing experiment with enough tooling that regressions, unsafe assumptions,
and undocumented behavior are caught early.

## Expectations

- Prefer self-descriptive names over abbreviated names.
- Add comments and documentation as if a motivated beginner will read the code next.
- Favor clear control flow over cleverness.
- Add tests whenever behavior changes.
- Document limitations honestly, especially where Windows or XInput impose hard constraints.

## Local development workflow

1. Open a shell that has a working C++ toolchain.
2. Install `pre-commit`.
3. Run `pre-commit run --all-files`.
4. Configure and build with `cmake --preset default`.
5. Run `ctest --preset default`.

## Pull requests

- Explain the problem being solved.
- Describe the operational and security risks.
- Note whether behavior changed on Windows only or in cross-platform core code.
- Update beginner docs and API docs when interfaces or workflows change.
