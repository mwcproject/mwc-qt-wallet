import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import StateMachineBridge 1.0
import WalletBridge 1.0

Item {
    property string passwordHash

    StateMachineBridge {
        id: stateMachine
    }

    WalletBridge {
        id: wallet
    }

    function showPassphraseCallback(ret, password) {
        if (ret) {
            stateMachine.activateShowSeed(password)
        }
    }

    function changeInstanceCallback(ret) {
        if (ret) {
            stateMachine.logout()
        }
    }

    Button {
        id: button_show_passphrase
        height: dp(60)
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
            // state::STATE::SHOW_SEED = 16
            if (stateMachine.canSwitchState(16)) {
                passwordHash = wallet.getPasswordHash()

                if (passwordHash !== "") {
                    messagebox.open("Wallet Password", "You are going to view wallet mnemonic passphrase.\n\nPlease input your wallet password to continue",
                            true, "Cancel", "Confirm", passwordHash, 1, "", showPassphraseCallback)
                    return;
                }
                // passwordHash should contain raw password value form the messgage box
                stateMachine.activateShowSeed(passwordHash)
            }
        }
    }

    Button {
        id: button_list_contacts
        height: dp(60)
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
            stateMachine.setActionWindow(13)    // Contacts Page
        }
    }

    Button {
        id: button_log_out
        height: dp(60)
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
            messagebox.open(qsTr("Change Instance"), qsTr("Changing an instance will log you out of this current wallet instance. Are you sure you want to log out?"), true, "No", "Yes", "", "", "", changeInstanceCallback)
        }
    }
}
