# WG Desktop

<p align="center">
  <img src="src/ui/resources/logo-generic.svg" alt="WG Desktop logo" width="128">
</p>

<p align="center">
  <strong>A clean, local-first WireGuard desktop client for Linux.</strong>
</p>

<p align="center">
  Import a WireGuard config, pick a profile, press Connect. No account, no
  subscription check, no hosted configuration service.
</p>

<p align="center">
  <a href="https://flathub.org/apps/io.github.srpvpn.wg-desktop">
    <img alt="Download on Flathub" src="https://flathub.org/api/badge?locale=en">
  </a>
</p>

<p align="center">
  <img alt="Linux" src="https://img.shields.io/badge/Linux-first-111111?style=flat-square&logo=linux&logoColor=white">
  <img alt="Flatpak" src="https://img.shields.io/badge/Flatpak-ready-4A90D9?style=flat-square&logo=flatpak&logoColor=white">
  <img alt="WireGuard" src="https://img.shields.io/badge/WireGuard-profiles-88171A?style=flat-square">
  <img alt="License" src="https://img.shields.io/badge/license-MPL--2.0-brightgreen?style=flat-square">
</p>

## Why WG Desktop?

WG Desktop is built for people who already have WireGuard configs and want a
friendly Linux desktop app around them. It keeps the hard parts boring:
NetworkManager integration, privileged daemon interaction, Flatpak packaging,
and the tunnel lifecycle are handled by the existing mature Linux stack in this
codebase.

Flatpak makes it especially practical: one install path, predictable runtime
dependencies, clean desktop integration, and the same app experience across
modern Linux distributions. That makes WG Desktop one of the most convenient
ways to use plain WireGuard profiles on Linux without turning VPN setup into a
system administration project.

## Features

- Import WireGuard `.conf` files.
- Paste WireGuard configuration text directly into the app.
- Store multiple local profiles.
- Rename, remove, and select the active profile.
- Connect and disconnect through the existing Linux VPN backend.
- Work offline after installation.
- Run without accounts, registration, subscriptions, or vendor-hosted config.

## Install

The intended distribution path is Flatpak/Flathub. Until the Flathub listing is
published, build and install the Flatpak locally:

```bash
flatpak-builder --force-clean --user --install build-flatpak linux/flatpak/io.github.srpvpn.wg-desktop.yml
flatpak run io.github.srpvpn.wg-desktop
```

On a normal Linux desktop, NetworkManager is expected to be available already.
Minimal server-style environments may need NetworkManager and WireGuard tooling
installed and enabled before VPN activation can work.

## Development

Use the Flatpak SDK for the most reliable Linux build, because it provides the
Qt/runtime versions expected by the project:

```bash
flatpak-builder --run build-flatpak linux/flatpak/io.github.srpvpn.wg-desktop.yml \
  cmake --build flatpak-unit-build --target mozillavpn -j2
```

Run focused WireGuard profile tests with:

```bash
flatpak-builder --run build-flatpak linux/flatpak/io.github.srpvpn.wg-desktop.yml \
  env PYTHONPATH="$PWD/flatpak-unit-build/pydeps" \
  ctest --test-dir flatpak-unit-build -R 'TestWireGuard(Config|ProfileModel)' \
  --output-on-failure
```

## Project Scope

WG Desktop currently targets Linux desktop and Flatpak. The priority is to keep
the Linux networking implementation stable while removing account/backend
dependencies and improving the local WireGuard profile workflow.

## License

WG Desktop is licensed under the Mozilla Public License 2.0. Existing source
file license headers and third-party license notices are preserved.
