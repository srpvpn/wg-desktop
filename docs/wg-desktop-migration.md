# WG Desktop Migration Notes

## Architecture Summary

WG Desktop keeps the Mozilla VPN Linux networking stack and replaces the Mozilla service-facing app layer with local WireGuard profiles.

The preserved path is:

1. QML UI selects/imports a local profile.
2. `WireGuardProfileModel` parses the selected `.conf` into `InterfaceConfig`.
3. `TaskControllerAction` asks `Controller` to activate that `InterfaceConfig`.
4. Existing Linux implementations receive the same `InterfaceConfig`:
   - non-Flatpak Linux: `LinuxController` -> D-Bus daemon -> `Daemon` -> `WireguardUtilsLinux`
   - Flatpak: `NetmgrController` -> NetworkManager WireGuard connection
5. Existing disconnect, state machine, split-tunnel settings, kill switch, daemon status, and backend log plumbing remain in place.

Internal Linux identifiers are intentionally unchanged in this phase:

- executable: `mozillavpn`
- Flatpak app-id: `org.mozilla.vpn`
- D-Bus service: `org.mozilla.vpn.dbus`
- polkit actions: `org.mozilla.vpn.activate`, `org.mozilla.vpn.deactivate`

This minimizes risk to the existing Linux privilege, daemon, and Flatpak integration.

## Removed Or Bypassed Mozilla Dependencies

Current migration code bypasses these mandatory backend paths:

- startup token gate in `MozillaVPN::initialize()`
- Mozilla account/user/device/server model requirement in `modelsInitialized()`
- periodic account/server/subscription/heartbeat/add-on refresh scheduling
- pre-activation Mozilla account check in the local-profile activation path
- first-run login/authentication UI
- Mozilla server selection UI
- direct authentication invocations (`authenticate()` / `authenticateWithType()`)
- support ticket creation
- subscription start/restore handlers
- visible in-app messaging navigation
- visible account/device/sign-out settings entries
- visible Get Help/support entry in Settings
- visible updater check button in About
- visible Mozilla Terms/Privacy service links in About
- Linux navigator registration for authentication, device-limit/removal, billing, and subscription screens
- Linux subscription error-handler navigation routes
- Linux CLI registration for `login`, `logout`, `servers`, `select`, `wgconf`, and `device`
- `activate`, `deactivate`, and `excludeip` CLI token/account gates
- `activate` CLI now uses the active local WireGuard profile instead of Mozilla `ServerData`
- start-at-boot UI token gate now checks for a configured local WireGuard profile
- `status` CLI account/subscription/device/server reporting and account refresh
- Mozilla account/server/subscription/auth/device/support/purchase/add-on/product task network requests
- Mozilla feature-list, heartbeat, IP-info, Sentry config/report, and hosted update checks
- release/update tasks that would otherwise reach Balrog/version endpoints
- hosted version API updater checks
- browser-based web subscription/authentication flow
- Linux subscription product loading/scheduling
- legacy pre-activation account/subscription check in `Controller::activate(ServerData, ...)`
- telemetry/Sentry enablement and the visible data-collection toggle
- Mozilla/FxA/SUMO/update URL opener labels
- default Linux `UrlOpener` FxA labels before app-level label registration
- direct Mozilla/SUMO/Add-ons HTTPS opens in settings QML
- extension activation now uses the active local profile when profiles exist
- native messaging server inventory and open-auth commands are inert for local-profile installs
- settings-triggered server switching is skipped for local profiles instead of touching Mozilla server data

Code for non-Linux and legacy Mozilla flows still exists in the tree, but the Linux standalone path no longer requires a Mozilla account to reach the main screen or activate a local profile.

## Modified Files

- `CMakeLists.txt`
- `.gitignore`
- `linux/extra/org.mozilla.vpn.desktop.sh`
- `linux/extra/org.mozilla.vpn.metainfo.sh`
- `src/cmake/sources.cmake`
- `src/controller.cpp`
- `src/controller.h`
- `src/commands/commandactivate.cpp`
- `src/commands/commanddeactivate.cpp`
- `src/commands/commanddevice.cpp`
- `src/commands/commandexcludeip.cpp`
- `src/commands/commandlogin.cpp`
- `src/commands/commandlogout.cpp`
- `src/commands/commandselect.cpp`
- `src/commands/commandservers.cpp`
- `src/commands/commandstatus.cpp`
- `src/commands/commandui.cpp`
- `src/commands/commandwgconf.cpp`
- `src/main.cpp`
- `src/models/wireguardprofilemodel.cpp`
- `src/models/wireguardprofilemodel.h`
- `src/mozillavpn.cpp`
- `src/mozillavpn.h`
- `src/mozillavpn_p.cpp`
- `src/mozillavpn_p.h`
- `src/platforms/linux/netmgrcontroller.cpp`
- `src/platforms/linux/netmgrcontroller.h`
- `src/productshandler.cpp`
- `src/purchasewebhandler.cpp`
- `src/releasemonitor.cpp`
- `src/settingslist.h`
- `src/settingswatcher.cpp`
- `src/systemtraynotificationhandler.cpp`
- `src/feature/features.h`
- `src/feature/taskgetfeaturelist.cpp`
- `src/tasks/account/taskaccount.cpp`
- `src/tasks/adddevice/taskadddevice.cpp`
- `src/tasks/addon/taskaddon.cpp`
- `src/tasks/addonindex/taskaddonindex.cpp`
- `src/tasks/authenticate/taskauthenticate.cpp`
- `src/tasks/captiveportallookup/taskcaptiveportallookup.cpp`
- `src/tasks/controlleraction/taskcontrolleraction.cpp`
- `src/tasks/createsupportticket/taskcreatesupportticket.cpp`
- `src/tasks/getlocation/taskgetlocation.cpp`
- `src/tasks/getsubscriptiondetails/taskgetsubscriptiondetails.cpp`
- `src/tasks/heartbeat/taskheartbeat.cpp`
- `src/tasks/ipfinder/taskipfinder.cpp`
- `src/tasks/products/taskproducts.cpp`
- `src/tasks/purchase/taskpurchase.cpp`
- `src/tasks/removedevice/taskremovedevice.cpp`
- `src/tasks/release/taskrelease.cpp`
- `src/tasks/sentry/tasksentry.cpp`
- `src/tasks/sentryconfig/tasksentryconfig.cpp`
- `src/tasks/servers/taskservers.cpp`
- `src/update/versionapi.cpp`
- `src/urlopener.cpp`
- `src/webextensionadapter.cpp`
- `src/ui/CMakeLists.txt`
- `src/ui/main.qml`
- `src/ui/screens/getHelp/ViewGetHelp.qml`
- `src/ui/screens/home/ViewHome.qml`
- `src/ui/screens/home/ViewServers.qml`
- `src/ui/screens/initialize/ViewInitialize.qml`
- `src/ui/screens/settings/ViewAboutUs.qml`
- `src/ui/screens/settings/ViewFirefoxExtensionInfo.qml`
- `src/ui/screens/settings/ViewPreferences.qml`
- `src/ui/screens/settings/ViewSettingsMenu.qml`
- `src/ui/sharedViews/ViewErrorFullScreen.qml`
- `src/ui/singletons/VPNWireGuardProfileModel.h`
- `src/utils/CMakeLists.txt`
- `src/utils/wireguardconfig.cpp`
- `src/utils/wireguardconfig.h`
- `tests/unit/CMakeLists.txt`
- `tests/unit/testwireguardconfig.cpp`
- `tests/unit/testwireguardconfig.h`
- `tests/unit/testwireguardprofilemodel.cpp`
- `tests/unit/testwireguardprofilemodel.h`

## Local Profile Behavior

Supported now:

- import local `.conf` files
- create a profile from pasted WireGuard config text
- store multiple profiles in local encrypted settings
- skip invalid/corrupt stored profiles on load
- list profiles
- select active profile
- rename profile
- remove profile
- expose a QML-safe `profileId` role for profile list actions
- return to the local import flow when the last local profile is removed

The parser supports:

- `[Interface] PrivateKey`
- `[Interface] Address`
- `[Interface] DNS`
- `[Peer] PublicKey`
- `[Peer] Endpoint`
- `[Peer] AllowedIPs`

Unsupported WireGuard keys are ignored rather than rejected.
Endpoint values may be IPv4, bracketed IPv6, or hostnames.

## Host Build Instructions

Initialize required submodules first:

```bash
git submodule update --init --recursive
```

Then configure and build:

```bash
cmake -S . -B build -DBUILD_TESTS=ON -DBUILD_CRASHREPORTING=OFF
cmake --build build --target mozillavpn
```

Host builds require Qt 6.9 or newer. The current host image has Qt 6.8.3, so host CMake configure stops before project build targets unless a newer Qt is provided.

Run unit tests:

```bash
cmake --build build --target unit_tests
ctest --test-dir build -R 'TestWireGuard(Config|ProfileModel)' --output-on-failure
```

## Flatpak Build Instructions

Initialize submodules first:

```bash
git submodule update --init --recursive
```

Build with the existing Flatpak manifest:

```bash
flatpak-builder --force-clean --user --install build-flatpak linux/flatpak/org.mozilla.vpn.yml
```

Run:

```bash
flatpak run org.mozilla.vpn
```

The manifest app-id remains `org.mozilla.vpn` in this phase to preserve existing sandbox and NetworkManager behavior.

## Verification Performed

Verified locally:

- `git submodule update --init --recursive`
- `git diff --check`
- direct compile/run check for `WireGuardConfig::parse`
- direct syntax compile for `src/models/wireguardprofilemodel.cpp`
- direct syntax compile for `src/settingsholder.cpp`
- direct syntax compile for `src/controller.cpp`
- direct syntax compile for `src/tasks/controlleraction/taskcontrolleraction.cpp`
- direct syntax compile for `src/settingswatcher.cpp`
- direct syntax compile for `src/platforms/linux/netmgrcontroller.cpp` with `MZ_LINUX` and `MZ_FLATPAK`
- direct syntax compile for `src/mozillavpn.cpp`
- direct syntax compile for `src/mozillavpn.cpp` after Linux-only navigation isolation
- direct syntax compile for `src/mozillavpn.cpp` after local profile model state-change handling
- direct syntax compile for `src/mozillavpn_p.cpp`
- direct syntax compile for `src/systemtraynotificationhandler.cpp`
- direct syntax compile for `src/commands/commandactivate.cpp`
- direct syntax compile for `src/commands/commanddeactivate.cpp`
- direct syntax compile for `src/commands/commandexcludeip.cpp`
- direct syntax compile for `src/commands/commandui.cpp`
- direct syntax compile for `src/commands/commandstatus.cpp`
- direct syntax compile for Linux-disabled command registrations in `commandlogin`, `commandlogout`, `commandservers`, `commandselect`, `commandwgconf`, and `commanddevice`
- direct syntax compile for `src/webextensionadapter.cpp`
- direct syntax compile for offline-disabled backend tasks: account, add-device, remove-device, authenticate, servers, heartbeat, subscription details, support ticket, purchase, products, add-on index/download, feature list, captive portal lookup, location lookup, IP finder, Sentry config, and Sentry sender
- direct syntax compile for offline-disabled release/update task
- direct syntax compile for `src/update/versionapi.cpp`
- direct syntax compile for `src/productshandler.cpp`
- direct syntax compile for `src/purchasewebhandler.cpp`
- direct syntax compile for `src/releasemonitor.cpp`
- direct syntax compile for `src/urlopener.cpp`
- direct syntax compile for `src/mozillavpn.cpp`, `src/urlopener.cpp`, and `src/tasks/release/taskrelease.cpp` after hiding support/update URL paths
- direct syntax compile for `tests/unit/testwireguardconfig.cpp` after adding hostname and IPv6 endpoint cases
- direct syntax compile for `tests/unit/testwireguardprofilemodel.cpp` after adding invalid stored profile coverage
- direct syntax compile for `tests/unit/testwireguardprofilemodel.cpp` after adding `profileId` role coverage
- targeted QML/source scan confirms profile list actions use `profileId` instead of the ambiguous `id` role
- targeted QML/source scan confirms new `MZTextField` placeholders use Nebula's `_placeholderText` API
- standalone parser runtime check for hostname and bracketed IPv6 endpoint configs
- source registration check confirms new parser, profile model, QML singleton, and unit test files are listed in the relevant CMake/QML module files
- direct syntax compile for `src/ui/singletons/VPNWireGuardProfileModel.h`
- targeted source scan found no remaining `request->auth()` or direct account/server/heartbeat/subscription/support/device/feature-list API request calls outside intentionally retained constants or non-Linux-only UI text
- targeted source scan found no direct QML `MZUrlOpener.openUrl("https://...")` calls under `src/ui`
- targeted source scan found no `NetworkRequest` creation or `request->auth/get/post/deleteResource` calls under standalone-disabled task/release/controller/url-opener paths
- Linux-preprocessed scan confirms auth/subscription/device-removal QML screen paths are not registered from `src/mozillavpn.cpp`
- host CMake configure reaches Qt discovery but stops because the host has Qt 6.8.3 while the project requires Qt 6.9+
- Flatpak SDK CMake configure for `flatpak-unit-build` succeeds with Qt 6.10.3
- `cmake --build flatpak-unit-build --target unit_tests -j2` succeeds inside `flatpak-builder --run` with local test-only PyYAML `PYTHONPATH`
- `ctest --test-dir flatpak-unit-build -R 'TestWireGuard(Config|ProfileModel)' --output-on-failure` passes 2/2 tests inside `flatpak-builder --run`
- Flatpak build with `org.kde.Platform//6.10` produced `build-flatpak/files/bin/mozillavpn`
- Flatpak build output includes `build-flatpak/files/bin/socksproxy`
- Flatpak build output includes `build-flatpak/files/share/applications/org.mozilla.vpn.desktop`
- Flatpak build output includes `build-flatpak/files/share/metainfo/org.mozilla.vpn.metainfo.xml`
- Flatpak build from the current source tree completed and installed `app/org.mozilla.vpn/x86_64/master`
- installed Flatpak metadata reports `WG Desktop - A standalone WireGuard desktop client`
- installed Flatpak commit: `b03a395c54a519410543af1506a1b55703ba0d3ad95575ea84cfef48ad390723`
- `flatpak run org.mozilla.vpn --help` succeeds and exposes only standalone Linux commands: `activate`, `deactivate`, `excludeip`, `status`, `ui`, `mockdaemon`, and `webext`
- generated WG Desktop desktop entry passes `desktop-file-validate` with only a non-fatal category hint
- generated WG Desktop metainfo passes `appstreamcli validate --no-net`
- generated WG Desktop desktop/metainfo source outputs no longer contain Mozilla localized product names, Mozilla support links, Mozilla release metadata URLs, or mandatory internet requirements

## Remaining TODO Items

- Runtime-test non-Flatpak Linux activation with a real `.conf`.
- Runtime-test Flatpak activation through NetworkManager with a real `.conf`.
- Remove or disable non-Linux platform build paths if they interfere with Linux-only packaging.
- Replace remaining visible Mozilla strings/icons where they appear in settings/about/help screens.
- Decide whether to rename internal Linux service IDs in a later, higher-risk phase.
