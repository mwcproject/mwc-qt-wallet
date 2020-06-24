import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.13
import CoreWindowBridge 1.0

Window {
    id: window
    visible: true
    title: qsTr("MWC-Mobile-Wallet")
    
    property int currentState
    property string initParams

    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    CoreWindowBridge {
        id: coreWindow
    }

/*
      CurrentState      Page
      21                Wallet
      22                AccountOptions
      23                Settings
      8                 Send
      9                 Receive
*/

    Connections {
        target: coreWindow
        onSgnUpdateActionStates: {
            currentState = actionState
            navbar.updateTitle(currentState)
        }
    }

    Rectangle
    {
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0
                color: "#9E00E7"
            }

            GradientStop {
                position: 1
                color: "#3600C9"
            }
        }
        anchors.fill: parent
    }

    Inputpassword {
        id: inputpassword
        anchors.fill: parent
        visible: currentState === 3
    }

    Rectangle {
        color: "#00000000"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: dp(60)
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.top: parent.top
        anchors.topMargin: dp(120)

        Wallet {
            id: wallethome
            anchors.fill: parent
            visible: currentState === 21
        }

        AccountOptions {
            id: accountoptions
            anchors.fill: parent
            visible: currentState === 22
        }

        Settings {
            id: walletsettings
            anchors.fill: parent
            visible: currentState === 23
        }

        Send {
            id: sendTab
            anchors.fill: parent
            visible: currentState === 8 && initParams.length === 0
        }

        SendOnline {
            id: sendonline
            anchors.fill: parent
            visible: currentState === 8 && initParams.length !== 0 && JSON.parse(initParams).isSendOnline
            onVisibleChanged: {
                if (visible) {
                    sendonline.init(JSON.parse(initParams))
                }
            }
        }

        SendOffline {
            id: sendoffline
            anchors.fill: parent
            visible: currentState === 8 && initParams.length !== 0 && !JSON.parse(initParams).isSendOnline
            onVisibleChanged: {
                if (visible) {
                    sendoffline.init(JSON.parse(initParams))
                }
            }
        }

        Receive {
            id: receiveTab
            anchors.fill: parent
            visible: currentState === 9
        }
    }

    Navbar {
        id: navbar
        anchors.fill: parent
        visible: currentState > 3
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:14;anchors_height:200}D{i:19;anchors_y:0}
D{i:21;anchors_width:200}
}
##^##*/
