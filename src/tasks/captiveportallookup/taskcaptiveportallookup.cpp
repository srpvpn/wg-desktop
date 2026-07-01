/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskcaptiveportallookup.h"

#include "captiveportal/captiveportal.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskCaptivePortalLookup");
}

TaskCaptivePortalLookup::TaskCaptivePortalLookup(
    ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy)
    : Task("TaskCaptivePortalLookup"),
      m_errorPropagationPolicy(errorPropagationPolicy) {
  MZ_COUNT_CTOR(TaskCaptivePortalLookup);
}

TaskCaptivePortalLookup::~TaskCaptivePortalLookup() {
  MZ_COUNT_DTOR(TaskCaptivePortalLookup);
}

void TaskCaptivePortalLookup::run() {
  logger.debug()
      << "Remote captive portal detector lookup disabled in standalone mode";
  emit completed();
}
