/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskaccount.h"

#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskAccount");
}

TaskAccount::TaskAccount(
    ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy)
    : Task("TaskAccount"), m_errorPropagationPolicy(errorPropagationPolicy) {
  MZ_COUNT_CTOR(TaskAccount);
}

TaskAccount::~TaskAccount() { MZ_COUNT_DTOR(TaskAccount); }

void TaskAccount::run() {
  logger.debug() << "Account backend disabled in standalone mode";
  emit completed();
}
