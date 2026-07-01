/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandstatus.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QTextStream>

#include "commandlineparser.h"
#include "constants.h"
#include "controller.h"
#include "leakdetector.h"
#include "loghandler.h"
#include "models/wireguardprofilemodel.h"
#include "mozillavpn.h"

namespace {
bool isTerminalState(Controller::State state) {
  return state == Controller::StateOff || state == Controller::StateOn ||
         state == Controller::StateConnectionError ||
         state == Controller::StatePermissionRequired;
}
}  // namespace

CommandStatus::CommandStatus(QObject* parent)
    : Command(parent, "status", "Show the current VPN status.") {
  MZ_COUNT_CTOR(CommandStatus);
}

CommandStatus::~CommandStatus() { MZ_COUNT_DTOR(CommandStatus); }

int CommandStatus::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  QString appName = tokens[0];

  CommandLineParser::Option hOption = CommandLineParser::helpOption();
  CommandLineParser::Option cacheOption("c", "cache",
                                        "Accepted for compatibility.");
  CommandLineParser::Option testingOption("t", "testing",
                                          "Run in testing mode.");

  QList<CommandLineParser::Option*> options;
  options.append(&hOption);
  options.append(&cacheOption);
  options.append(&testingOption);

  CommandLineParser clp;
  if (clp.parse(tokens, options, false)) {
    return 1;
  }

  if (!tokens.isEmpty()) {
    QList<CommandLineParser::Option*> options;
    return CommandLineParser::unknownOption(this, tokens[1], tokens[0], options,
                                            false);
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

    stream << "Profile count: " << profiles->rowCount({}) << Qt::endl;
    stream << "Active profile: ";
    if (profiles->hasProfiles()) {
      stream << profiles->activeProfileName() << Qt::endl;
    } else {
      stream << "none" << Qt::endl;
    }

    Controller controller;

    QEventLoop loop;
    QObject::connect(&controller, &Controller::stateChanged, &controller, [&] {
      if (isTerminalState(controller.state())) {
        loop.exit();
      }
    });
    controller.initialize();
    if (!isTerminalState(controller.state())) {
      loop.exec();
    }

    stream << "VPN state: ";
    switch (controller.state()) {
      case Controller::StateInitializing:
        stream << "initializing";
        break;

      case Controller::StatePermissionRequired:
        stream << "permission-required";
        break;

      case Controller::StateOff:
        stream << "off";
        break;

      case Controller::StateRegeneratingKey:
        stream << "regenerating-key";
        break;

      case Controller::StateConnecting:
        stream << "connecting";
        break;

      case Controller::StateConnectionError:
        stream << "connection-error";
        break;

      case Controller::StateConfirming:
        stream << "confirming";
        break;

      case Controller::StateOn:
        [[fallthrough]];
      case Controller::StateOnPartial:
        [[fallthrough]];
      case Controller::StateSilentSwitching:
        stream << "on";
        break;

      case Controller::StateDisconnecting:
        stream << "disconnecting";
        break;

      case Controller::StateSwitching:
        stream << "switching";
        break;
    }

    stream << Qt::endl;

    return 0;
  });
}

static Command::RegistrationProxy<CommandStatus> s_commandStatus;
