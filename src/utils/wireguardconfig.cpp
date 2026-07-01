/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wireguardconfig.h"

#include <QHostAddress>
#include <QMap>

#include "interfaceconfig.h"

namespace {

constexpr const char* SECTION_INTERFACE = "interface";
constexpr const char* SECTION_PEER = "peer";

void setError(QString* error, const QString& message) {
  if (error) {
    *error = message;
  }
}

QString stripComment(QString line) {
  const int hashIndex = line.indexOf('#');
  const int semicolonIndex = line.indexOf(';');
  int commentIndex = -1;

  if (hashIndex >= 0 && semicolonIndex >= 0) {
    commentIndex = std::min(hashIndex, semicolonIndex);
  } else if (hashIndex >= 0) {
    commentIndex = hashIndex;
  } else if (semicolonIndex >= 0) {
    commentIndex = semicolonIndex;
  }

  if (commentIndex >= 0) {
    line.truncate(commentIndex);
  }
  return line.trimmed();
}

bool isValidAddressWithPrefix(const QString& value) {
  if (!value.contains('/')) {
    return QHostAddress(value).protocol() != QAbstractSocket::UnknownNetworkLayerProtocol;
  }

  const QPair<QHostAddress, int> subnet = QHostAddress::parseSubnet(value);
  return subnet.first.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol &&
         subnet.second >= 0;
}

bool appendAddress(const QString& value, QString& ipv4, QString& ipv6,
                   QString* error) {
  if (!isValidAddressWithPrefix(value)) {
    setError(error, QString("Invalid Address value: %1").arg(value));
    return false;
  }

  const QString host = value.section('/', 0, 0);
  const QHostAddress address(host);
  if (address.protocol() == QAbstractSocket::IPv4Protocol) {
    ipv4 = value;
    return true;
  }
  if (address.protocol() == QAbstractSocket::IPv6Protocol) {
    ipv6 = value;
    return true;
  }

  setError(error, QString("Invalid Address value: %1").arg(value));
  return false;
}

bool parseEndpoint(const QString& value, QString& ipv4OrHost, QString& ipv6,
                   int& port, QString* error) {
  QString host;
  QString portValue;

  if (value.startsWith('[')) {
    const int closeBracket = value.indexOf(']');
    if (closeBracket < 0 || value.mid(closeBracket + 1, 1) != ":") {
      setError(error, "Endpoint IPv6 address must use [address]:port format");
      return false;
    }
    host = value.mid(1, closeBracket - 1);
    portValue = value.mid(closeBracket + 2);
  } else {
    const int separator = value.lastIndexOf(':');
    if (separator <= 0 || separator == value.length() - 1) {
      setError(error, "Endpoint must include host and port");
      return false;
    }
    host = value.left(separator);
    portValue = value.mid(separator + 1);

    if (host.contains(':')) {
      setError(error, "Endpoint IPv6 address must use [address]:port format");
      return false;
    }
  }

  bool ok = false;
  const int parsedPort = portValue.toInt(&ok);
  if (!ok || parsedPort <= 0 || parsedPort > 65535) {
    setError(error, QString("Invalid Endpoint port: %1").arg(portValue));
    return false;
  }

  const QHostAddress address(host);
  if (address.protocol() == QAbstractSocket::IPv6Protocol) {
    ipv6 = host;
  } else {
    ipv4OrHost = host;
  }
  port = parsedPort;
  return true;
}

bool appendAllowedIp(const QString& value, QList<IPAddress>& ranges,
                     QString* error) {
  if (!isValidAddressWithPrefix(value)) {
    setError(error, QString("Invalid AllowedIPs value: %1").arg(value));
    return false;
  }
  ranges.append(IPAddress(value));
  return true;
}

QStringList splitCsv(const QString& value) {
  QStringList output;
  for (const QString& item : value.split(',')) {
    const QString trimmed = item.trimmed();
    if (!trimmed.isEmpty()) {
      output.append(trimmed);
    }
  }
  return output;
}

}  // namespace

bool WireGuardConfig::parse(const QString& content, InterfaceConfig& config,
                            QString* error) {
  InterfaceConfig parsed;
  parsed.m_hopType = InterfaceConfig::SingleHop;

  QString section;
  const QStringList lines = content.split('\n');
  for (int index = 0; index < lines.length(); ++index) {
    const QString line = stripComment(lines.at(index));
    if (line.isEmpty()) {
      continue;
    }

    if (line.startsWith('[') && line.endsWith(']')) {
      section = line.mid(1, line.length() - 2).trimmed().toLower();
      continue;
    }

    const int separator = line.indexOf('=');
    if (separator <= 0) {
      setError(error, QString("Invalid WireGuard line %1").arg(index + 1));
      return false;
    }

    const QString key = line.left(separator).trimmed().toLower();
    const QString value = line.mid(separator + 1).trimmed();
    if (value.isEmpty()) {
      setError(error, QString("Empty WireGuard value for %1").arg(key));
      return false;
    }

    if (section == SECTION_INTERFACE) {
      if (key == "privatekey") {
        parsed.m_privateKey = value;
      } else if (key == "address") {
        for (const QString& address : splitCsv(value)) {
          if (!appendAddress(address, parsed.m_deviceIpv4Address,
                             parsed.m_deviceIpv6Address, error)) {
            return false;
          }
        }
      } else if (key == "dns") {
        const QStringList dnsServers = splitCsv(value);
        if (!dnsServers.isEmpty()) {
          parsed.m_dnsServer = dnsServers.first();
        }
      }
    } else if (section == SECTION_PEER) {
      if (key == "publickey") {
        parsed.m_serverPublicKey = value;
      } else if (key == "endpoint") {
        if (!parseEndpoint(value, parsed.m_serverIpv4AddrIn,
                           parsed.m_serverIpv6AddrIn, parsed.m_serverPort,
                           error)) {
          return false;
        }
      } else if (key == "allowedips") {
        for (const QString& range : splitCsv(value)) {
          if (!appendAllowedIp(range, parsed.m_allowedIPAddressRanges, error)) {
            return false;
          }
        }
      }
    }
  }

  if (parsed.m_privateKey.isEmpty()) {
    setError(error, "Interface PrivateKey is required");
    return false;
  }
  if (parsed.m_deviceIpv4Address.isEmpty() &&
      parsed.m_deviceIpv6Address.isEmpty()) {
    setError(error, "Interface Address is required");
    return false;
  }
  if (parsed.m_serverPublicKey.isEmpty()) {
    setError(error, "Peer PublicKey is required");
    return false;
  }
  if (parsed.m_serverIpv4AddrIn.isEmpty() &&
      parsed.m_serverIpv6AddrIn.isEmpty()) {
    setError(error, "Peer Endpoint is required");
    return false;
  }
  if (parsed.m_serverPort == 0) {
    setError(error, "Peer Endpoint port is required");
    return false;
  }
  if (parsed.m_allowedIPAddressRanges.isEmpty()) {
    setError(error, "Peer AllowedIPs is required");
    return false;
  }

  config = parsed;
  setError(error, QString());
  return true;
}
