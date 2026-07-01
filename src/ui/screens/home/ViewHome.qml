/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZFlickable {
    id: vpnFlickable

    objectName: "viewMainFlickable"

    flickContentHeight: col.height + col.anchors.topMargin
    anchors.left: parent.left
    anchors.right: parent.right

    states: [
        State {
            when: window.fullscreenRequired()
            PropertyChanges {
                target: mobileHeader
                visible: true
            }
        },
        State {
            when: !window.fullscreenRequired()
            PropertyChanges {
                target: mobileHeader
                visible: false
            }
        }
    ]

    GridLayout {
        id: col

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: MZTheme.theme.windowMargin / 2
        anchors.rightMargin: MZTheme.theme.windowMargin / 2
        flow: GridLayout.TopToBottom
        rowSpacing: MZTheme.theme.windowMargin
        anchors.top: parent.top
        anchors.topMargin: MZTheme.theme.windowMargin

        RowLayout {
            id: mobileHeader
            Layout.preferredHeight: MZTheme.theme.rowHeight
            Layout.alignment: Qt.AlignHCenter
            spacing: 6

            MZIcon {
                source: MZAssetLookup.getImageSource("Logo")
                sourceSize.height: 20
                sourceSize.width: 20
                antialiasing: true
                Layout.alignment: Qt.AlignVCenter
            }

            MZBoldLabel {
                text: "WG Desktop"
                color: MZTheme.colors.fontTitleColor
                Layout.alignment: Qt.AlignVCenter
            }
        }

        ControllerView {
            id: box
            z: 1
        }

        ControllerNav {
            function handleClick() {
                if (disableRowWhen) {
                    return
                }
                window.showServerList(false)
            }

            id: serverInfo

            objectName: "serverListButton"
            btnObjectName: "serverListButton-btn"

            titleText: "Active profile"

            descriptionText: "current profile - " + VPNWireGuardProfileModel.activeProfileName

            disableRowWhen: (VPNController.state !== VPNController.StateOn &&
                            VPNController.state !== VPNController.StateOnPartial &&
                            VPNController.state !== VPNController.StateOff &&
                            VPNController.state !== VPNController.StateConnectionError)
            Layout.topMargin: 12
            contentChildren: [

                MZBoldLabel {
                    objectName: "profileListButton-label"
                    Layout.fillWidth: true
                    text: VPNWireGuardProfileModel.activeProfileName
                }
            ]
        }
    }
}
