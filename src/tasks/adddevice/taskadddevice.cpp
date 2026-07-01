/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskadddevice.h"

#include <QJsonObject>
#include <QJsonValue>

#include "constants.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {

Logger logger("TaskAddDevice");

}  // anonymous namespace

TaskAddDevice::TaskAddDevice(const QString& deviceName, const QString& deviceID)
    : Task("TaskAddDevice"), m_deviceName(deviceName), m_deviceID(deviceID) {
  MZ_COUNT_CTOR(TaskAddDevice);
}

TaskAddDevice::~TaskAddDevice() { MZ_COUNT_DTOR(TaskAddDevice); }

void TaskAddDevice::run() {
  logger.debug() << "Adding the device" << logger.sensitive(m_deviceName);
  logger.debug() << "Device add backend disabled in standalone mode";
  emit completed();
}
