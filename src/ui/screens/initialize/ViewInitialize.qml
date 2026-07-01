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

MZFlickable {
    id: root

    objectName: "viewInitialize"
    flickContentHeight: content.height + (MZTheme.theme.windowMargin * 2)

    FileDialog {
        id: importDialog
        title: "Import WireGuard configuration"
        nameFilters: ["WireGuard configs (*.conf)", "All files (*)"]
        onAccepted: {
            if (VPNWireGuardProfileModel.addProfileFromFile(profileName.text, selectedFile)) {
                VPN.onboardingCompleted();
            }
        }
    }

    ColumnLayout {
        id: content

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: MZTheme.theme.windowMargin
        }

        spacing: MZTheme.theme.windowMargin

        MZHeadline {
            Layout.fillWidth: true
            text: "Import WireGuard configuration"
        }

        MZSubtitle {
            Layout.fillWidth: true
            text: "WG Desktop stores WireGuard profiles locally. No Mozilla account, registration, or service connection is required."
        }

        MZTextField {
            id: profileName
            Layout.fillWidth: true
            _placeholderText: "Profile name"
        }

        MZButton {
            Layout.fillWidth: true
            text: "Import .conf file"
            onClicked: importDialog.open()
        }

        MZSubtitle {
            Layout.fillWidth: true
            text: "Or paste WireGuard config text"
        }

        TextArea {
            id: pastedConfig
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(260, root.height * 0.35)
            placeholderText: "[Interface]\nPrivateKey = ...\nAddress = ...\n\n[Peer]\nPublicKey = ...\nEndpoint = ...\nAllowedIPs = ..."
            textFormat: TextEdit.PlainText
            wrapMode: TextEdit.Wrap
        }

        MZButton {
            Layout.fillWidth: true
            text: "Add pasted profile"
            onClicked: {
                if (VPNWireGuardProfileModel.addProfileFromText(profileName.text, pastedConfig.text)) {
                    VPN.onboardingCompleted();
                }
            }
        }

        MZSubtitle {
            Layout.fillWidth: true
            color: MZTheme.colors.errorAccent
            text: VPNWireGuardProfileModel.lastError
            visible: text.length > 0
        }
    }
}
