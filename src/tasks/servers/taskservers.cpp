/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskservers.h"

#include "constants.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskServers");
}

TaskServers::TaskServers(
    ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy)
    : Task("TaskServers"), m_errorPropagationPolicy(errorPropagationPolicy) {
  MZ_COUNT_CTOR(TaskServers);
}

TaskServers::~TaskServers() { MZ_COUNT_DTOR(TaskServers); }

void TaskServers::run() {
  logger.debug() << "Remote server list fetch disabled in standalone mode";
  emit completed();
}
