/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NETMGRUTILS_H
#define NETMGRUTILS_H

#include <QString>

namespace NetmgrUtils {

inline bool isDuplicateUuidError(const QString& message) {
  const QString lower = message.toLower();
  return lower.contains("uuid") && lower.contains("already exists");
}

}  // namespace NetmgrUtils

#endif  // NETMGRUTILS_H
