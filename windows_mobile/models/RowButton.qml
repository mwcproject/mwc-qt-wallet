import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import UtilBridge 1.0
import QtGraphicalEffects 1.0


Rectangle {
    id: rect_network
    anchors.top: button_login.bottom
    anchors.topMargin: dp(80)
    anchors.horizontalCenter: parent.horizontalCenter
    radius: dp(50)
    color: "#00000000"
    height: dp(40)
    width: parent.width/1.5

    Button {
        id: mainnetButton
        height: parent.height - dp(10)
        width: parent.width/2 - dp(15)
        anchors.rightMargin: dp(20)
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left

        background: Rectangle {
            id: mainnetRectangle
            color: "#111111"
            radius: dp(10)
            Text {
                id: mainnetText
                text: qsTr("Create")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.letterSpacing: dp(2)
                font.pixelSize: dp(16)
                color: "white"
            }
        }
        onClicked: {
           startWallet.createNewWalletInstance("", false);
        }
    }

    Button {
        id: testnetButton
        height: parent.height -dp(10)
        width: parent.width/2 - dp(15)
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        background: Rectangle {
            id: testnetRectangle
            color: "#111111"
            radius: dp(10)
            Text {
                id: testnetText
                text: qsTr("Restore")
                font.letterSpacing: dp(2)
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(16)
                color: "white"
            }
        }
        onClicked: {
            startWallet.createNewWalletInstance("", true);
        }
    }
}



