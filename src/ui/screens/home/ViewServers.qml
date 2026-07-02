/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Dialogs
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

Item {
    id: root
    objectName: "viewServers"

    FileDialog {
        id: importDialog
        title: "Import WireGuard configuration"
        nameFilters: ["WireGuard configs (*.conf)", "All files (*)"]
        onAccepted: VPNWireGuardProfileModel.addProfileFromFile(newProfileName.text, selectedFile)
    }

    MZMenu {
        id: menu
        objectName: "profileListBackButton"
        title: "WireGuard profiles"
        _menuOnBackClicked: () => stackview.pop()
    }

    MZFlickable {
        anchors {
            top: menu.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            margins: MZTheme.theme.windowMargin
        }
        flickContentHeight: content.height

        ColumnLayout {
            id: content
            width: parent.width
            spacing: MZTheme.theme.windowMargin

            MZHeadline {
                Layout.fillWidth: true
                text: "Profiles"
            }

            Repeater {
                model: VPNWireGuardProfileModel

                delegate: Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: profileLayout.implicitHeight + MZTheme.theme.windowMargin
                    color: active ? MZTheme.colors.dividerColor : "transparent"
                    radius: 8

                    ColumnLayout {
                        id: profileLayout
                        anchors {
                            fill: parent
                            margins: MZTheme.theme.windowMargin / 2
                        }
                        spacing: MZTheme.theme.windowMargin / 2

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: MZTheme.theme.windowMargin / 2

                            MZTextField {
                                id: renameInput
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                                text: name
                            }

                            Rectangle {
                                Layout.preferredHeight: MZTheme.theme.rowHeight
                                Layout.preferredWidth: activeStatus.implicitWidth + MZTheme.theme.windowMargin
                                color: MZTheme.colors.vpnToggleConnected.defaultColor
                                radius: 6
                                visible: active

                                MZSubtitle {
                                    id: activeStatus
                                    anchors.centerIn: parent
                                    color: MZTheme.colors.normalButton.fontColor
                                    font.family: MZTheme.theme.fontBoldFamily
                                    text: "Active"
                                }
                            }

                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: MZTheme.theme.windowMargin / 2

                            MZButton {
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                                Layout.preferredWidth: 1
                                text: "Select"
                                enabled: !active
                                onClicked: VPNWireGuardProfileModel.selectProfile(profileId)
                            }

                            MZButton {
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                                Layout.preferredWidth: 1
                                text: "Rename"
                                onClicked: VPNWireGuardProfileModel.renameProfile(profileId, renameInput.text)
                            }

                            MZButton {
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                                Layout.preferredWidth: 1
                                buttonType: MZButton.ButtonType.destructive
                                text: "Remove"
                                onClicked: VPNWireGuardProfileModel.removeProfile(profileId)
                            }
                        }
                    }
                }
            }

            MZHeadline {
                Layout.fillWidth: true
                text: "Add profile"
            }

            MZTextField {
                id: newProfileName
                Layout.fillWidth: true
                _placeholderText: "Profile name"
            }

            MZButton {
                Layout.fillWidth: true
                text: "Import .conf file"
                onClicked: importDialog.open()
            }

            TextArea {
                id: pastedConfig
                Layout.fillWidth: true
                Layout.preferredHeight: 220
                placeholderText: "[Interface]\nPrivateKey = ...\nAddress = ...\n\n[Peer]\nPublicKey = ...\nEndpoint = ...\nAllowedIPs = ..."
                textFormat: TextEdit.PlainText
                wrapMode: TextEdit.Wrap
            }

            MZButton {
                Layout.fillWidth: true
                text: "Add pasted profile"
                onClicked: VPNWireGuardProfileModel.addProfileFromText(newProfileName.text, pastedConfig.text)
            }

            MZSubtitle {
                Layout.fillWidth: true
                color: MZTheme.colors.errorAccent
                text: VPNWireGuardProfileModel.lastError
                visible: text.length > 0
            }
        }
    }
}
