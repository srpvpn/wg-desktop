/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NETMGRUTILS_H
#define NETMGRUTILS_H

#include <QHostAddress>
#include <QString>
#include <QStringList>

namespace NetmgrUtils {

inline bool isDuplicateUuidError(const QString& message) {
  const QString lower = message.toLower();
  return lower.contains("uuid") && lower.contains("already exists");
}

inline bool hasNewHandshakeTraffic(uint64_t baselineRxBytes,
                                   uint64_t currentRxBytes) {
  return currentRxBytes > baselineRxBytes;
}

inline bool hasIpv4DefaultRoute(const QStringList& allowedIpRanges) {
  return allowedIpRanges.contains("0.0.0.0/0");
}

inline bool routeOutputUsesInterface(const QString& routeOutput,
                                     const QString& interfaceName) {
  const QStringList tokens = routeOutput.simplified().split(' ');
  for (qsizetype i = 0; i + 1 < tokens.size(); ++i) {
    if (tokens[i] == "dev") {
      return tokens[i + 1] == interfaceName;
    }
  }
  return false;
}

inline quint32 routeHexToIpv4(const QString& value) {
  bool ok = false;
  const quint32 raw = value.toUInt(&ok, 16);
  if (!ok) {
    return 0;
  }
  return ((raw & 0x000000ff) << 24) | ((raw & 0x0000ff00) << 8) |
         ((raw & 0x00ff0000) >> 8) | ((raw & 0xff000000) >> 24);
}

inline int bitCount(quint32 value) {
  int count = 0;
  while (value != 0) {
    count += value & 1;
    value >>= 1;
  }
  return count;
}

inline bool mainRouteBlocksInterface(const QString& routeTable,
                                     const QString& probeAddress,
                                     const QString& interfaceName) {
  const QHostAddress probe(probeAddress);
  if (probe.protocol() != QAbstractSocket::IPv4Protocol) {
    return false;
  }

  const quint32 probeIpv4 = probe.toIPv4Address();
  int bestPrefixLength = -1;
  QString bestInterface;

  const QStringList lines = routeTable.split('\n', Qt::SkipEmptyParts);
  for (const QString& line : lines) {
    if (line.startsWith("Iface")) {
      continue;
    }

    const QStringList fields = line.simplified().split(' ');
    if (fields.size() < 8) {
      continue;
    }

    const quint32 destination = routeHexToIpv4(fields[1]);
    const quint32 mask = routeHexToIpv4(fields[7]);
    if ((probeIpv4 & mask) != (destination & mask)) {
      continue;
    }

    const int prefixLength = bitCount(mask);
    if (prefixLength > bestPrefixLength) {
      bestPrefixLength = prefixLength;
      bestInterface = fields[0];
    }
  }

  // NetworkManager suppresses only the main-table default route before looking
  // up the WireGuard fwmark table. More specific main-table routes still win.
  return bestPrefixLength > 0 && bestInterface != interfaceName;
}

}  // namespace NetmgrUtils

#endif  // NETMGRUTILS_H
