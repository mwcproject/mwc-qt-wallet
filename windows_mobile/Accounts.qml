import QtQuick 2.12
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import WalletBridge 1.0
import ConfigBridge 1.0
import AccountsBridge 1.0
import UtilBridge 1.0

Item {
    property string selectedAccountName

    WalletBridge {
        id: wallet
    }

    AccountsBridge {
        id: accState
    }

    ConfigBridge {
        id: config
    }

    UtilBridge {
        id: util
    }

    Connections {
        target: wallet
        onSgnWalletBalanceUpdated: {
            updateAccountsList()
        }

        onSgnAccountRenamed: (success, errorMessage) => {
            wallet.requestWalletBalanceUpdate()
            if(!success) {
                messagebox.open(qsTr("Account rename failure"), qsTr("Your account wasn't renamed.\n" + errorMessage))
            }
        }

        onSgnAccountCreated: {
            updateAccountsList()
        }
    }

    function updateAccountsList() {
        rect_progress.visible = false
        const data2show = accState.getAccountsBalancesToShow()

        // update the list with accounts
        accountsModel.clear()
        const rowSz = 5;
        for (let r = 0; r < data2show.length - rowSz + 1; r += rowSz) {
            accountsModel.append({
                 account: data2show[r],
                 spendable: data2show[r+1],
                 awaiting: data2show[r+2],
                 locked: data2show[r+3],
                 total: data2show[r+4]
            })
        }
        image_transfer.enabled = accountsModel.count > 1
    }

    function startWaiting() {
        accountsModel.clear()
        rect_progress.visible = true
    }

    function onAddAccount(ok, newAccountName) {
        if (!ok || newAccountName === "")
            return;

        const err = accState.validateNewAccountName(newAccountName)
        if (err !== "") {
            messagebox.open("Wrong account name",  err)
            return;
        }

        startWaiting()
        wallet.createAccount(newAccountName)
    }

    function onRenameAccount(ok, newAccountName) {
        if (!ok || newAccountName === "" || newAccountName === selectedAccountName)
            return;

        const err = accState.validateNewAccountName(newAccountName)
        if (err !== "") {
            messagebox.open("Wrong account name",  err)
            return;
        }

        wallet.renameAccount(selectedAccountName, newAccountName)
        startWaiting()
    }

    function onDeleteAccount(ok) {
        if (ok) {
            accState.deleteAccount(selectedAccountName)
            startWaiting()
        }
    }

    onVisibleChanged: {
        if (visible) {
            rect_progress.visible = false
            updateAccountsList()
        }
    }

    ListModel {
        id: accountsModel
    }

    ListView {
        id: accountsList
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: rect_buttons.bottom
        anchors.topMargin: dp(15)
        model: accountsModel
        delegate: accountsDelegate
        focus: true
    }

    Component {
        id: accountsDelegate
        Rectangle {
            height: dp(180)
            color: "#00000000"
            anchors.left: parent.left
            anchors.right: parent.right

            Rectangle {
                height: dp(170)
                color: "#33bf84ff"
                anchors.top: parent.top
                anchors.topMargin: dp(15)
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                anchors.left: parent.left
                anchors.leftMargin: dp(20)

                Text {
                    color: "#ffffff"
                    text: account
                    font.bold: true
                    font.pixelSize: dp(15)
                    anchors.top: parent.top
                    anchors.topMargin: dp(20)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                }

                Text {
                    width: dp(200)
                    height: dp(15)
                    text: "Total: " + total
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignRight
                    font.pixelSize: dp(15)
                    anchors.top: parent.top
                    anchors.topMargin: dp(20)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(35)
                }

                Rectangle {
                    height: dp(1)
                    color: "#ffffff"
                    anchors.top: parent.top
                    anchors.topMargin: dp(55)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(35)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                }

                Text {
                    color: "#ffffff"
                    text: "Spendable: " + spendable
                    anchors.top: parent.top
                    anchors.topMargin: dp(70)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    font.pixelSize: dp(15)
                }

                Text {
                    color: "#ffffff"
                    text: "Awaiting: " + awaiting
                    anchors.top: parent.top
                    anchors.topMargin: dp(100)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    font.pixelSize: dp(15)
                }

                Text {
                    color: "#ffffff"
                    text: "Locked: " + locked
                    anchors.top: parent.top
                    anchors.topMargin: dp(130)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    font.pixelSize: dp(15)
                }

                Image {
                    id: image_rename
                    anchors.right: parent.right
                    anchors.rightMargin: image_remove.visible ? dp(90) : dp(35)
                    anchors.top: parent.top
                    anchors.topMargin: dp(100)
                    width: dp(45)
                    height: dp(45)
                    fillMode: Image.PreserveAspectFit
                    source: "../img/Edit@2x.svg"
                    visible: index > 0

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            selectedAccountName = account
                            inputDlg.open(qsTr("Rename MWC Account"), qsTr("Input a new name for your account " + account), "", account, 32, onRenameAccount)
                        }
                    }
                }

                Image {
                    id: image_remove
                    anchors.right: parent.right
                    anchors.rightMargin: dp(35)
                    anchors.top: parent.top
                    anchors.topMargin: dp(100)
                    width: dp(45)
                    height: dp(45)
                    fillMode: Image.PreserveAspectFit
                    source: "../img/Delete@2x.svg"
                    visible: index > 0 && index < accountsModel.count && accState.canDeleteAccount(account)

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            selectedAccountName = account
                            messagebox.open("Delete account", "Are you sure that you want to delete this account?", true, "No", "Yes", "", "", "", onDeleteAccount)
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: rect_buttons
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: dp(55)
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

        Rectangle {
            width: dp(250)
            height: dp(45)
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#00000000"

            Image {
                id: image_transfer
                anchors.left: parent.left
                anchors.top: parent.top
                width: dp(45)
                height: dp(45)
                fillMode: Image.PreserveAspectFit
                source: "../img/Transfer@2x.svg"

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        accState.doTransferFunds()
                    }
                }
            }

            Image {
                id: image_add
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                width: dp(45)
                height: dp(45)
                fillMode: Image.PreserveAspectFit
                source: "../img/Add@2x.svg"

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        inputDlg.open(qsTr("Add account"), qsTr("Please specify the name of a new account in your wallet"), "account name", "", 32, onAddAccount)
                    }
                }
            }

            Image {
                id: image_refresh
                anchors.right: parent.right
                anchors.top: parent.top
                width: dp(45)
                height: dp(45)
                fillMode: Image.PreserveAspectFit
                source: "../img/Refresh@2x.svg"

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        wallet.requestWalletBalanceUpdate()
                        startWaiting()
                    }
                }
            }
        }
    }

    InputDlg {
        id: inputDlg
        anchors.verticalCenter: parent.verticalCenter
    }

    Rectangle {
        id: rect_progress
        width: dp(60)
        height: dp(30)
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#00000000"
        visible: false
        AnimatedImage {
            id: animation
            source: "../img/loading.gif"
        }
    }

    MessageBox {
        id: messagebox
        anchors.verticalCenter: parent.verticalCenter
    }
}
