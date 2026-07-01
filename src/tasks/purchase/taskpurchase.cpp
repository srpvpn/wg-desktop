/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskpurchase.h"

#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#ifdef MZ_IOS
#  include "platforms/ios/iosutils.h"
#endif

#include <QJsonObject>

namespace {
Logger logger("TaskPurchase");
}

#ifdef MZ_IOS
// static
TaskPurchase* TaskPurchase::createForIOS(const QString& originalTransactionId) {
  TaskPurchase* task = new TaskPurchase(IOS);
  task->m_iOSOriginalTransactionId = originalTransactionId;
  return task;
}
#endif

#ifdef MZ_ANDROID
// static
TaskPurchase* TaskPurchase::createForAndroid(const QString& sku,
                                             const QString& token) {
  TaskPurchase* task = new TaskPurchase(Android);
  task->m_androidSku = sku;
  task->m_androidToken = token;
  return task;
}
#endif

#ifdef MZ_WASM
// static
TaskPurchase* TaskPurchase::createForWasm(const QString& productId) {
  TaskPurchase* task = new TaskPurchase(Wasm);
  task->m_productId = productId;
  return task;
}
#endif

TaskPurchase::TaskPurchase(Op op) : Task("TaskPurchase"), m_op(op) {
  MZ_COUNT_CTOR(TaskPurchase);
}

TaskPurchase::~TaskPurchase() { MZ_COUNT_DTOR(TaskPurchase); }

void TaskPurchase::run() {
  logger.debug() << "Purchase backend disabled in standalone mode";
  emit failed(QNetworkReply::OperationCanceledError, QByteArray());
  emit completed();
}
