# WG Desktop

WG Desktop is a standalone Linux desktop client for local WireGuard profiles.
It imports WireGuard `.conf` files, stores profiles locally, and connects using
the existing Linux NetworkManager and Flatpak integration already present in
this codebase.

## Scope

This build targets Linux desktop and Flatpak. Other platforms remain in the
tree for source compatibility, but they are not part of the standalone WG
Desktop migration.

## Features

- Import a WireGuard `.conf` file.
- Paste WireGuard configuration text.
- Store multiple local profiles.
- Rename, remove, and select the active profile.
- Connect and disconnect through the existing Linux VPN backend.
- Work without accounts, registration, subscriptions, or hosted configuration
  services.

## Build

Use the Flatpak build for Linux verification:

```bash
flatpak-builder --force-clean --user --install build-flatpak linux/flatpak/org.mozilla.vpn.yml
flatpak run org.mozilla.vpn
```

For focused development tests:

```bash
flatpak-builder --run build-flatpak linux/flatpak/org.mozilla.vpn.yml \
  env PYTHONPATH="$PWD/flatpak-unit-build/pydeps" \
  ctest --test-dir flatpak-unit-build -R 'TestWireGuard(Config|ProfileModel)' \
  --output-on-failure
```

## Runtime Requirements

On a normal Linux desktop installation, NetworkManager is expected to be
available already. Server or minimal desktop environments may require installing
and enabling NetworkManager and WireGuard tools before VPN activation can work.

## Licensing

This project remains licensed under the MPL-2.0 license. Existing source file
license headers are preserved.
