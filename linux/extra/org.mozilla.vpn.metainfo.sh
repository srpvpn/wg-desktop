#!/bin/sh
# Generate a localized metainfo file
SRCDIR=$(cd $(dirname $0)/../.. && pwd)
XLIFFTOOL=${SRCDIR}/scripts/utils/xlifftool.py

DATADIR="/usr/local/share"
GITREF=""
OUTPUT="-"

helpFunction() {
  echo "Usage: $0 [OPTIONS]"
  echo ""
  echo "Build options:"
  echo "  -o, --output FILE      Output metainfo to FILE (default: stdout)"
  echo "  -d, --datadir DIR      Data directory for app (default: ${DATADIR})"
  echo "  -g, --gitref REF       Github ref external URLs"
  echo "  -h, --help             Print this message and exit"
  echo ""
}

while [ $# -gt 0 ]; do
  key="$1"

  case $key in
  -o | --output)
    OUTPUT="$2"
    shift
    shift
    ;;
  -d | --datadir)
    DATADIR="$2"
    shift
    shift
    ;;
  -g | --gitref)
    GITREF="$2"
    shift
    shift
    ;;
  -h | --help)
    helpFunction >&2
    exit 0
    ;;
  *)
    helpFunction >&2
    exit 1
    ;;
  esac
done

# Determine a sensible gitref if none was provided.
if [ -z "${GITREF}" ]; then
  if [ -d "${SRCDIR}/.git" ]; then
    GITREF="$(git -C ${SRCDIR} rev-parse  HEAD)"
  else
    GITREF="refs/heads/main"
  fi
fi

if [ "${OUTPUT}" != "-" ]; then
  exec 1>"${OUTPUT}"
fi

cat << EOF
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>org.mozilla.vpn</id>

  <name>WG Desktop</name>
  <summary>A standalone WireGuard desktop client</summary>

EOF

# Do not emit legacy localized names. The existing translation catalog is
# still largely service-specific, so the standalone build keeps packaging
# metadata intentionally unlocalized until WG Desktop strings are translated.

cat << EOF

  <metadata_license>CC0-1.0</metadata_license>
  <project_license>MPL-2.0</project_license>
  <content_rating type="oars-1.1" />
  <categories>
    <category>Network</category>
    <category>Security</category>
  </categories>

  <developer id="org.wgdesktop">
    <name>WG Desktop</name>
  </developer>

  <icon type="stock">org.mozilla.vpn</icon>
  <branding>
    <color type="primary" scheme_preference="light">#f9f9fa</color>
    <color type="primary" scheme_preference="dark">#42414d</color>
  </branding>

  <url type="homepage">https://www.wireguard.com</url>

  <supports>
    <control>pointing</control>
    <control>keyboard</control>
    <control>touch</control>
  </supports>
  <requires>
    <display_length compare="gt">640</display_length>
  </requires>

  <description>
    <p>WG Desktop imports local WireGuard configuration files and connects through the existing Linux desktop networking integration.</p>
    <p><em>Local profile storage</em></p>
    <p>Profiles are stored on the device and no account or hosted service is required.</p>
    <p><em>Linux desktop integration</em></p>
    <p>The application preserves the existing Linux daemon, NetworkManager, Flatpak, and tunnel lifecycle behavior.</p>
    <ul>
      <li>Import .conf files</li>
      <li>Paste WireGuard configuration text</li>
      <li>Manage multiple local profiles</li>
      <li>Connect and disconnect using the existing Linux VPN stack</li>
    </ul>
  </description>
  <launchable type="desktop-id">org.mozilla.vpn.desktop</launchable>
</component>
EOF
