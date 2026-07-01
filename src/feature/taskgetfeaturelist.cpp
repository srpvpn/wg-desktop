/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskgetfeaturelist.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

#include "constants.h"
#include "feature/featuremodel.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "settingsholder.h"

namespace {
Logger logger("TaskGetFeatureList");
}

TaskGetFeatureList::TaskGetFeatureList() : Task("TaskGetFeatureList") {
  MZ_COUNT_CTOR(TaskGetFeatureList);
}

TaskGetFeatureList::~TaskGetFeatureList() { MZ_COUNT_DTOR(TaskGetFeatureList); }

void TaskGetFeatureList::run() {
  logger.debug() << "Remote feature list fetch disabled in standalone mode";
  emit completed();
}
