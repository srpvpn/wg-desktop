/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskgetsubscriptiondetails.h"

#include <QJsonObject>

#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/subscriptiondata.h"
#include "models/user.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskGetSubscriptionDetails");
}  // anonymous namespace

TaskGetSubscriptionDetails::TaskGetSubscriptionDetails(
    AuthenticationPolicy authenticationPolicy,
    ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy)
    : Task("TaskGetSubscriptionDetails"),
      m_authenticationPolicy(authenticationPolicy),
      m_errorPropagationPolicy(errorPropagationPolicy) {
  MZ_COUNT_CTOR(TaskGetSubscriptionDetails);
}

TaskGetSubscriptionDetails::~TaskGetSubscriptionDetails() {
  MZ_COUNT_DTOR(TaskGetSubscriptionDetails);
}

void TaskGetSubscriptionDetails::run() {
  logger.debug() << "Subscription details backend disabled in standalone mode";
  maybeComplete(false);
}

void TaskGetSubscriptionDetails::maybeComplete(bool status) {
  if (!status) {
    MozillaVPN::instance()->subscriptionData()->resetData();
  }

  emit operationCompleted(status);
  emit completed();
}
