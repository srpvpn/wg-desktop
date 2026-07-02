# Security Policy

WG Desktop is a local WireGuard profile client. It does not require an account,
subscription service, or hosted configuration API.

## Reporting Issues

Report security issues privately to the current project maintainer. Do not open
public issues for vulnerabilities that could expose private keys, profile data,
or tunnel traffic.

## Sensitive Data

Do not commit WireGuard private keys, real user profiles, generated tunnel
configs, logs containing private keys, or server access credentials.

## Runtime Assumptions

The supported Linux build uses Flatpak, NetworkManager, system D-Bus, and the
existing privileged networking path. Security fixes should preserve that model
unless there is a verified reason to change it.
