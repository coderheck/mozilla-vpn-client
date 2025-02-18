/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DBUSSERVICE_H
#define DBUSSERVICE_H

#include <QDBusContext>

#include "apptracker.h"
#include "daemon/daemon.h"
#include "dnsutilslinux.h"
#include "iputilslinux.h"
#include "pidtracker.h"
#include "wireguardutilslinux.h"

class DbusAdaptor;

class DBusService final : public Daemon, protected QDBusContext {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DBusService)
  Q_CLASSINFO("D-Bus Interface", "org.mozilla.vpn.dbus")

 public:
  DBusService(QObject* parent);
  ~DBusService();

  void setAdaptor(DbusAdaptor* adaptor);

  using Daemon::activate;

 public slots:
  bool activate(const QString& jsonConfig);

  bool deactivate(bool emitSignals = true) override;
  QString status();

  QString version();
  QString getLogs();
  void cleanupLogs() { cleanLogs(); }

  QString runningApps();
  bool firewallApp(const QString& appName, const QString& state);
  bool firewallPid(int rootpid, const QString& state);
  bool firewallClear();

 protected:
  WireguardUtils* wgutils() const override { return m_wgutils; }
  bool supportIPUtils() const override { return true; }
  IPUtils* iputils() override;
  bool supportDnsUtils() const override { return true; }
  DnsUtils* dnsutils() override;

 private:
  bool removeInterfaceIfExists();
  bool isCallerAuthorized();
  void dropRootPermissions();

 private slots:
  void appLaunched(const QString& cgroup, const QString& appId, int rootpid);
  void appTerminated(const QString& cgroup, const QString& appId);

  void userListCompleted(QDBusPendingCallWatcher* call);
  void userCreated(uint uid, const QDBusObjectPath& path);
  void userRemoved(uint uid, const QDBusObjectPath& path);

 private:
  DbusAdaptor* m_adaptor = nullptr;
  WireguardUtilsLinux* m_wgutils = nullptr;
  IPUtilsLinux* m_iputils = nullptr;
  DnsUtilsLinux* m_dnsutils = nullptr;

  AppTracker* m_appTracker = nullptr;
  QList<QString> m_excludedApps;

  uint m_sessionUid = 0;
};

#endif  // DBUSSERVICE_H
