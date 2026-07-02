/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandactivate.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QTextStream>

#include "commandlineparser.h"
#include "constants.h"
#include "controller.h"
#include "interfaceconfig.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/wireguardprofilemodel.h"
#include "mozillavpn.h"

namespace {
bool isTerminalState(Controller::State state) {
  return state == Controller::StateOff || state == Controller::StateOn ||
         state == Controller::StateConnectionError ||
         state == Controller::StatePermissionRequired;
}
}  // namespace

CommandActivate::CommandActivate(QObject* parent)
    : Command(parent, "activate", "Activate the VPN tunnel") {
  MZ_COUNT_CTOR(CommandActivate);
}

CommandActivate::~CommandActivate() { MZ_COUNT_DTOR(CommandActivate); }

int CommandActivate::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  QString appName = tokens[0];

  CommandLineParser::Option hOption = CommandLineParser::helpOption();
  CommandLineParser::Option testingOption("t", "testing",
                                          "Run in testing mode.");

  QList<CommandLineParser::Option*> options;
  options.append(&hOption);
  options.append(&testingOption);

  CommandLineParser clp;
  if (clp.parse(tokens, options, false)) {
    return 1;
  }

  if (hOption.m_set) {
    clp.showHelp(this, appName, options, false, false);
    return 0;
  }

  if (testingOption.m_set) {
    QCoreApplication::setOrganizationName("WG Desktop Testing");
    LogHandler::instance()->setStderr(true);
  }

  return MozillaVPN::runCommandLineApp([&]() {
    MozillaVPN vpn;
    if (testingOption.m_set) {
      Constants::setStaging();
    }

    QTextStream stream(stdout);
    WireGuardProfileModel* profiles = vpn.wireGuardProfileModel();
    Q_ASSERT(profiles);
    InterfaceConfig activeProfileConfig;
    QString profileError;
    if (!profiles->activeInterfaceConfig(activeProfileConfig, &profileError)) {
      stream << profileError << Qt::endl;
      return 1;
    }

    QEventLoop loop;
    QObject::connect(vpn.controller(), &Controller::stateChanged, &vpn, [&] {
      if (isTerminalState(vpn.controller()->state())) {
        loop.exit();
      }
    });

    vpn.controller()->initialize();
    if (!isTerminalState(vpn.controller()->state())) {
      loop.exec();
    }
    vpn.controller()->disconnect();

    // If we are connecting right now, we want to wait untile the operation is
    // completed.
    if (!isTerminalState(vpn.controller()->state())) {
      QObject::connect(vpn.controller(), &Controller::stateChanged, &vpn, [&] {
        if (isTerminalState(vpn.controller()->state())) {
          loop.exit();
        }
      });
      loop.exec();
      vpn.controller()->disconnect();
    }

    if (vpn.controller()->state() != Controller::StateOff) {
      QTextStream stream(stdout);
      stream << "The VPN tunnel is already active" << Qt::endl;
      return 0;
    }

    QObject::connect(vpn.controller(), &Controller::stateChanged, &vpn, [&] {
      if (isTerminalState(vpn.controller()->state())) {
        loop.exit();
      }
    });
    vpn.controller()->activate(activeProfileConfig, Controller::ClientUser);
    if (!isTerminalState(vpn.controller()->state())) {
      loop.exec();
    }
    vpn.controller()->disconnect();

    if (vpn.controller()->state() == Controller::StateOn) {
      stream << "The VPN tunnel is now active" << Qt::endl;
      return 0;
    }

    stream << "The VPN tunnel activation failed" << Qt::endl;
    return 1;
  });
}

static Command::RegistrationProxy<CommandActivate> s_commandActivate;
