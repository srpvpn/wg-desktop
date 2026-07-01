/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wireguardprofilemodel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QUuid>

#include "leakdetector.h"
#include "settingsholder.h"
#include "wireguardconfig.h"

namespace {

constexpr const char* JSON_PROFILES = "profiles";
constexpr const char* JSON_ID = "id";
constexpr const char* JSON_NAME = "name";
constexpr const char* JSON_CONFIG = "config";

}  // namespace

WireGuardProfileModel::WireGuardProfileModel(QObject* parent)
    : QAbstractListModel(parent) {
  MZ_COUNT_CTOR(WireGuardProfileModel);
  loadFromSettings();
}

WireGuardProfileModel::~WireGuardProfileModel() {
  MZ_COUNT_DTOR(WireGuardProfileModel);
}

QString WireGuardProfileModel::activeProfileName() const {
  const int index = profileIndex(m_activeProfileId);
  if (index < 0) {
    return QString();
  }
  return m_profiles.at(index).name;
}

bool WireGuardProfileModel::addProfileFromText(const QString& name,
                                               const QString& configText) {
  return addProfile(name, configText, nullptr);
}

bool WireGuardProfileModel::addProfileFromFile(const QString& name,
                                               const QUrl& fileUrl) {
  if (!fileUrl.isLocalFile()) {
    setLastError(tr("WireGuard configuration file must be local."));
    return false;
  }

  QFile file(fileUrl.toLocalFile());
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    setLastError(tr("Unable to open WireGuard configuration file."));
    return false;
  }

  const QString profileName =
      name.trimmed().isEmpty() ? QFileInfo(file).completeBaseName() : name;
  return addProfile(profileName, QString::fromUtf8(file.readAll()), nullptr);
}

bool WireGuardProfileModel::addProfile(const QString& name,
                                       const QString& configText,
                                       QString* error) {
  InterfaceConfig parsed;
  QString parseError;
  if (!WireGuardConfig::parse(configText, parsed, &parseError)) {
    return setError(error, parseError);
  }

  Profile profile;
  profile.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  profile.name = name.trimmed().isEmpty() ? tr("WireGuard Profile")
                                          : name.trimmed();
  profile.configText = configText;

  const int row = m_profiles.length();
  beginInsertRows(QModelIndex(), row, row);
  m_profiles.append(profile);
  endInsertRows();

  if (m_activeProfileId.isEmpty()) {
    m_activeProfileId = profile.id;
  }

  writeSettings();
  setLastError(QString());
  if (error) {
    error->clear();
  }
  emit changed();
  return true;
}

bool WireGuardProfileModel::removeProfile(const QString& id) {
  const int index = profileIndex(id);
  if (index < 0) {
    setLastError(tr("WireGuard profile not found."));
    return false;
  }

  beginRemoveRows(QModelIndex(), index, index);
  m_profiles.removeAt(index);
  endRemoveRows();

  if (m_activeProfileId == id) {
    m_activeProfileId =
        m_profiles.isEmpty() ? QString() : m_profiles.constFirst().id;
  }

  writeSettings();
  setLastError(QString());
  emit changed();
  return true;
}

bool WireGuardProfileModel::renameProfile(const QString& id,
                                          const QString& name) {
  const int index = profileIndex(id);
  if (index < 0) {
    setLastError(tr("WireGuard profile not found."));
    return false;
  }

  const QString trimmed = name.trimmed();
  if (trimmed.isEmpty()) {
    setLastError(tr("WireGuard profile name cannot be empty."));
    return false;
  }

  m_profiles[index].name = trimmed;
  emit dataChanged(this->index(index, 0), this->index(index, 0), {NameRole});

  writeSettings();
  setLastError(QString());
  emit changed();
  return true;
}

bool WireGuardProfileModel::selectProfile(const QString& id) {
  const int nextIndex = profileIndex(id);
  if (nextIndex < 0) {
    setLastError(tr("WireGuard profile not found."));
    return false;
  }

  const int previousIndex = profileIndex(m_activeProfileId);
  m_activeProfileId = id;

  if (previousIndex >= 0) {
    emit dataChanged(index(previousIndex, 0), index(previousIndex, 0),
                     {ActiveRole});
  }
  emit dataChanged(index(nextIndex, 0), index(nextIndex, 0), {ActiveRole});

  writeSettings();
  setLastError(QString());
  emit changed();
  return true;
}

bool WireGuardProfileModel::activeInterfaceConfig(InterfaceConfig& config,
                                                  QString* error) const {
  const int index = profileIndex(m_activeProfileId);
  if (index < 0) {
    if (error) {
      *error = tr("No active WireGuard profile selected.");
    }
    return false;
  }

  return WireGuardConfig::parse(m_profiles.at(index).configText, config, error);
}

QHash<int, QByteArray> WireGuardProfileModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[IdRole] = "id";
  roles[ProfileIdRole] = "profileId";
  roles[NameRole] = "name";
  roles[ActiveRole] = "active";
  return roles;
}

int WireGuardProfileModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return m_profiles.length();
}

QVariant WireGuardProfileModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= m_profiles.size()) {
    return QVariant();
  }

  const Profile& profile = m_profiles.at(index.row());
  switch (role) {
    case IdRole:
      [[fallthrough]];
    case ProfileIdRole:
      return profile.id;
    case NameRole:
      return profile.name;
    case ActiveRole:
      return profile.id == m_activeProfileId;
    default:
      return QVariant();
  }
}

int WireGuardProfileModel::profileIndex(const QString& id) const {
  for (int i = 0; i < m_profiles.length(); ++i) {
    if (m_profiles.at(i).id == id) {
      return i;
    }
  }
  return -1;
}

void WireGuardProfileModel::loadFromSettings() {
  beginResetModel();
  m_profiles.clear();
  m_activeProfileId = SettingsHolder::instance()->activeWireGuardProfileId();

  const QByteArray data = SettingsHolder::instance()->wireGuardProfiles();
  const QJsonDocument document = QJsonDocument::fromJson(data);
  if (document.isObject()) {
    const QJsonArray profiles =
        document.object().value(JSON_PROFILES).toArray();
    for (const QJsonValue& value : profiles) {
      const QJsonObject object = value.toObject();
      Profile profile;
      profile.id = object.value(JSON_ID).toString();
      profile.name = object.value(JSON_NAME).toString();
      profile.configText = object.value(JSON_CONFIG).toString();
      if (!profile.id.isEmpty() && !profile.name.isEmpty() &&
          !profile.configText.isEmpty()) {
        InterfaceConfig ignored;
        if (!WireGuardConfig::parse(profile.configText, ignored)) {
          continue;
        }
        m_profiles.append(profile);
      }
    }
  }

  if (profileIndex(m_activeProfileId) < 0) {
    m_activeProfileId =
        m_profiles.isEmpty() ? QString() : m_profiles.constFirst().id;
  }
  endResetModel();
}

void WireGuardProfileModel::writeSettings() const {
  QJsonArray profiles;
  for (const Profile& profile : m_profiles) {
    QJsonObject object;
    object.insert(JSON_ID, profile.id);
    object.insert(JSON_NAME, profile.name);
    object.insert(JSON_CONFIG, profile.configText);
    profiles.append(object);
  }

  QJsonObject root;
  root.insert(JSON_PROFILES, profiles);
  SettingsHolder::instance()->setWireGuardProfiles(
      QJsonDocument(root).toJson(QJsonDocument::Compact));
  SettingsHolder::instance()->setActiveWireGuardProfileId(m_activeProfileId);
}

void WireGuardProfileModel::setLastError(const QString& error) {
  if (m_lastError == error) {
    return;
  }
  m_lastError = error;
  emit lastErrorChanged();
}

bool WireGuardProfileModel::setError(QString* error, const QString& message) {
  if (error) {
    *error = message;
  }
  setLastError(message);
  return false;
}
