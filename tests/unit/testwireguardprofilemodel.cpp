/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testwireguardprofilemodel.h"

#include "interfaceconfig.h"
#include "models/wireguardprofilemodel.h"
#include "settingsholder.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

QString validConfig(const QString& privateKey, const QString& address,
                    const QString& publicKey, const QString& endpoint) {
  return QString(R"(
[Interface]
PrivateKey = %1
Address = %2
DNS = 10.64.0.1

[Peer]
PublicKey = %3
Endpoint = %4
AllowedIPs = 0.0.0.0/0, ::/0
)")
      .arg(privateKey, address, publicKey, endpoint);
}

}  // namespace

void TestWireGuardProfileModel::cleanup() { SettingsHolder::testCleanup(); }

void TestWireGuardProfileModel::addProfilePersistsAndSelectsFirstProfile() {
  WireGuardProfileModel model;
  QString error;

  QVERIFY(model.addProfile("Home", validConfig("privateA=", "10.0.0.2/32",
                                               "publicA=",
                                               "203.0.113.10:51820"),
                           &error));
  QCOMPARE(error, QString());
  QCOMPARE(model.rowCount(QModelIndex()), 1);
  QVERIFY(model.hasProfiles());
  QCOMPARE(model.activeProfileName(), QString("Home"));

  InterfaceConfig activeConfig;
  QVERIFY(model.activeInterfaceConfig(activeConfig, &error));
  QCOMPARE(activeConfig.m_privateKey, QString("privateA="));
  QCOMPARE(activeConfig.m_deviceIpv4Address, QString("10.0.0.2/32"));
  QCOMPARE(activeConfig.m_serverPublicKey, QString("publicA="));
  QCOMPARE(activeConfig.m_serverIpv4AddrIn, QString("203.0.113.10"));

  WireGuardProfileModel reloaded;
  QCOMPARE(reloaded.rowCount(QModelIndex()), 1);
  QCOMPARE(reloaded.activeProfileName(), QString("Home"));
  QVERIFY(reloaded.activeInterfaceConfig(activeConfig, &error));
  QCOMPARE(activeConfig.m_serverPort, 51820);
}

void TestWireGuardProfileModel::renameSelectAndRemoveProfiles() {
  WireGuardProfileModel model;
  QString error;

  QVERIFY(model.addProfile("Home", validConfig("privateA=", "10.0.0.2/32",
                                               "publicA=",
                                               "203.0.113.10:51820"),
                           &error));
  const QString firstId = model.activeProfileId();

  QVERIFY(model.addProfile("Office", validConfig("privateB=", "10.0.0.3/32",
                                                 "publicB=",
                                                 "198.51.100.9:53"),
                           &error));
  QCOMPARE(model.rowCount(QModelIndex()), 2);
  QCOMPARE(model.activeProfileId(), firstId);

  const QModelIndex secondIndex = model.index(1, 0);
  const QString secondId =
      model.data(secondIndex, WireGuardProfileModel::IdRole).toString();
  QVERIFY(!secondId.isEmpty());
  QCOMPARE(model.data(secondIndex, WireGuardProfileModel::ProfileIdRole)
               .toString(),
           secondId);

  QVERIFY(model.renameProfile(secondId, "Travel"));
  QCOMPARE(model.data(secondIndex, WireGuardProfileModel::NameRole).toString(),
           QString("Travel"));

  QVERIFY(model.selectProfile(secondId));
  QCOMPARE(model.activeProfileName(), QString("Travel"));

  QVERIFY(model.removeProfile(secondId));
  QCOMPARE(model.rowCount(QModelIndex()), 1);
  QCOMPARE(model.activeProfileId(), firstId);

  QVERIFY(model.removeProfile(firstId));
  QCOMPARE(model.rowCount(QModelIndex()), 0);
  QVERIFY(!model.hasProfiles());
  QCOMPARE(model.activeProfileId(), QString());
}

void TestWireGuardProfileModel::loadSkipsInvalidStoredProfiles() {
  const QString valid = validConfig("privateA=", "10.0.0.2/32", "publicA=",
                                    "203.0.113.10:51820");
  QJsonArray profiles;
  profiles.append(QJsonObject{
      {"id", "invalid"},
      {"name", "Invalid"},
      {"config",
       "[Interface]\nPrivateKey = missingPeer=\nAddress = 10.0.0.2/32\n"},
  });
  profiles.append(QJsonObject{
      {"id", "valid"},
      {"name", "Valid"},
      {"config", valid},
  });
  const QByteArray stored =
      QJsonDocument(QJsonObject{{"profiles", profiles}})
          .toJson(QJsonDocument::Compact);

  SettingsHolder::instance()->setWireGuardProfiles(stored);
  SettingsHolder::instance()->setActiveWireGuardProfileId("invalid");

  WireGuardProfileModel model;
  QCOMPARE(model.rowCount(QModelIndex()), 1);
  QCOMPARE(model.activeProfileId(), QString("valid"));
  QCOMPARE(model.activeProfileName(), QString("Valid"));
}

static TestWireGuardProfileModel s_testWireGuardProfileModel;
