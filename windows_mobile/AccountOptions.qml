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
        id: button_accounts
        height: dp(60)
        anchors.bottom: button_show_passphrase.top
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
                text: qsTr("id-account")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }
        onClicked: {
            stateMachine.setActionWindow(4)    // Accounts Page
        }
    }

    Button {
        id: button_show_passphrase
        height: dp(60)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: dp(-40)
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
                text: qsTr("id-show-passphrase")
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
                    messagebox.open(qsTr("id-wallet-password"), qsTr("id-mnemonic-warning"),
                            true, qsTr("id-cancel"), qsTr("id-confirm"), passwordHash, 1, "", showPassphraseCallback)
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
        anchors.verticalCenterOffset: dp(40)
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("id-contacts")
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
                text: qsTr("id-logout")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }
        onClicked: {
            messagebox.open(qsTr("id-change-wallet"), qsTr("id-change-wallet-warning"), true, qsTr("id-no"), qsTr("id-yes"), "", "", "", changeInstanceCallback)
        }
    }
}
