import QtQuick 2.12
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.15
import WalletBridge 1.0
import StateMachineBridge 1.0
import ConfigBridge 1.0
import AccountsBridge 1.0
import UtilBridge 1.0
import "./models"

Item {
    id: control
    readonly property int status_ignore: 0
    readonly property int status_red: 1
    readonly property int status_yellow: 2
    readonly property int status_green: 3

    // temporary fix for a bug related to init account switch --
    property bool initAccount: false
    // --

    property alias topVisible: navbarTop.visible


    property bool isEdit: false
    property string docName
    property alias open: nav.open

    function updateTitle(state) {
        switch (state) {
            case 4:
                text_title.text = qsTr("Accounts")
                break
            case 5:
                text_title.text = qsTr("AccountTransfer")
                break
            case 6:
                text_title.text = qsTr("Notifications")
                break
            case 8:
                text_title.text = qsTr("Send")
                break
            case 9:
                text_title.text = qsTr("Receive")
                break
            case 10:
                text_title.text = qsTr("Listeners")
                break
            case 11:
                text_title.text = qsTr("Transactions")
                break
            case 12:
                text_title.text = qsTr("Outputs")
                break
            case 13:
                text_title.text = qsTr("Contacts")
                break
            case 14:
                text_title.text = qsTr("Config")
                break
            case 16:
                text_title.text = qsTr("Passphrase")
                break
            case 17:
                text_title.text = qsTr("MWC Node Status (Cloud)")
                break
            case 19:
                text_title.text = qsTr("Finalize")
                break
            case 21:
                text_title.text = qsTr("Wallet")
                break
            case 22:
                text_title.text = qsTr("Account Options")
                break
            case 23:
                text_title.text = qsTr("Settings")
                break
        }
    }

    function changeInstanceCallback(ret) {
        if (ret) {
            stateMachine.logout()
        }
    }



    StateMachineBridge {
        id: stateMachine
    }

    ConfigBridge {
        id: config
    }

    WalletBridge {
        id: wallet
    }

    Connections {
        target: wallet

        onSgnWalletBalanceUpdated: {
            updateAccountsList()
            totalAmount = wallet.getTotalMwcAmount()
        }

        onSgnConfigUpdate: {
            updateNetworkName()
            updateInstanceAccountText()
        }
    }

    onVisibleChanged: {
        if (visible) {
            updateListenerBtn()
            updateNetworkName()
            updateInstanceAccountText()
            totalAmount = wallet.getTotalMwcAmount()
            updateAccountList()
        }
    }


    function updateAccountsList() {
        //rect_progress.visible = false
        const data2show = accState.getAccountsBalancesToShow()

        if (!initAccount) {
            initAccount = true
            wallet.switchAccount("default")
        }
        selectedAccount = wallet.getCurrentAccountName()
        //console.log("select: " + selectedAccount)
        accountsModel.clear()
        const rowSz = 5;
        for (let r = 0; r < data2show.length - rowSz + 1; r += rowSz) {
            console.log("name_ :" + selectedAccount + " " + data2show[r])
            if (String(data2show[r]) == String(selectedAccount)) {

                spendableBalance = data2show[r+1]
                awaitBalance = data2show[r+2]
                lockedBalance = data2show[r+3]
                console.log("locked_ :" + data2show[r+3] + "  " + data2show[r+2])
            }
            accountsModel.append({
                account: data2show[r],
                spendable: data2show[r+1],
                awaiting: data2show[r+2],
                locked: data2show[r+3],
                total: data2show[r+4]
            })
        }






        //image_transfer.enabled = accountsModel.count > 1
    }

    function startWaiting() {
        accountsModel.clear()
        //rect_progress.visible = true
    }

    function onAddAccount(ok, newAccountName) {
        if (!ok || newAccountName === "")
            return;

        const err = accState.validateNewAccountName(newAccountName)
        if (err !== "") {
            messagebox.open("Wrong account name", err)
            return;
        }

        startWaiting()
        wallet.createAccount(newAccountName)
    }

    function onRenameAccount(ok, newAccountName) {
        if (!ok || newAccountName === "" || newAccountName === selectedAccount)
            return;

        const err = accState.validateNewAccountName(newAccountName)
        if (err !== "") {
            messagebox.open("Wrong account name", err)
            return;
        }

        wallet.renameAccount(selectedAccount, newAccountName)
        startWaiting()
    }
width:
    function onDeleteAccount(ok) {
        if (ok) {
            accState.deleteAccount(selectedAccount)
            startWaiting()
        }
    }

    ListModel {
        id: accountsModel
    }


    AccountsBridge {
        id: accState
    }

    UtilBridge {
        id: util
    }

    Rectangle {
        id: navbarTop
        height: parent.height/14
        width: parent.width
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        color: "#00000000"

        RowLayout {
            id: layout
            spacing: 0
            Rectangle {
                id: rec_account
                Layout.fillHeight: true
                Layout.minimumHeight:navbarTop.height
                Layout.minimumWidth: navbarTop.width/5
                color: "#00000000"
                Rectangle {
                    id: rec_acc
                    //color: "#181818"
                    height: (4*parent.height)/5
                    width: control.width
                    radius: dp(50)
                    anchors.left: parent.left
                    anchors.leftMargin: (-4*control.width)/5
                    color: "#181818"

                    ImageColor {
                        id: img_account
                        img_height: parent.height* 0.6
                        img_source: "../../img/account.svg"
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: dp(20)
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            nav.toggle()
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.minimumHeight: navbarTop.height
                Layout.minimumWidth: navbarTop.width*(3/5)
                color: "#00000000"
                Text {
                    id: text_title
                    color: "#ffffff"
                    text: "Dashboard"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    font.bold: true
                    font.pixelSize: dp(18)
                    font.letterSpacing: dp(0.5)
                    font.capitalization:Font.AllUppercase
                    font.family: barlow.medium
                }
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.minimumHeight: navbarTop.height
                Layout.minimumWidth: navbarTop.width/5
                color: "#00000000"
                Rectangle {
                    height: parent.height*0.8
                    width: height
                    color: "#181818"
                    radius: dp(150)
                    anchors.horizontalCenter: parent.horizontalCenter
                    ImageColor {
                        id: logout
                        img_source: "../../img/eye.svg"
                        img_height: parent.height* 0.6
                        img_color: "gray"
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            darkTheme = !darkTheme
                            /*if (darkTheme)
                                util.setBarAndroid(4294967040,4294967040, 16)
                            if (!darkTheme)
                                util.setBarAndroid(4294967040,4294967040, 8)*/
                            console.log("dark Theme ", darkTheme)
                            hiddenAmount = !hiddenAmount
                        }
                    }
                }

            }


        }

    }

    NavigationDrawer {
        id: nav
        visible: true
        //property type name: value
        Rectangle {
            id: drawer
            anchors.fill: parent
            color: "#181818"
        }

        Rectangle {
            anchors.fill: parent
            color: "#00000000"

            ImageColor {
                id: image_logo
                img_height: dp(29)
                img_source: "../../img/mwc-logo.svg"
                img_color: "#ffffff"
                anchors.top: parent.top
                anchors.topMargin: dp(70)
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                id: text_account
                text: qsTr("TOTAL WORTH")
                color: "#c4c4c4"
                font.pixelSize: dp(12)
                font.letterSpacing: dp(0.5)
                anchors.top: image_logo.bottom
                anchors.topMargin: dp(40)
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                id: text_spend_balance
                text: (hiddenAmount? hidden : totalAmount ) + " MWC"
                font.weight: Font.Light
                color: "white"
                font.pixelSize: dp(20)
                //font.letterSpacing: dp(1)
                anchors.top: text_account.bottom
                anchors.topMargin: dp(3)
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                id: text_secondary_currency
                text: (hiddenAmount? hidden : price) + " USD"
                color: "#c4c4c4"
                font.pixelSize: dp(12)
                font.letterSpacing: dp(1)
                anchors.top: text_spend_balance.bottom
                anchors.topMargin: dp(2)
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                id: text_acc_list
                text: qsTr("MY ACCOUNTS")
                color: "#c2c2c2"
                font.pixelSize: dp(15)
                font.letterSpacing: dp(1)
                font.italic: true
                anchors.left: rect_phrase.left
                anchors.leftMargin: dp(15)
                anchors.verticalCenter: rect_edit.verticalCenter
            }

            Rectangle {
                id: rect_edit
                height: text_acc_list.height*2
                width: height
                color: "#353237"
                radius: dp(50)
                anchors.right: rect_phrase.right
                anchors.rightMargin: dp(25)
                anchors.bottom: rect_phrase.top
                anchors.bottomMargin: dp(8)
                visible: true
                ImageColor {
                    id: img_editButton
                    img_height: rect_edit.height/1.7
                    img_source: "../../img/edit.svg"
                    img_color: "#ffffff"
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        isEdit = !isEdit
                    }
                }
            }


            Rectangle {
                id: rect_phrase
                height: parent.height/2.5
                width: parent.width
                color: grid_seed.Layout.minimumHeight > parent.height/2.5? "#141414" : "#00000000"
                anchors.top: text_secondary_currency.bottom
                anchors.topMargin: dp(80)

                Flickable {
                    id: scroll
                    width: parent.width
                    height: parent.height
                    contentHeight: grid_seed.Layout.minimumHeight
                    clip: true
                    ScrollBar.vertical: ScrollBar {
                        policy: Qt.ScrollBarAsNeeded
                    }

                    ColumnLayout {
                        id: grid_seed
                        spacing: 0
                        width: rect_phrase.width

                        Repeater {
                            id: rep
                            model: accountsModel
                            Rectangle {
                                id: rec_acc_balance
                                height: dp(60)
                                width: nav.width
                                color: selectedAccount === account? "#252525" : "#00000000"

                                ImageColor {
                                    id: img_check
                                    img_height: parent.height/2.5
                                    img_source: isEdit && nav.open? "../../img/remove.svg"  : "../../img/check.svg"
                                    img_color: isEdit && nav.open? "#00000000" : "gray"
                                    visible: (isEdit && nav.open)? (account !== "default" && selectedAccount !== account ? true : false) : (selectedAccount === account? true : false)
                                    anchors.left: parent.left
                                    anchors.leftMargin: dp(25)
                                    anchors.verticalCenter: parent.verticalCenter
                                }

                                Text {
                                    id: acc_name
                                    text: "%1\n".arg(account) + (hiddenAmount? hidden : spendable) + " MWC"
                                    color: "gray"
                                    font.pixelSize: dp(15)
                                    font.italic: true
                                    font.weight: Font.Light
                                    anchors.left: img_check.right
                                    anchors.leftMargin: dp(25)
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                ImageColor {
                                    id: img_edit
                                    img_height: parent.height/2.5
                                    img_source: "../../img/check.svg"
                                    img_color: "gray"
                                    visible: (isEdit && nav.open)? (account !== "default" && selectedAccount !== account ? true : false) : false
                                    anchors.right: parent.right
                                    anchors.rightMargin: dp(25)
                                    anchors.verticalCenter: parent.verticalCenter

                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        if (selectedAccount !== account && !isEdit) {
                                            selectedAccount = account
                                            wallet.switchAccount(account)
                                            spendableBalance = spendable
                                            awaitBalance = awaiting
                                            lockedBalance = locked
                                            transactionsItem.requestTransactions()

                                        }
                                    }
                                }
                                MouseArea {
                                    anchors.fill: img_check
                                    onClicked: {
                                        selectedAccount = account
                                        messagebox.open("Delete account", "Are you sure that you want to delete this account?", true, "No", "Yes", "", "", "", onDeleteAccount)
                                    }
                                }
                            }
                        }

                        Rectangle {
                            id: rec_new_acc
                            height: dp(60)
                            width: parent.width
                            //color: "gray"
                            //visible: !isEdit? true : false
                            ImageColor {
                                id: img_new_acc
                                img_height: rec_new_acc.height/2.5
                                img_source: "../../img/plus.svg"
                                img_color: "gray"
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: rec_new_acc.left
                                anchors.leftMargin: dp(25)
                            }
                            Text {
                                id: name_new_acc
                                text: qsTr("Add new account")
                                color: "gray"
                                font.pixelSize: dp(15)
                                font.italic: true
                                font.weight: Font.Light
                                anchors.left: img_new_acc.right
                                anchors.leftMargin: dp(25)
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    inputDlg.open(qsTr("Add account"), qsTr("Please specify the name of a new account in your wallet"), "account name", "", 32, onAddAccount)
                                }
                            }
                        }
                    }
                }
            }

            Rectangle {
                id: navbarButton
                height: dp(65)
                width: parent.width
                anchors.bottom: parent.bottom
                anchors.bottomMargin: dp(20)
                anchors.horizontalCenter: parent.horizontalCenter
                color: "#00000000"

                RowLayout {
                    id: layoutNavBottom
                    spacing: 0
                    Rectangle {
                        color: "#00000000"
                        Layout.fillHeight: true
                        Layout.minimumHeight: parent.parent.height
                        Layout.minimumWidth: parent.parent.width/3
                        Rectangle {
                            id: rec_refresh
                            height: parent.height/1.4
                            width: height
                            color: "#353237"
                            radius: dp(50)
                            anchors.horizontalCenter: parent.horizontalCenter
                            ImageColor {
                                id: img_refresh
                                img_height: rec_refresh.height/1.7
                                img_source:"../../img/arrow.svg"
                                img_color: "#ffffff"
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    stateMachine.setActionWindow(8)
                                }
                            }
                        }
                        Text {
                            id: text_refresh
                            text: qsTr("Refresh")
                            color: "#ffffff"
                            font.pixelSize: dp(12)
                            font.letterSpacing: dp(0.5)
                            anchors.top: rec_refresh.bottom
                            anchors.topMargin: dp(5)
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }

                    Rectangle {
                        color: "#00000000"
                        Layout.fillHeight: true
                        Layout.minimumHeight: parent.parent.height
                        Layout.minimumWidth: parent.parent.width/3
                        Rectangle {
                            id: rec_transfer
                            height: parent.height/1.3
                            width: height
                            color: "#353237"
                            radius: dp(50)
                            anchors.horizontalCenter: parent.horizontalCenter
                            ImageColor {
                                id: img_transfer
                                img_height: rec_transfer.height/1.7
                                img_source:"../../img/swap.svg"
                                img_color: "#ffffff"
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    stateMachine.setActionWindow(5)
                                    nav.toggle()
                                }
                            }
                        }
                        Text {
                            id: text_transfer
                            text: qsTr("Transfer")
                            color: "#ffffff"
                            font.pixelSize: dp(12)
                            font.letterSpacing: dp(0.5)
                            anchors.top: rec_transfer.bottom
                            anchors.topMargin: dp(5)
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }

                    Rectangle {
                        color: "#00000000"
                        Layout.fillHeight: true
                        Layout.minimumHeight: parent.parent.height
                        Layout.minimumWidth: parent.parent.width/3
                        Rectangle {
                            id: rect_logout
                            height: parent.height/1.3
                            width: height
                            color: Theme.inputError
                            radius: dp(50)
                            anchors.horizontalCenter: parent.horizontalCenter
                            ImageColor {
                                id: img_logout
                                img_height: rect_logout.height/1.7
                                img_source:"../../img/logout.svg"
                                img_color: "#ffffff"
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    messagebox.open(qsTr("Logout"), qsTr("Are you sure you want to logout?"), true, "No", "Yes", "", "", "", changeInstanceCallback)
                                }
                            }
                        }
                        Text {
                            id: text_logout
                            text: qsTr("Logout")
                            color: "#ffffff"
                            font.pixelSize: dp(12)
                            font.letterSpacing: dp(0.5)
                            anchors.top: rect_logout.bottom
                            anchors.topMargin: dp(5)
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:640;width:320}D{i:43;anchors_x:38}D{i:44;anchors_x:116}
}
##^##*/
