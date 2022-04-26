import QtQuick 2.0
import QtQuick.Window 2.0
import StartWalletBridge 1.0
import ConfigBridge 1.0

Item {
    id: element

    StartWalletBridge {
        id: startWallet
    }

    ConfigBridge {
        id: config
    }

    Image {
        id: image_logo
        width: dp(200)
        height: dp(30)
        fillMode: Image.PreserveAspectFit
        source: "../img/BigLogo@2x.svg"
        anchors.top: parent.top
        anchors.topMargin: dp(100)
        anchors.horizontalCenter: parent.horizontalCenter
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
        color: Style.titleColor
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

    Text {
        id: text_version
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: dp(15)
        color: "grey"
        font.pixelSize: dp(14)
        text: config.get_APP_NAME() + " v" + config.getBuildVersion()
    }

}
