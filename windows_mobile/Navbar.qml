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
    readonly property int status_ignore: 0
    readonly property int status_red: 1
    readonly property int status_yellow: 2
    readonly property int status_green: 3

    // temporary fix for a bug related to init account switch --
    property bool initAccount: false
    // --

    property double spendableBalance: 0
    property double awaitBalance: 0
    property double lockedBalance: 0

    property alias topVisible: navbarTop.visible

    property string selectedAccount
    property bool isEdit: false
    property string docName

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
                text_title.text = qsTr("Wallet Settings")
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
            text_spend_balance.text = wallet.getTotalMwcAmount() + " MWC"
            updateAccountsList()
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
            //updateAccountList()
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

    Rectangle
    {
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
                Layout.fillHeight: true
                Layout.minimumHeight: parent.parent.height
                Layout.minimumWidth: parent.parent.width*(1/5)
                color: "#00000000"
                Image {
                    id: notification
                    height: parent.height* 0.4
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    source: "../img/menu.svg"
                    fillMode: Image.PreserveAspectFit

                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        nav.toggle()
                        //stateMachine.setActionWindow(6)
                    }
                }
            }
            Rectangle {
                Layout.fillHeight: true
                Layout.minimumHeight: parent.parent.height
                Layout.minimumWidth: parent.parent.width*(3/5)
                color: "#00000000"
                Text {
                    id: text_title
                    color: "#ffffff"
                    text: "Wallet Dashboard"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    font.bold: true
                    font.pixelSize: dp(16)
                }
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.minimumHeight: parent.parent.height
                Layout.minimumWidth: parent.parent.width/5
                color: "#00000000"
                Image {
                    id: logout
                    height: parent.height* 0.4
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    source: "../img/logout.svg"
                    fillMode: Image.PreserveAspectFit

                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {

                        messagebox.open(qsTr("LOGOUT / CHANGE WALLET"), qsTr("Are you sure you want to logout?"), true, "No", "Yes", "", "", "", changeInstanceCallback)
                    }
                }
            }


        }

    }
    NavigationDrawer {
        id: nav
        //property type name: value
        Rectangle
        {
            anchors.fill: parent
            color: "#181818"
        }
        Rectangle {
            anchors.fill: parent
            color: "#00000000"

            Image {
                id: image_logo
                width: dp(58)
                height: dp(29)
                anchors.top: parent.top
                anchors.topMargin: dp(70)
                anchors.horizontalCenter: parent.horizontalCenter
                source: "../img/mwc-logo.svg"
                fillMode: Image.PreserveAspectFit
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
                text: ""
                font.weight: Font.Light
                color: "white"
                font.pixelSize: dp(17)
                //font.letterSpacing: dp(1)
                anchors.top: text_account.bottom
                anchors.topMargin: dp(3)
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                id: text_secondary_currency
                text: qsTr("58 000.18 USD")
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
                anchors.bottom: rect_phrase.top
                anchors.bottomMargin: dp(8)
            }

            ImageColor {
                id: img_editList
                img_height: dp(20)
                img_source: "../../img/edit.svg"
                img_color: isEdit? "white" : "grey"
                anchors.right: rect_phrase.right
                anchors.rightMargin: dp(25)
                anchors.bottom: rect_phrase.top
                anchors.bottomMargin: dp(8)
                visible: true
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
                color: "#0f0f0f"
                anchors.top: text_secondary_currency.bottom
                anchors.topMargin: dp(80)

                Flickable {
                    id: scroll
                    width: parent.width
                    height: parent.height
                    contentHeight: grid_seed.Layout.minimumHeight
                    boundsMovement: Flickable.StopAtBounds
                    //flickableDirection: Flickable.VerticalFlick
                    //ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                    //Flickable.flickableDirection: Flickable.VerticalFlick
                    clip: true
                    //flickableItem.atYEnd: true
                    //anchors.horizontalCenter: parent.horizontalCenter

                    ScrollBar.vertical: ScrollBar {
                        policy: Qt.ScrollBarAlwaysOn
                        //parent: flickable.parent
                        //anchors.top: flickable.top
                        //anchors.left: flickable.right
                        //anchors.bottom: flickable.bottom
                    }


                    ColumnLayout {
                        id: grid_seed
                        spacing: 0
                        //width: rect_phrase.width
                        //height: rect_phrase.height
                        //Layout.margins: dp(25)

                        Repeater {
                            id: rep
                            model: accountsModel
                            Rectangle {
                                id: rec_acc
                                height: dp(60)
                                width: nav.width
                                color: selectedAccount === account? "#363636" : "#00000000"

                                ImageColor {
                                    id: img_check
                                    img_height: parent.height/2.5
                                    img_source: isEdit? "../../img/remove.svg"  : "../../img/check.svg"
                                    //img_visible: true
                                    img_color: isEdit? "#00000000" : "white"
                                    visible: isEdit? (account !== "default" && selectedAccount !== account ? true : false) : (selectedAccount === account? true : false)
                                    anchors.left: parent.left
                                    anchors.leftMargin: dp(25)
                                    anchors.verticalCenter: parent.verticalCenter
                                }

                                Text {
                                    id: acc_name
                                    text: qsTr("%1\n%2" + " MWC").arg(account).arg(spendable)
                                    color: "white"
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
                                    //img_visible: true
                                    img_color: "white"
                                    visible: isEdit? (account !== "default" && selectedAccount !== account ? true : false) : false
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
                            width: nav.width
                            color: "#00000000"
                            visible: isEdit? false : true
                            //anchors.top: grid_seed.bottom
                            Image {
                                id: img_new_acc
                                height: rec_new_acc.height/2.5
                                source:"../img/plus.svg"
                                fillMode: Image.PreserveAspectFit
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: rec_new_acc.left
                                anchors.leftMargin: dp(25)
                                //paddingLeft: dp(25)
                            }
                            ColorOverlay {
                                id: over_new_acc
                                anchors.fill: img_new_acc
                                source: img_new_acc
                                color: "#ffffff"
                            }
                            Text {
                                id: name_new_acc
                                text: qsTr("Add New Account")
                                color: "white"
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

            Button {
                id: button_changeinstance
                height: dp(50)
                anchors.right: parent.right
                anchors.rightMargin: dp(35)
                anchors.left: parent.left
                anchors.leftMargin: dp(35)
                anchors.top: parent.bottom
                anchors.topMargin: dp(-100)
                background: Rectangle {
                    id: rectangle
                    color: "#00000000"
                    radius: dp(4)
                    border.color: "white"
                    border.width: dp(2)
                    Text {
                        id: loginText
                        text: qsTr("Logout / Change Wallet")
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: dp(18)
                        color: "white"
                    }
                }

                onClicked: {
                    //nav.toggle()
                    messagebox.open(qsTr("LOGOUT / CHANGE WALLET"), qsTr("Are you sure you want to logout?"), true, "No", "Yes", "", "", "", changeInstanceCallback)
                }
            }
        }
    }


    Rectangle {
        id: navbarBottom
        height: parent.height/14
        width: parent.width
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: "#15171c"

        RowLayout {
            id: layoutNavBottom
            spacing: 0
            Rectangle {
                color: "#00000000"
                Layout.fillHeight: true
                Layout.minimumHeight: parent.parent.height
                Layout.minimumWidth: parent.parent.width/5
                Layout.preferredWidth: 100
                Layout.maximumWidth: 300
                ImageColor {
                    id: image_wallet
                    img_height: parent.height* 0.5
                    anchors.bottom: text_wallet.top
                    anchors.bottomMargin: dp(2)
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    img_source: "../../img/wallet.svg"
                    img_color: currentState === 21 ? "white" : "grey"


                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        stateMachine.setActionWindow(21)
                    }
                }
            }
            Rectangle {
                color: "#00000000"
                Layout.fillHeight: true
                Layout.minimumHeight: parent.parent.height
                Layout.minimumWidth: parent.parent.width/5
                Layout.preferredWidth: 100
                Layout.maximumWidth: 300
                ImageColor {
                    id: image_txs
                    img_height: parent.height* 0.5
                    anchors.bottom: text_txs.top
                    anchors.bottomMargin: dp(2)
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    img_source: "../../img/list.svg"
                    img_color: currentState === 11 ? "white" : "grey"

                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        stateMachine.setActionWindow(11)
                    }
                }
            }
            Rectangle {
                color: "#00000000"
                Layout.fillHeight: true
                Layout.minimumHeight: parent.parent.height
                Layout.minimumWidth: parent.parent.width/5
                Layout.preferredWidth: 100
                Layout.maximumWidth: 300
                ImageColor {
                    id: image_listeners
                    img_height: parent.height* 0.5
                    anchors.bottom: text_listeners.top
                    anchors.bottomMargin: dp(2)
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    img_source: "../../img/listener.svg"
                    img_color: currentState === 10 ? "white" : "grey"
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        stateMachine.setActionWindow(10)
                    }
                }
            }
            Rectangle {
                color: "#00000000"
                Layout.fillHeight: true
                Layout.minimumHeight: parent.parent.height
                Layout.minimumWidth: parent.parent.width/5
                Layout.preferredWidth: 100
                Layout.maximumWidth: 300

                ImageColor {
                    id: image_account
                    img_height: parent.height* 0.5
                    anchors.bottom: text_account.top
                    anchors.bottomMargin: dp(2)
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    img_source: "../../img/notification.svg"
                    img_color: currentState === 6 ? "white" : "grey"

                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        stateMachine.setActionWindow(6)
                    }
                }
            }
            Rectangle {
                color: "#00000000"
                Layout.fillHeight: true
                Layout.minimumHeight: parent.parent.height
                Layout.minimumWidth: parent.parent.width/5
                Layout.preferredWidth: 100
                Layout.maximumWidth: 300
                ImageColor {
                    id: image_settings
                    img_height: parent.height* 0.5
                    anchors.bottom: text_settings.top
                    anchors.bottomMargin: dp(2)
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    img_source: "../../img/setting.svg"
                    img_color: currentState === 23 ? "white" : "grey"


                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        stateMachine.setActionWindow(23)
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
