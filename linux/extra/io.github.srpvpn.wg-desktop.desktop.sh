#!/bin/sh
# Generate a localized desktop entry file
SRCDIR=$(cd $(dirname $0)/../.. && pwd)
XLIFFTOOL=${SRCDIR}/scripts/utils/xlifftool.py

helpFunction() {
  echo "Usage: $0 [OPTIONS]"
  echo ""
  echo "Build options:"
  echo "  -o, --output FILE      Output desktop entry to FILE (default: stdout)"
  echo "  -b, --bindir DIR       Binary directory application for app"
  echo "  -h, --help             Print this message and exit"
  echo ""
}

BINDIR="/usr/local/bin"
OUTPUT="-"

while [ $# -gt 0 ]; do
  key="$1"

  case $key in
  -o | --output)
    OUTPUT="$2"
    shift
    shift
    ;;
  -b | --bindir)
    BINDIR="$2"
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

if [ "${OUTPUT}" != "-" ]; then
  exec 1>"${OUTPUT}"
fi

cat << EOF
[Desktop Entry]
Name=WG Desktop
Version=1.5
Exec=${BINDIR}/mozillavpn ui %u
Comment=A standalone WireGuard desktop client.
Type=Application
Icon=io.github.srpvpn.wg-desktop
Terminal=false
StartupNotify=true
Categories=Network;Security;
MimeType=x-scheme-handler/wg-desktop
StartupWMClass=mozillavpn

EOF

# Do not emit legacy localized names. The existing translation catalog is
# still largely service-specific, so the standalone build keeps packaging
# metadata intentionally unlocalized until WG Desktop strings are translated.
