/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "purchasewebhandler.h"

#include "leakdetector.h"
#include "logger.h"
#include "taskscheduler.h"

namespace {
Logger logger("PurchaseWebHandler");
}  // namespace

PurchaseWebHandler::PurchaseWebHandler(QObject* parent)
    : PurchaseHandler(parent) {
  MZ_COUNT_CTOR(PurchaseWebHandler);
}

PurchaseWebHandler::~PurchaseWebHandler() { MZ_COUNT_DTOR(PurchaseWebHandler); }

void PurchaseWebHandler::startSubscription(const QString&) {
  logger.debug() << "Web subscription flow disabled in standalone mode";
  m_subscriptionState = eInactive;
  emit subscriptionFailed();
}

void PurchaseWebHandler::cancelSubscription() {
  logger.debug() << "Cancel subscription";
  m_subscriptionState = eInactive;
  // We delete all tasks so that we clean-up the authentication task we started
  // above.
  // TODO - Do we have a mechanism to just clean-up the one task?
  TaskScheduler::deleteTasks();
  emit subscriptionCanceled();
}

void PurchaseWebHandler::startRestoreSubscription() {
  logger.error() << "Restore not implemented!";
  emit subscriptionFailed();
}

void PurchaseWebHandler::nativeRegisterProducts() {
  logger.error()
      << "nativeRegisterProducts should not be called for PurchaseWebHandler";
  Q_ASSERT(false);
}
