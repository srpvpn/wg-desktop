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

static TestNetmgrUtils s_testNetmgrUtils;
