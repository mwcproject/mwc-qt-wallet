import QtQuick 2.0
import QtQuick.Window 2.0
import StartWalletBridge 1.0
import ConfigBridge 1.0
import "./models"

Item {
    id: element

    StartWalletBridge {
        id: startWallet
    }

    ConfigBridge {
        id: config
    }

      ImageColor {
        id: image_logo
        img_width: dp(105)
        img_source: "../img/mwc-logo.svg"
        anchors.top: parent.top
        anchors.topMargin: dp(50)
        anchors.horizontalCenter: parent.horizontalCenter
    }


    ConfirmButton {
        id: button_create
        title: "Create Wallet"
        anchors.bottomMargin: dp(20)
        anchors.bottom: button_restore.top
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: {
            startWallet.createNewWalletInstance("", false);
        }
    }
    ConfirmButton {
        id: button_restore
        title: "Restore"
        anchors.bottomMargin: dp(40)
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: {
            startWallet.createNewWalletInstance("", true);
        }
    }
    
}
