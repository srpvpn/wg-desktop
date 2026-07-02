# Contributing

WG Desktop is maintained as a standalone Linux WireGuard desktop client. Keep
changes small and preserve the existing Linux NetworkManager, daemon, privilege,
and Flatpak integration paths.

## Development Scope

The supported migration target is Linux + Flatpak. Other platform directories
may remain in the source tree, but changes should not assume they are currently
supported by the standalone client.

## Build

```bash
cmake -S . -B build -DBUILD_TESTS=ON -DBUILD_CRASHREPORTING=OFF
cmake --build build --target mozillavpn
ctest --test-dir build --output-on-failure
```

Use Flatpak for Linux runtime verification:

```bash
flatpak-builder --force-clean --user --install build-flatpak linux/flatpak/org.mozilla.vpn.yml
flatpak run org.mozilla.vpn
```

## Testing

Run focused tests near the code you changed first. For WireGuard profile work:

```bash
flatpak-builder --run build-flatpak linux/flatpak/org.mozilla.vpn.yml \
  env PYTHONPATH="$PWD/flatpak-unit-build/pydeps" \
  ctest --test-dir flatpak-unit-build -R 'TestWireGuard(Config|ProfileModel)' \
  --output-on-failure
```

## Style

- Follow existing C++ and QML style.
- Preserve MPL-2.0 license headers in existing files.
- Do not commit WireGuard private keys, real profile configs, credentials, or
  logs containing secrets.
- Prefer adapter-style changes over rewrites, especially in Linux networking
  code.

## Linux Networking Rule

Do not rewrite the Linux VPN/network implementation unless there is a verified
bug that cannot be fixed locally. The existing NetworkManager and Flatpak flow
is the integration contract for WG Desktop.
