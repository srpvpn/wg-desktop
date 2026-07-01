/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskaddonindex.h"

#include "addons/manager/addonmanager.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskAddonIndex");
}

TaskAddonIndex::TaskAddonIndex() : Task("TaskAddonIndex") {
  MZ_COUNT_CTOR(TaskAddonIndex);
}

TaskAddonIndex::~TaskAddonIndex() { MZ_COUNT_DTOR(TaskAddonIndex); }

void TaskAddonIndex::run() {
  logger.debug() << "Remote addon index fetch disabled in standalone mode";
  AddonManager::instance()->updateIndex(false);
  emit completed();
}

void TaskAddonIndex::maybeComplete() {
  if (m_indexData.isEmpty()) {
    return;
  }

  if (AddonManager::signatureVerificationNeeded() &&
      m_indexSignData.isEmpty()) {
    return;
  }

  AddonManager::instance()->updateIndex(true, m_indexData, m_indexSignData);
  emit completed();
}
