import QtQuick 2.0
import QtQuick.Window 2.0
import StartWalletBridge 1.0

Item {
    id: element
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    StartWalletBridge {
        id: startWallet
    }

    Image {
        id: image_logo
        width: dp(60)
        height: dp(30)
        fillMode: Image.PreserveAspectFit
        source: "../img/TBLogo@2x.svg"
        anchors.top: parent.top
        anchors.topMargin: dp(80)
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Text {
        id: label_mwc
        color: "#ffffff"
        text: qsTr("mwc")
        font.bold: true
        anchors.top: image_logo.bottom
        anchors.topMargin: dp(14)
        anchors.horizontalCenter: image_logo.horizontalCenter
        font.pixelSize: dp(16)
    }

    Image {
        id: image_new
        width: dp(60)
        height: dp(60)
        anchors.horizontalCenterOffset: dp(-90)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        fillMode: Image.PreserveAspectFit
        source: "../img/NewInstanceBtn_big@2x.svg"
    }

    Text {
        id: label_new
        color: "#ffffff"
        text: qsTr("New Instance")
        anchors.top: image_new.bottom
        anchors.topMargin: dp(15)
        anchors.horizontalCenter: image_new.horizontalCenter
        font.pixelSize: dp(18)
    }

    MouseArea {
        anchors.left: label_new.left
        anchors.top: image_new.top
        width: label_new.width
        height: label_new.height + dp(75)
        onClicked: {
            startWallet.createNewWalletInstance("", false);
        }
    }

    Image {
        id: image_restore
        width: dp(60)
        height: dp(60)
        anchors.horizontalCenterOffset: dp(90)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        fillMode: Image.PreserveAspectFit
        source: "../img/RestoreBtn_big@2x.svg"
    }

    Text {
        id: label_restore
        color: "#ffffff"
        text: qsTr("Restore Instance")
        anchors.top: image_restore.bottom
        anchors.topMargin: dp(15)
        anchors.horizontalCenter: image_restore.horizontalCenter
        font.pixelSize: dp(18)
    }

    MouseArea {
        anchors.left: label_restore.left
        anchors.top: image_restore.top
        width: label_restore.width
        height: label_restore.height + dp(75)
        onClicked: {
            startWallet.createNewWalletInstance("", true);
        }
    }

    Image {
        id: image_help
        width: dp(30)
        height: dp(30)
        anchors.horizontalCenterOffset: dp(-40)
        anchors.bottomMargin: dp(90)
        fillMode: Image.PreserveAspectFit
        source: "../img/HelpBtn@2x.svg"
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Text {
        id: text_help
        text: qsTr("Help")
        anchors.leftMargin: dp(20)
        color: "white"
        anchors.left: image_help.right
        anchors.verticalCenter: image_help.verticalCenter
        font.pixelSize: dp(18)
    }

    MouseArea {
        anchors.left: image_help.left
        anchors.top: image_help.top
        height: dp(30)
        width: text_help.width + dp(50)
        onClicked: {
            console.log("help instance clicked")
        }
    }
}
