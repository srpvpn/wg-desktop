/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskremovedevice.h"

#include "constants.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/user.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskRemoveDevice");
}

TaskRemoveDevice::TaskRemoveDevice(const QString& publicKey)
    : Task("TaskRemoveDevice"),
      m_publicKey(publicKey),
      // It could be that we are removing our own device. We need to complete
      // the operation even if the client resets the token. Because of this,
      // let's take a copy of the authorization header now. At ::run(), it could
      // be too late.
      m_authHeader(NetworkRequest::authorizationHeader()) {
  MZ_COUNT_CTOR(TaskRemoveDevice);
}

TaskRemoveDevice::~TaskRemoveDevice() {
  MZ_COUNT_DTOR(TaskRemoveDevice);

  // Nothing guarantees that when this task is deleted, the VPN object is still
  // alive. We cannot use the QObject-parenting solution because it deletes the
  // parent before the children.
  MozillaVPN* vpn = MozillaVPN::maybeInstance();
  if (vpn) {
    vpn->deviceRemovalCompleted(m_publicKey);
  }
}

void TaskRemoveDevice::run() {
  logger.debug() << "Removing the device with public key"
                 << logger.keys(m_publicKey);
  logger.debug() << "Device removal backend disabled in standalone mode";
  emit completed();
}
