/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandactivate.h"

#include <QEventLoop>
#include <QTextStream>

#include "commandlineparser.h"
#include "controller.h"
#include "leakdetector.h"
#include "mozillavpn.h"

CommandActivate::CommandActivate(QObject* parent)
    : Command(parent, "activate", "Activate the VPN tunnel") {
  MZ_COUNT_CTOR(CommandActivate);
}

CommandActivate::~CommandActivate() { MZ_COUNT_DTOR(CommandActivate); }

int CommandActivate::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  return runCommandLineApp([&]() {
    if (tokens.length() > 1) {
      QList<CommandLineParser::Option*> options;
      return CommandLineParser::unknownOption(this, tokens[1], tokens[0],
                                              options, false);
    }

    if (!userAuthenticated()) {
      return 1;
    }

    MozillaVPN vpn;
    if (!loadModels()) {
      return 1;
    }

    QEventLoop loop;
    QObject::connect(
        vpn.connectionManager(), &ConnectionManager::stateChanged, &vpn, [&] {
          if (vpn.connectionManager()->state() == ConnectionManager::StateOff ||
              vpn.connectionManager()->state() == ConnectionManager::StateOn) {
            loop.exit();
          }
        });

    vpn.connectionManager()->initialize();
    loop.exec();
    vpn.connectionManager()->disconnect();

    // If we are connecting right now, we want to wait untile the operation is
    // completed.
    if (vpn.connectionManager()->state() != ConnectionManager::StateOff &&
        vpn.connectionManager()->state() != ConnectionManager::StateOn) {
      QObject::connect(vpn.connectionManager(),
                       &ConnectionManager::stateChanged, &vpn, [&] {
                         if (vpn.connectionManager()->state() ==
                                 ConnectionManager::StateOff ||
                             vpn.connectionManager()->state() ==
                                 ConnectionManager::StateOn) {
                           loop.exit();
                         }
                       });
      loop.exec();
      vpn.connectionManager()->disconnect();
    }

    if (vpn.connectionManager()->state() != ConnectionManager::StateOff) {
      QTextStream stream(stdout);
      stream << "The VPN tunnel is already active" << Qt::endl;
      return 0;
    }

    QObject::connect(
        vpn.connectionManager(), &ConnectionManager::stateChanged, &vpn, [&] {
          if (vpn.connectionManager()->state() == ConnectionManager::StateOff ||
              vpn.connectionManager()->state() == ConnectionManager::StateOn) {
            loop.exit();
          }
        });
    vpn.connectionManager()->activate(*vpn.serverData());
    loop.exec();
    vpn.connectionManager()->disconnect();

    if (vpn.connectionManager()->state() == ConnectionManager::StateOn) {
      QTextStream stream(stdout);
      stream << "The VPN tunnel is now active" << Qt::endl;
      return 0;
    }

    QTextStream stream(stdout);
    stream << "The VPN tunnel activation failed" << Qt::endl;
    return 1;
  });
}

static Command::RegistrationProxy<CommandActivate> s_commandActivate;
