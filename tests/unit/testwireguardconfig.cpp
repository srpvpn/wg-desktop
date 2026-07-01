/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testwireguardconfig.h"

#include "interfaceconfig.h"
#include "wireguardconfig.h"

void TestWireGuardConfig::parseSinglePeerConfig() {
  const QString config = R"(
[Interface]
PrivateKey = localPrivateKey=
Address = 10.64.222.2/32, fd00:1234::2/128
DNS = 10.64.0.1

[Peer]
PublicKey = remotePublicKey=
Endpoint = 198.51.100.7:51820
AllowedIPs = 0.0.0.0/0, ::/0
PersistentKeepalive = 25
)";

  InterfaceConfig parsed;
  QString error;

  QVERIFY(WireGuardConfig::parse(config, parsed, &error));
  QCOMPARE(error, QString());
  QCOMPARE(parsed.m_hopType, InterfaceConfig::SingleHop);
  QCOMPARE(parsed.m_privateKey, QString("localPrivateKey="));
  QCOMPARE(parsed.m_deviceIpv4Address, QString("10.64.222.2/32"));
  QCOMPARE(parsed.m_deviceIpv6Address, QString("fd00:1234::2/128"));
  QCOMPARE(parsed.m_dnsServer, QString("10.64.0.1"));
  QCOMPARE(parsed.m_serverPublicKey, QString("remotePublicKey="));
  QCOMPARE(parsed.m_serverIpv4AddrIn, QString("198.51.100.7"));
  QCOMPARE(parsed.m_serverIpv6AddrIn, QString());
  QCOMPARE(parsed.m_serverPort, 51820);
  QCOMPARE(parsed.m_allowedIPAddressRanges.length(), 2);
  QCOMPARE(parsed.m_allowedIPAddressRanges.at(0).toString(),
           QString("0.0.0.0/0"));
  QCOMPARE(parsed.m_allowedIPAddressRanges.at(1).toString(), QString("::/0"));
}

void TestWireGuardConfig::parseHostnameEndpoint() {
  const QString config = R"(
[Interface]
PrivateKey = localPrivateKey=
Address = 10.64.222.2/32

[Peer]
PublicKey = remotePublicKey=
Endpoint = vpn.example.test:51820
AllowedIPs = 0.0.0.0/0
)";

  InterfaceConfig parsed;
  QString error;

  QVERIFY(WireGuardConfig::parse(config, parsed, &error));
  QCOMPARE(error, QString());
  QCOMPARE(parsed.m_serverIpv4AddrIn, QString("vpn.example.test"));
  QCOMPARE(parsed.m_serverIpv6AddrIn, QString());
  QCOMPARE(parsed.m_serverPort, 51820);
}

void TestWireGuardConfig::parseIpv6Endpoint() {
  const QString config = R"(
[Interface]
PrivateKey = localPrivateKey=
Address = fd00:1234::2/128

[Peer]
PublicKey = remotePublicKey=
Endpoint = [2001:db8::7]:51820
AllowedIPs = ::/0
)";

  InterfaceConfig parsed;
  QString error;

  QVERIFY(WireGuardConfig::parse(config, parsed, &error));
  QCOMPARE(error, QString());
  QCOMPARE(parsed.m_serverIpv4AddrIn, QString());
  QCOMPARE(parsed.m_serverIpv6AddrIn, QString("2001:db8::7"));
  QCOMPARE(parsed.m_serverPort, 51820);
}

void TestWireGuardConfig::parseRejectsMissingEndpoint() {
  const QString config = R"(
[Interface]
PrivateKey = localPrivateKey=
Address = 10.64.222.2/32

[Peer]
PublicKey = remotePublicKey=
AllowedIPs = 0.0.0.0/0
)";

  InterfaceConfig parsed;
  QString error;

  QVERIFY(!WireGuardConfig::parse(config, parsed, &error));
  QVERIFY(error.contains("Endpoint"));
}

static TestWireGuardConfig s_testWireGuardConfig;
