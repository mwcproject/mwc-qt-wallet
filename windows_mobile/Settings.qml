import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import StateMachineBridge 1.0

Item {
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    StateMachineBridge {
        id: stateMachine
    }

    function resyncCallback(ret) {
        if (ret) {
            stateMachine.activateResyncState()
        }
    }

    Button {
        id: button_wallet_configuration
        height: dp(72)
        anchors.bottom: button_listeners.top
        anchors.bottomMargin: dp(23)
        anchors.right: parent.right
        anchors.rightMargin: dp(75)
        anchors.left: parent.left
        anchors.leftMargin: dp(75)
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Wallet Configuration")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }
        onClicked: {
            stateMachine.setActionWindow(14)    // Wallet Configuration Page
        }
    }

    Button {
        id: button_listeners
        height: dp(72)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: dp(-48)
        anchors.right: parent.right
        anchors.rightMargin: dp(75)
        anchors.left: parent.left
        anchors.leftMargin: dp(75)
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Listeners")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }
        onClicked: {
            stateMachine.setActionWindow(10)    // Wallet Listeners Page
        }
    }

    Button {
        id: button_node_overview
        height: dp(72)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: dp(48)
        anchors.right: parent.right
        anchors.rightMargin: dp(75)
        anchors.left: parent.left
        anchors.leftMargin: dp(75)
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Node Overview")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }
        onClicked: {
            stateMachine.setActionWindow(17)    // NodeInfo Page
        }
    }

    Button {
        id: button_resync
        height: dp(72)
        anchors.top: button_node_overview.bottom
        anchors.topMargin: dp(23)
        anchors.right: parent.right
        anchors.rightMargin: dp(75)
        anchors.left: parent.left
        anchors.leftMargin: dp(75)
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Re-sync with Full Node")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }
        onClicked: {
            messagebox.open("Re-sync account with a node", "Account re-sync will validate transactions and outputs for your accounts. Re-sync can take several minutes.\nWould you like to continue?",
                                        true, "No", "Yes", "", "", "", resyncCallback)
        }
    }
}
