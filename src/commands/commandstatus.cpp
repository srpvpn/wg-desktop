/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandstatus.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
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

QString stateName(Controller::State state) {
  switch (state) {
    case Controller::StateInitializing:
      return "initializing";
    case Controller::StatePermissionRequired:
      return "permission-required";
    case Controller::StateOff:
      return "off";
    case Controller::StateRegeneratingKey:
      return "regenerating-key";
    case Controller::StateConnecting:
      return "connecting";
    case Controller::StateConnectionError:
      return "connection-error";
    case Controller::StateConfirming:
      return "confirming";
    case Controller::StateOn:
    case Controller::StateOnPartial:
    case Controller::StateSilentSwitching:
      return "on";
    case Controller::StateDisconnecting:
      return "disconnecting";
    case Controller::StateSwitching:
      return "switching";
  }

  Q_UNREACHABLE();
}

QString errorName(Controller::ErrorCode error) {
  switch (error) {
    case Controller::ErrorNone:
      return "none";
    case Controller::ErrorFatal:
      return "fatal";
    case Controller::ErrorSplitTunnelInit:
      return "split-tunnel-init";
    case Controller::ErrorSplitTunnelStart:
      return "split-tunnel-start";
    case Controller::ErrorSplitTunnelExclude:
      return "split-tunnel-exclude";
    case Controller::ErrorServerTimeout:
      return "server-timeout";
    case Controller::ErrorNoServerAvailable:
      return "no-server-available";
  }

  Q_UNREACHABLE();
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
  CommandLineParser::Option jsonOption("j", "json",
                                       "Output status as JSON.");
  CommandLineParser::Option testingOption("t", "testing",
                                          "Run in testing mode.");

  QList<CommandLineParser::Option*> options;
  options.append(&hOption);
  options.append(&cacheOption);
  options.append(&jsonOption);
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

    if (jsonOption.m_set) {
      QJsonObject status;
      status["profile_count"] = profiles->rowCount({});
      status["active_profile"] = profiles->hasProfiles()
                                     ? profiles->activeProfileName()
                                     : QJsonValue::Null;
      status["vpn_state"] = stateName(controller.state());
      status["error"] = errorName(controller.error());
      stream << QJsonDocument(status).toJson(QJsonDocument::Compact)
             << Qt::endl;
    } else {
      stream << "Profile count: " << profiles->rowCount({}) << Qt::endl;
      stream << "Active profile: ";
      if (profiles->hasProfiles()) {
        stream << profiles->activeProfileName() << Qt::endl;
      } else {
        stream << "none" << Qt::endl;
      }
      stream << "VPN state: " << stateName(controller.state()) << Qt::endl;
    }

    return 0;
  });
}

static Command::RegistrationProxy<CommandStatus> s_commandStatus;
