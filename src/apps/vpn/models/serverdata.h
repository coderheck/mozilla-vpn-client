/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERDATA_H
#define SERVERDATA_H

#include <QObject>

class ServerCountryModel;
class ServerCountry;
class ServerCity;
class Server;

class ServerData final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ServerData);

  Q_PROPERTY(QString exitCountryCode READ exitCountryCode NOTIFY changed)
  Q_PROPERTY(QString exitCityName READ exitCityName NOTIFY changed)
  Q_PROPERTY(
      QString localizedExitCityName READ localizedExitCityName NOTIFY changed)

  Q_PROPERTY(bool multihop READ multihop NOTIFY changed)

  Q_PROPERTY(QString entryCountryCode READ entryCountryCode NOTIFY changed)
  Q_PROPERTY(QString entryCityName READ entryCityName NOTIFY changed)
  Q_PROPERTY(
      QString localizedEntryCityName READ localizedEntryCityName NOTIFY changed)

  Q_PROPERTY(QString previousExitCountryCode READ previousExitCountryCode NOTIFY
                 changed)
  Q_PROPERTY(
      QString previousExitCityName READ previousExitCityName NOTIFY changed)
  Q_PROPERTY(QString localizedPreviousExitCityName READ
                 localizedPreviousExitCityName NOTIFY changed)

 public:
  ServerData();
  ~ServerData();

  void initialize();

  [[nodiscard]] bool fromSettings();

  Q_INVOKABLE void changeServer(const QString& countryCode,
                                const QString& cityName,
                                const QString& entryCountryCode = QString(),
                                const QString& entryCityName = QString());
  bool hasServerData() const { return !m_exitCountryCode.isEmpty(); }

  const QList<Server> exitServers() const;
  const QList<Server> entryServers() const;

  const QString& exitCountryCode() const { return m_exitCountryCode; }
  const QString& exitCityName() const { return m_exitCityName; }
  QString localizedExitCityName() const;

  bool multihop() const {
    return !m_entryCountryCode.isEmpty() && !m_entryCityName.isEmpty();
  }

  const QString& entryCountryCode() const { return m_entryCountryCode; }
  const QString& entryCityName() const { return m_entryCityName; }
  QString localizedEntryCityName() const;

  const QString& previousExitCountryCode() const {
    return m_previousExitCountryCode;
  }
  const QString& previousExitCityName() const { return m_previousExitCityName; }
  QString localizedPreviousExitCityName() const;

  void forget();

  void update(const QString& exitCountryCode, const QString& exitCityName,
              const QString& entryCountryCode = QString(),
              const QString& entryCityName = QString());

  void retranslate() { emit changed(); }

 signals:
  void changed();

 private:
  bool settingsChanged();

 private:
  bool m_initialized = false;

  QString m_exitCountryCode;
  QString m_exitCityName;

  QString m_entryCountryCode;
  QString m_entryCityName;

  QString m_previousExitCountryCode;
  QString m_previousExitCityName;
};

#endif  // SERVERDATA_H
