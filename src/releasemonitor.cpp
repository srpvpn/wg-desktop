/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "releasemonitor.h"

#include "constants.h"
#include "controller.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "tasks/release/taskrelease.h"
#include "taskscheduler.h"
#include "update/updater.h"

namespace {
Logger logger("ReleaseMonitor");
}

ReleaseMonitor::ReleaseMonitor() {
  MZ_COUNT_CTOR(ReleaseMonitor);

  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout, this, [this]() {
    ReleaseMonitor::runSoon(ErrorHandler::DoNotPropagateError);
  });
}

ReleaseMonitor::~ReleaseMonitor() { MZ_COUNT_DTOR(ReleaseMonitor); }

void ReleaseMonitor::runSoon(
    ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy) {
  Q_UNUSED(errorPropagationPolicy);

  logger.debug() << "Release check disabled in standalone mode";
  emit updateNotAvailable();
}

void ReleaseMonitor::schedule() {
  logger.debug() << "Release monitor scheduling disabled in standalone mode";
  m_timer.stop();
}

void ReleaseMonitor::updateRequired() {
  logger.warning() << "update required";
  MozillaVPN::instance()->controller()->updateRequired();
}

void ReleaseMonitor::updateSoon() {
  logger.debug() << "Release update disabled in standalone mode";
  MozillaVPN::instance()->setUpdating(false);
}
