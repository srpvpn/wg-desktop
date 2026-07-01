/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDPROFILEMODEL_H
#define WIREGUARDPROFILEMODEL_H

#include <QAbstractListModel>
#include <QUrl>

#include "interfaceconfig.h"

class WireGuardProfileModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(WireGuardProfileModel)

  Q_PROPERTY(bool hasProfiles READ hasProfiles NOTIFY changed)
  Q_PROPERTY(QString activeProfileId READ activeProfileId NOTIFY changed)
  Q_PROPERTY(QString activeProfileName READ activeProfileName NOTIFY changed)
  Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

 public:
  enum ModelRoles {
    IdRole = Qt::UserRole + 1,
    ProfileIdRole,
    NameRole,
    ActiveRole,
  };
  Q_ENUM(ModelRoles)

  explicit WireGuardProfileModel(QObject* parent = nullptr);
  ~WireGuardProfileModel() override;

  bool hasProfiles() const { return !m_profiles.isEmpty(); }
  QString activeProfileId() const { return m_activeProfileId; }
  QString activeProfileName() const;
  QString lastError() const { return m_lastError; }

  Q_INVOKABLE bool addProfileFromText(const QString& name,
                                      const QString& configText);
  Q_INVOKABLE bool addProfileFromFile(const QString& name,
                                      const QUrl& fileUrl);
  bool addProfile(const QString& name, const QString& configText,
                  QString* error = nullptr);

  Q_INVOKABLE bool removeProfile(const QString& id);
  Q_INVOKABLE bool renameProfile(const QString& id, const QString& name);
  Q_INVOKABLE bool selectProfile(const QString& id);

  bool activeInterfaceConfig(InterfaceConfig& config,
                             QString* error = nullptr) const;

  QHash<int, QByteArray> roleNames() const override;
  int rowCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  void changed();
  void lastErrorChanged();

 private:
  struct Profile {
    QString id;
    QString name;
    QString configText;
  };

  int profileIndex(const QString& id) const;
  void loadFromSettings();
  void writeSettings() const;
  void setLastError(const QString& error);
  bool setError(QString* error, const QString& message);

 private:
  QList<Profile> m_profiles;
  QString m_activeProfileId;
  QString m_lastError;
};

#endif  // WIREGUARDPROFILEMODEL_H
