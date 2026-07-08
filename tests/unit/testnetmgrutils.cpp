/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testnetmgrutils.h"

#include "platforms/linux/netmgrutils.h"

void TestNetmgrUtils::detectsDuplicateUuidErrors() {
  QVERIFY(NetmgrUtils::isDuplicateUuidError(
      "a connection with this UUID already exists"));
}

void TestNetmgrUtils::ignoresUnrelatedErrors() {
  QVERIFY(!NetmgrUtils::isDuplicateUuidError("permission denied"));
}

void TestNetmgrUtils::requiresNewRxTrafficAfterActivation() {
  QVERIFY(!NetmgrUtils::hasNewHandshakeTraffic(256, 256));
  QVERIFY(!NetmgrUtils::hasNewHandshakeTraffic(256, 128));
  QVERIFY(NetmgrUtils::hasNewHandshakeTraffic(256, 512));
}

void TestNetmgrUtils::detectsFullTunnelIpv4Profiles() {
  QVERIFY(NetmgrUtils::hasIpv4DefaultRoute({"0.0.0.0/0"}));
  QVERIFY(NetmgrUtils::hasIpv4DefaultRoute({"10.0.0.0/8", "0.0.0.0/0"}));
  QVERIFY(!NetmgrUtils::hasIpv4DefaultRoute({"10.0.0.0/8"}));
  QVERIFY(!NetmgrUtils::hasIpv4DefaultRoute({"::/0"}));
}

void TestNetmgrUtils::validatesKernelRouteInterface() {
  QVERIFY(NetmgrUtils::routeOutputUsesInterface(
      "1.1.1.1 dev wg0 src 192.168.6.148 uid 1000\n    cache\n", "wg0"));
  QVERIFY(!NetmgrUtils::routeOutputUsesInterface(
      "1.1.1.1 dev amn0 src 10.8.1.3 uid 1000\n    cache\n", "wg0"));
  QVERIFY(!NetmgrUtils::routeOutputUsesInterface(
      "1.1.1.1 via 192.168.0.1 dev wlo1 src 192.168.0.104 uid 1000\n",
      "wg0"));
}

void TestNetmgrUtils::detectsBlockingMainRoutes() {
  const QString routeTable =
      "Iface\tDestination\tGateway \tFlags\tRefCnt\tUse\tMetric\tMask\t\tMTU\tWindow\tIRTT\n"
      "amn0\t00000000\t00000000\t0001\t0\t0\t1\t00000080\t0\t0\t0\n"
      "wlo1\t00000000\t0100A8C0\t0003\t0\t0\t600\t00000000\t0\t0\t0\n";
  QVERIFY(NetmgrUtils::mainRouteBlocksInterface(routeTable, "1.1.1.1",
                                                "wg0"));

  const QString defaultOnly =
      "Iface\tDestination\tGateway \tFlags\tRefCnt\tUse\tMetric\tMask\t\tMTU\tWindow\tIRTT\n"
      "wlo1\t00000000\t0100A8C0\t0003\t0\t0\t600\t00000000\t0\t0\t0\n";
  QVERIFY(!NetmgrUtils::mainRouteBlocksInterface(defaultOnly, "1.1.1.1",
                                                 "wg0"));

  const QString wireGuardRoute =
      "Iface\tDestination\tGateway \tFlags\tRefCnt\tUse\tMetric\tMask\t\tMTU\tWindow\tIRTT\n"
      "wg0\t00000000\t00000000\t0001\t0\t0\t1\t00000080\t0\t0\t0\n";
  QVERIFY(!NetmgrUtils::mainRouteBlocksInterface(wireGuardRoute, "1.1.1.1",
                                                 "wg0"));
}

static TestNetmgrUtils s_testNetmgrUtils;
