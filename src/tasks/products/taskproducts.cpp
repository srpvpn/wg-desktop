/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskproducts.h"

#include "constants.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "productshandler.h"

namespace {
Logger logger("TaskProducts");
}

TaskProducts::TaskProducts() : Task("TaskProducts") {
  MZ_COUNT_CTOR(TaskProducts);
}

TaskProducts::~TaskProducts() { MZ_COUNT_DTOR(TaskProducts); }

void TaskProducts::run() {
  logger.debug() << "Product plan fetch disabled in standalone mode";
  emit completed();
}
