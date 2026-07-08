/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "netmgrcontroller.h"

#include <sys/stat.h>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QFile>
#include <QScopeGuard>
#include <QUuid>

#include "dbustypes.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/device.h"
#include "models/keys.h"
#include "models/wireguardprofilemodel.h"
#include "mozillavpn.h"
#include "netmgrdevice.h"
#include "netmgrtypes.h"
#include "netmgrutils.h"
#include "settingsholder.h"

constexpr uint32_t WG_FIREWALL_MARK = 0xca6c;
constexpr const char* WG_INTERFACE_NAME = "wg0";
constexpr uint16_t WG_KEEPALIVE_PERIOD = 60;
constexpr uint16_t WG_EXCLUDE_RULE_PRIO = 100;
constexpr int WG_HANDSHAKE_POLL_MSEC = 500;

namespace {
Logger logger("NetmgrController");
}

NetmgrController::NetmgrController() {
  MZ_COUNT_CTOR(NetmgrController);

  m_client =
      new QDBusInterface(DBUS_NM_SERVICE, DBUS_NM_PATH, DBUS_NM_INTERFACE,
                         QDBusConnection::systemBus(), this);

  QVariant version = m_client->property("Version");
  m_version = QVersionNumber::fromString(version.toString());
  logger.info() << "NetworkManager version:" << m_version.toString();

  m_handshakePollTimer.setInterval(WG_HANDSHAKE_POLL_MSEC);
  connect(&m_handshakePollTimer, &QTimer::timeout, this,
          &NetmgrController::checkHandshakeTraffic);
}

NetmgrController::~NetmgrController() {
  MZ_COUNT_DTOR(NetmgrController);
  logger.debug() << "Destroying NetmgrController";
}

// static
QString NetmgrController::nmInterface(const QString& name) {
  return QStringLiteral(DBUS_NM_INTERFACE) + "." + name;
}

void NetmgrController::initialize(const Device* device, const Keys* keys) {
  // Ensure we use a consistent UUID for the wireguard interface.
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);
  QUuid uuid(settingsHolder->tunnelUuid());
  if (uuid.isNull()) {
    uuid = QUuid::createUuid();
    settingsHolder->setTunnelUuid(uuid.toString(QUuid::WithoutBraces));
  }
  m_uuid = uuid.toString(QUuid::WithoutBraces);

  // Generic connection settings.
  m_config.insert("id", QCoreApplication::applicationName());
  m_config.insert("interface-name", QString(WG_INTERFACE_NAME));
  m_config.insert("type", QString("wireguard"));
  m_config.insert("uuid", m_uuid);
  m_config.insert("autoconnect", false);
  // TODO: Permissions?
  // TODO: Timestamp?

  m_wireguard.insert("fwmark", WG_FIREWALL_MARK);
  m_wireguard.insert("ip4-auto-default-route", 1);
  m_wireguard.insert("ip6-auto-default-route", 1);
  m_wireguard.insert("peer-routes", true);

  if (device && keys) {
    InterfaceConfig config;
    config.m_privateKey = keys->privateKey();
    config.m_deviceIpv4Address = device->ipv4Address();
    config.m_deviceIpv6Address = device->ipv6Address();
    applyInterfaceConfig(config);
  } else {
    InterfaceConfig config;
    QString error;
    if (!MozillaVPN::instance()
             ->wireGuardProfileModel()
             ->activeInterfaceConfig(config, &error)) {
      logger.warning() << "Unable to initialize NetworkManager profile:"
                       << error;
      emit initialized(false, false, QDateTime());
      return;
    }
    applyInterfaceConfig(config);
  }

  QString path = QStringLiteral(DBUS_NM_PATH) + "/Settings";
  QDBusInterface iface(DBUS_NM_SERVICE, path, nmInterface("Settings"),
                       m_client->connection());

  // Check if the connection already exists.
  QDBusReply<QDBusObjectPath> reply = iface.call("GetConnectionByUuid", m_uuid);
  if (reply.isValid()) {
    logger.info() << "Connection" << m_uuid << "already exists";
    initCompleted(reply.value(), QVariantMap());
    return;
  }

  // Create the connection
  logger.info() << "Adding connection:" << m_uuid;
  QVariantList args;
  args << serializeConfig();
  args << (uint)IN_MEMORY;
  args << QVariantMap();
  bool okay = iface.callWithCallback(
      "AddConnection2", args, this,
      SLOT(initCompleted(const QDBusObjectPath&, const QVariantMap&)),
      SLOT(dbusInitError(const QDBusError&)));
  if (!okay) {
    logger.debug() << "AddConnection2 failed";
    emit initialized(false, false, QDateTime());
  }
}

void NetmgrController::applyInterfaceConfig(const InterfaceConfig& config) {
  m_wireguard.insert("private-key", config.m_privateKey);

  m_ipv4config.clear();
  m_ipv6config.clear();

  if (!config.m_deviceIpv4Address.isEmpty()) {
    QStringList ipv4split = config.m_deviceIpv4Address.split('/');
    QVariantMap ipv4address;
    ipv4address.insert("address", ipv4split.first());
    ipv4address.insert("prefix",
                       ipv4split.length() > 1 ? ipv4split[1].toUInt()
                                              : (uint)32);
    m_ipv4config.insert("method", "manual");
    m_ipv4config.insert("address-data", NetmgrDataList(ipv4address));
    m_deviceIpv4Address = QHostAddress(ipv4split.first());
  } else {
    m_ipv4config.insert("method", "disabled");
    m_deviceIpv4Address = QHostAddress();
  }

  if (!config.m_deviceIpv6Address.isEmpty()) {
    QStringList ipv6split = config.m_deviceIpv6Address.split('/');
    QVariantMap ipv6address;
    ipv6address.insert("address", ipv6split.first());
    ipv6address.insert("prefix",
                       ipv6split.length() > 1 ? ipv6split[1].toUInt()
                                              : (uint)128);
    m_ipv6config.insert("method", "manual");
    m_ipv6config.insert("address-data", NetmgrDataList(ipv6address));
    m_deviceIpv6Address = QHostAddress(ipv6split.first());
  } else {
    m_ipv6config.insert("method", "disabled");
    m_deviceIpv6Address = QHostAddress();
  }
}

void NetmgrController::initCompleted(const QDBusObjectPath& path,
                                     const QVariantMap& results) {
  logger.debug() << "connection created:" << path.path();
  m_remote = new QDBusInterface(DBUS_NM_SERVICE, path.path(),
                                nmInterface("Settings.Connection"),
                                m_client->connection(), this);

  // Monitor for changes in the connected devices.
  connect(m_client, SIGNAL(DeviceAdded(QDBusObjectPath)), this,
          SLOT(deviceAdded(QDBusObjectPath)));
  connect(m_client, SIGNAL(DeviceRemoved(QDBusObjectPath)), this,
          SLOT(deviceRemoved(QDBusObjectPath)));

  // Fetch the device in case it already exists.
  bool isConnected = false;
  QDBusReply<QDBusObjectPath> reply =
      m_client->call("GetDeviceByIpIface", WG_INTERFACE_NAME);
  if (reply.isValid()) {
    m_device = new NetmgrDevice(reply.value().path(), this);
    connect(m_device, &NetmgrDevice::stateChanged, this,
            &NetmgrController::deviceStateChanged);

    isConnected = m_device->state() == NetmgrDevice::ACTIVATED &&
                  m_device->uuid() == m_uuid;
  }

  emit initialized(true, isConnected, guessUptime());
}

void NetmgrController::dbusInitError(const QDBusError& error) {
  logger.warning() << "initialization failed:" << error.message();

  if (NetmgrUtils::isDuplicateUuidError(error.message())) {
    QString path = QStringLiteral(DBUS_NM_PATH) + "/Settings";
    QDBusInterface iface(DBUS_NM_SERVICE, path, nmInterface("Settings"),
                         m_client->connection());
    QDBusReply<QDBusObjectPath> reply =
        iface.call("GetConnectionByUuid", m_uuid);
    if (reply.isValid()) {
      logger.info() << "Reusing existing connection:" << m_uuid;
      initCompleted(reply.value(), QVariantMap());
      return;
    }

    logger.warning() << "Unable to reuse duplicate connection:"
                     << reply.error().message();
  }

  emit initialized(false, false, QDateTime());
}

void NetmgrController::dbusBackendError(const QDBusError& error) {
  logger.warning() << "DBus message failed:" << error.message();
  emit backendFailure(Controller::ErrorFatal);
}

// static
QVariantMap NetmgrController::wgPeer(const InterfaceConfig& config) {
  QStringList ipList;
  for (const IPAddress& i : config.m_allowedIPAddressRanges) {
    ipList.append(i.toString());
  }

  QVariantMap peer;
  QString endpoint;
  if (!config.m_serverIpv4AddrIn.isNull()) {
    endpoint =
        config.m_serverIpv4AddrIn + ":" + QString::number(config.m_serverPort);
  } else {
    endpoint = "[" + config.m_serverIpv6AddrIn +
               "]:" + QString::number(config.m_serverPort);
  }
  peer.insert("endpoint", endpoint);
  peer.insert("persistent-keepalive", WG_KEEPALIVE_PERIOD);
  peer.insert("public-key", config.m_serverPublicKey);
  peer.insert("allowed-ips", ipList);
  return peer;
}

// static
void NetmgrController::setDnsConfig(QVariantMap& map,
                                    const QHostAddress& server) {
  if (server.isNull()) {
    map.remove("dns");
    map.remove("dns-priority");
    map.remove("dns-search");
  } else if (server.protocol() == QAbstractSocket::IPv6Protocol) {
    NetmgrIpv6List dnsServers;
    dnsServers.append(server.toIPv6Address());
    map.insert("dns", dnsServers);
    map.insert("dns-priority", 10);
    map.insert("dns-search", QStringList("."));
  } else {
    QList<uint> dnsServers;
    dnsServers.append(qToBigEndian<quint32>(server.toIPv4Address()));
    map.insert("dns", QVariant::fromValue(dnsServers));
    map.insert("dns-priority", 10);
    map.insert("dns-search", QStringList("."));
  }
}

QVariant NetmgrController::serializeConfig() const {
  NetmgrConfig config;
  config.insert("connection", m_config);
  config.insert("ipv4", m_ipv4config);
  config.insert("ipv6", m_ipv6config);
  config.insert("wireguard", m_wireguard);
  return QVariant::fromValue(config);
}

void NetmgrController::activate(const InterfaceConfig& config,
                                Controller::Reason reason) {
  applyInterfaceConfig(config);
  m_waitingForHandshakeTraffic = true;
  m_activationRxBytes = 0;

  // Let NetworkManager derive routes and policy rules from peer AllowedIPs.
  // With ip*-auto-default-route enabled, default routes are placed in the
  // WireGuard fwmark table and matching policy rules are installed.
  QStringList allowedIpRanges;
  for (const IPAddress& ip : config.m_allowedIPAddressRanges) {
    allowedIpRanges.append(ip.toString());
  }
  m_requiresIpv4DefaultRoute =
      NetmgrUtils::hasIpv4DefaultRoute(allowedIpRanges);

  NetmgrDataList peers(wgPeer(config));
  m_ipv4config.remove("route-data");
  m_ipv6config.remove("route-data");
  m_wireguard.insert("peers", peers);

  // Keep the server details for later.
  m_serverPublicKey = config.m_serverPublicKey;
  m_serverIpv4Gateway = QHostAddress(config.m_serverIpv4Gateway);
  m_serverIpv6Gateway = QHostAddress(config.m_serverIpv6Gateway);

  // Update the DNS server.
  if ((config.m_dnsServer == config.m_serverIpv4Gateway) ||
      (config.m_dnsServer == config.m_serverIpv6Gateway)) {
    setDnsConfig(m_ipv4config, m_serverIpv4Gateway);
    setDnsConfig(m_ipv6config, m_serverIpv6Gateway);
  } else if (config.m_dnsServer.contains(':')) {
    setDnsConfig(m_ipv4config, QHostAddress());
    setDnsConfig(m_ipv6config, QHostAddress(config.m_dnsServer));
  } else {
    setDnsConfig(m_ipv4config, QHostAddress(config.m_dnsServer));
    setDnsConfig(m_ipv6config, QHostAddress());
  }

  // Update the connection settings.
  QList<QVariant> args;
  args << serializeConfig();
  args << (uint)IN_MEMORY;
  args << QVariantMap();
  bool okay = m_remote->callWithCallback(
      "Update2", args, this, SLOT(peerCompleted(const QVariantMap&)),
      SLOT(dbusBackendError(const QDBusError&)));
  if (!okay) {
    logger.debug() << "Update2 failed";
  }
}

void NetmgrController::peerCompleted(const QVariantMap& results) {
  logger.debug() << "Peer configured";
  for (auto i = results.constBegin(); i != results.constEnd(); i++) {
    logger.debug() << "peer result:" << i.key() << "->" << i.value().toString();
  }

  // activate the vpn
  QList<QVariant> args;
  args << QDBusObjectPath(m_remote->path());
  args << QDBusObjectPath("/");
  args << QDBusObjectPath("/");
  bool okay = m_client->callWithCallback(
      "ActivateConnection", args, this,
      SLOT(activateCompleted(const QDBusObjectPath&)),
      SLOT(dbusBackendError(const QDBusError&)));
  if (!okay) {
    logger.debug() << "ActivateConnection failed";
  }
}

void NetmgrController::activateCompleted(const QDBusObjectPath& path) {
  logger.info() << "Connection activated:" << path.path();
}

void NetmgrController::deactivate() {
  if (!m_device || m_device->uuid() != m_uuid) {
    logger.warning() << "Client already disconnected";
    emit disconnected();
    return;
  }

  QDBusMessage call = QDBusMessage::createMethodCall(
      DBUS_NM_SERVICE, DBUS_NM_PATH, DBUS_NM_INTERFACE, "DeactivateConnection");
  call << QDBusObjectPath(m_device->activeConnection());

  QDBusPendingReply<> reply = m_client->connection().asyncCall(call);
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);
  QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this,
                   [&] { emit disconnected(); });
  QObject::connect(watcher, &QDBusPendingCallWatcher::finished, watcher,
                   &QObject::deleteLater);
}

void NetmgrController::deviceAdded(const QDBusObjectPath& devpath) {
  NetmgrDevice* device = new NetmgrDevice(devpath.path(), this);
  auto guard = qScopeGuard([device]() { delete device; });

  logger.debug() << "device added:" << devpath.path();
  logger.debug() << "interface:" << device->name() << device->driver();
  if (device->name() != WG_INTERFACE_NAME) {
    return;
  }
  if (device->driver() != "wireguard") {
    return;
  }

  // Update the current device.
  if (m_device) {
    delete m_device;
  }
  m_device = device;
  guard.dismiss();

  // Watch it for state changes.
  connect(m_device, &NetmgrDevice::stateChanged, this,
          &NetmgrController::deviceStateChanged);
  deviceStateChanged(m_device->state(), NetmgrDevice::UNKNOWN, 0);
}

void NetmgrController::deviceRemoved(const QDBusObjectPath& path) {
  logger.debug() << "device removed:" << path.path();
  if (m_device && m_device->path() == path.path()) {
    m_handshakePollTimer.stop();
    m_waitingForHandshakeTraffic = false;
    delete m_device;
    m_device = nullptr;

    m_connectionPath = "/";
    emit disconnected();
  }
}

void NetmgrController::deviceStateChanged(uint state, uint prev, uint reason) {
  auto newstate = static_cast<NetmgrDevice::State>(state);
  auto prevstate = static_cast<NetmgrDevice::State>(prev);
  logger.debug() << "device state changed:" << prevstate << "->" << newstate;

  if (m_device->uuid() != m_uuid) {
    return;
  }

  if (newstate == NetmgrDevice::ACTIVATED) {
    m_device->setStatisticsRefreshRate(WG_HANDSHAKE_POLL_MSEC);
    m_activationRxBytes = m_device->rxBytes();
    logger.debug() << "WireGuard interface activated with rx baseline:"
                   << m_activationRxBytes;
    checkHandshakeTraffic();
    if (m_waitingForHandshakeTraffic) {
      m_handshakePollTimer.start();
    }
  } else if (newstate == NetmgrDevice::DEACTIVATING ||
             newstate == NetmgrDevice::FAILED) {
    m_handshakePollTimer.stop();
    m_waitingForHandshakeTraffic = false;
  }
}

void NetmgrController::checkHandshakeTraffic() {
  if (!m_waitingForHandshakeTraffic || !m_device ||
      m_device->uuid() != m_uuid ||
      m_device->state() != NetmgrDevice::ACTIVATED) {
    return;
  }

  const uint64_t rxBytes = m_device->rxBytes();
  logger.debug() << "WireGuard handshake traffic check rx:" << rxBytes
                 << "baseline:" << m_activationRxBytes;
  if (!NetmgrUtils::hasNewHandshakeTraffic(m_activationRxBytes, rxBytes)) {
    return;
  }

  if (!ipv4DefaultRouteReady()) {
    return;
  }

  m_handshakePollTimer.stop();
  m_waitingForHandshakeTraffic = false;
  emit connected(m_serverPublicKey);
}

bool NetmgrController::ipv4DefaultRouteReady() const {
  if (!m_requiresIpv4DefaultRoute) {
    return true;
  }

  QFile routeTable("/proc/net/route");
  if (!routeTable.open(QIODevice::ReadOnly | QIODevice::Text)) {
    logger.warning() << "Unable to validate IPv4 route for WireGuard";
    return false;
  }

  const QString contents = QString::fromLocal8Bit(routeTable.readAll());
  if (!NetmgrUtils::mainRouteBlocksInterface(contents, "1.1.1.1",
                                             WG_INTERFACE_NAME)) {
    return true;
  }

  logger.warning() << "WireGuard received traffic, but IPv4 default traffic is"
                   << "still routed through another interface";
  return false;
}

// static
uint64_t NetmgrController::readSysfsFile(const QString& path) {
  QFile file(path);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return 0;
  }
  QString line = QString::fromLocal8Bit(file.readLine());
  file.close();

  return line.toULongLong();
}

void NetmgrController::checkStatus() {
  QString txPath =
      QString("/sys/class/net/%1/statistics/tx_bytes").arg(WG_INTERFACE_NAME);
  QString rxPath =
      QString("/sys/class/net/%1/statistics/rx_bytes").arg(WG_INTERFACE_NAME);

  ControllerStatus st;
  st.m_connected = true;
  st.m_timestamp = guessUptime();
  st.m_ipv4Gateway = m_serverIpv4Gateway;
  st.m_ipv6Gateway = m_serverIpv6Gateway;
  st.m_ipv4Address = m_deviceIpv4Address;
  st.m_ipv6Address = m_deviceIpv6Address;
  st.m_rxBytes = readSysfsFile(rxPath);
  st.m_txBytes = readSysfsFile(txPath);

  emit statusUpdated(st);
}

QDateTime NetmgrController::guessUptime() {
  QString devPath = QString("/sys/class/net/%1").arg(WG_INTERFACE_NAME);
  struct stat st;
  if (lstat(qPrintable(devPath), &st) != 0) {
    return QDateTime();
  } else {
    logger.debug() << "got st_mtime:" << st.st_mtime;
    return QDateTime::fromSecsSinceEpoch(st.st_mtime);
  }
}
