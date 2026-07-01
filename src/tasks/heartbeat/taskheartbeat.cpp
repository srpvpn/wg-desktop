/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskheartbeat.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskHeartbeat");
}

TaskHeartbeat::TaskHeartbeat() : Task("TaskHeartbeat") {
  MZ_COUNT_CTOR(TaskHeartbeat);
}

TaskHeartbeat::~TaskHeartbeat() { MZ_COUNT_DTOR(TaskHeartbeat); }

void TaskHeartbeat::run() {
  logger.debug() << "Heartbeat backend disabled in standalone mode";
  MozillaVPN::instance()->heartbeatCompleted(true);
  emit completed();
}
