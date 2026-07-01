# Repository Guidelines

## Project Structure & Module Organization
Source code lives in `src/`, with Linux networking in `src/platforms/linux/` and daemon code in `src/daemon/`. QML UI is under `src/ui/`, reusable helpers are in `src/utils/`, and command-line entry points are in `src/commands/`. Unit tests are in `tests/unit/` and `src/utils/tests/`. Flatpak packaging is in `linux/flatpak/`; Linux-specific desktop metadata lives under `linux/extra/`.

## Build, Test, and Development Commands
Use the existing CMake build for local work:

```bash
cmake -S . -B build -DBUILD_TESTS=ON -DBUILD_CRASHREPORTING=OFF
cmake --build build --target mozillavpn
ctest --test-dir build --output-on-failure
```

For Linux/Flatpak verification:

```bash
flatpak-builder --force-clean --user --install build-flatpak linux/flatpak/org.mozilla.vpn.yml
flatpak run org.mozilla.vpn --help
```

## Coding Style & Naming Conventions
Follow the repository’s existing C++ and QML style: ASCII files, 2-space indentation in QML/C++, `CamelCase` types, `snake_case` filenames for tests, and short, explicit helper names. Prefer `rg` for search and `apply_patch` for edits. Keep Linux networking and daemon flows intact unless a change is strictly required.

## Testing Guidelines
Add unit coverage beside the code being changed, usually in `tests/unit/` or `src/utils/tests/`. Name new tests `Test*` and keep the corresponding `.cpp` and `.h` pairs together. Run focused tests first, then the full target when practical. For WireGuard profile work, validate both parser behavior and persisted profile model behavior.

## Commit & Pull Request Guidelines
Recent history uses short, imperative summaries and sometimes bot-style prefixes such as `[Bot]`. Keep commit subjects concise and specific. PRs should describe the user-visible change, note any Linux/Flatpak impact, and include verification commands or screenshots for UI changes.

## Architecture Notes
This repository is currently being adapted into a standalone Linux WireGuard client. Preserve the existing Linux daemon, privilege, and Flatpak integration paths when possible; prefer small adapter-style changes over rewrites.
