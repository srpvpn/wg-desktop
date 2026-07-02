# Installing WG Desktop

WG Desktop currently targets Linux desktop and Flatpak.

## Flatpak Build

Install Flatpak Builder:

```bash
flatpak install -y flathub org.flatpak.Builder
flatpak remote-add --if-not-exists --user flathub https://dl.flathub.org/repo/flathub.flatpakrepo
```

Build and install the application:

```bash
flatpak-builder --force-clean --user --install build-flatpak linux/flatpak/org.mozilla.vpn.yml
```

Run the application:

```bash
flatpak run org.mozilla.vpn
```

## Development Build

When building outside Flatpak, ensure the required Qt version and Linux
dependencies are available:

```bash
cmake -S . -B build -DBUILD_TESTS=ON -DBUILD_CRASHREPORTING=OFF
cmake --build build --target mozillavpn
ctest --test-dir build --output-on-failure
```

The Flatpak build remains the recommended verification path because it matches
the supported runtime environment.

## Linux Runtime Notes

WG Desktop uses the existing Linux NetworkManager backend. On Ubuntu Desktop
24.04 and similar desktop distributions, NetworkManager is normally installed
and active by default. Minimal server installations may need:

```bash
sudo apt-get install network-manager wireguard-tools
sudo systemctl enable --now NetworkManager
```

After installation, import a working WireGuard `.conf` profile and press
Connect.
