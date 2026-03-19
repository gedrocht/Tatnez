# Testing and Quality

This repository aims to be stricter than a typical hobby hardware project.

## Automated quality layers

- `clang-format` for style consistency
- `clang-tidy` for modern C++ and correctness checks
- `cppcheck` for additional static analysis
- GoogleTest for unit and orchestration tests
- sanitizer builds on Linux
- coverage thresholds with `gcovr`
- CodeQL for security-oriented code scanning
- Gitleaks for secret detection
- dependency review for pull requests

## Why there are fake controller backends in tests

GitHub Actions runners do not have your actual controller attached. The fake backend lets us verify:

- that playback stops motors after success
- that playback stops motors after failure
- that disconnected controllers are rejected cleanly
- that sleep timing is requested correctly

## Coverage philosophy

Coverage is treated as a guardrail, not a vanity metric. The threshold is designed to keep the core
logic well exercised without pretending that one number can prove the absence of bugs.
