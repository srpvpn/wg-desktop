/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDCONFIG_H
#define WIREGUARDCONFIG_H

#include <QString>

class InterfaceConfig;

class WireGuardConfig final {
 public:
  static bool parse(const QString& content, InterfaceConfig& config,
                    QString* error = nullptr);
};

#endif  // WIREGUARDCONFIG_H
