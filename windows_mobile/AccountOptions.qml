import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0

Item {
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    Button {
        id: button_show_passphrase
        height: dp(72)
        anchors.bottom: button_list_contacts.top
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
                text: qsTr("Show Passphrase")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }
        onClicked: {
            console.log("Show Passphrase")
        }
    }

    Button {
        id: button_list_contacts
        height: dp(72)
        anchors.right: parent.right
        anchors.rightMargin: dp(75)
        anchors.left: parent.left
        anchors.leftMargin: dp(75)
        anchors.verticalCenter: parent.verticalCenter
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("List Contacts")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }
        onClicked: {
            console.log("List Contacts")
        }
    }

    Button {
        id: button_log_out
        height: dp(72)
        anchors.top: button_list_contacts.bottom
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
                text: qsTr("Log Out")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }
        onClicked: {
            console.log("Log Out")
        }
    }
}
