/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNWIREGUARDPROFILEMODEL_H
#define VPNWIREGUARDPROFILEMODEL_H

#include <QQmlEngine>

#include "models/wireguardprofilemodel.h"
#include "mozillavpn.h"

struct VPNWireGuardProfileModel {
  Q_GADGET
  QML_FOREIGN(WireGuardProfileModel)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static WireGuardProfileModel* create(QQmlEngine* qmlEngine,
                                       QJSEngine* jsEngine) {
    Q_UNUSED(qmlEngine);
    Q_UNUSED(jsEngine);
    return MozillaVPN::instance()->wireGuardProfileModel();
  }
};

#endif  // VPNWIREGUARDPROFILEMODEL_H
