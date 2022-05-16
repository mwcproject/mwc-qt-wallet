import QtQuick 2.15
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.15 as T
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
    property alias navHeight: navbarTop.height

    property int selectedAccountIndex: -1


    property bool isEdit: false
    property string docName
    //property alias open: nav.open

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
            //updateNetworkName()
            //
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
                    //color: Theme.card
                    height: (4*parent.height)/5
                    width: control.width
                    radius: dp(50)
                    anchors.left: parent.left
                    anchors.leftMargin: (-4*control.width)/5
                    color: Theme.card

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
                            nav.open()
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
                    color: Theme.textPrimary
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
                id: rec_view
                Layout.fillHeight: true
                Layout.minimumHeight: navbarTop.height
                Layout.minimumWidth: navbarTop.width/5
                color: "#00000000"
                Rectangle {
                    height: parent.height*0.8
                    width: height
                    color: Theme.card
                    radius: dp(150)
                    anchors.horizontalCenter: parent.horizontalCenter
                    ImageColor {
                        id: logout
                        img_source: hiddenAmount ? "../../img/invisible.svg" : "../../img/visibility.svg"
                        img_height: parent.height* 0.6
                        img_color: "gray"
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            hiddenAmount = !hiddenAmount
                            console.log("hidden Amount", hiddenAmount)
                            walletConfig.setWalletAmountHiddenEnabled(hiddenAmount)
                            let lmao = walletConfig.getWalletAmountHiddenEnabled()
                            console.log("hidden Amount", lmao)
                        }
                    }
                }

            }
            states: [

                State {
                    when: currentState == 21 || currentState == 11
                    PropertyChanges {
                        target: rec_account; x: 0
                    }
                    PropertyChanges {
                        target: rec_view; x: 4*control.width/5
                    }
                },
                State {
                    when:  currentState !== 21 && currentState != 11
                    PropertyChanges {
                        target: rec_account; x: -rec_account.width
                    }
                    PropertyChanges {
                        target: rec_view; x: control.width
                    }
                }]

            transitions: Transition {
                PropertyAnimation {
                    properties: "x";
                    duration: 500
                }
            }


        }

    }

    Drawer {
        id: nav
        property alias repeater: rep
        height: parent.height
        width: 7*parent.width/10
        edge: Qt.LeftEdge
        interactive: (currentState == 21 || currentState == 11) ? true: false

        onClosed: {
            isEdit = false
        }

        background: Rectangle {
            //radius: 50
            height: parent.height
            width: parent.width
            //y: -parent.height/5
            color: Theme.bg
        }

        T.Overlay.modal: Rectangle {
            color: "#80000000"
            Behavior on opacity {
                NumberAnimation {
                    duration: 150
                }
            }
        }

        enter: Transition {
            SmoothedAnimation {
                velocity: -1
                duration: 750
                easing.type: Easing.OutCirc
                easing.amplitude: 2.0
                easing.period: 1.5
            }
        }
        exit: Transition {
            SmoothedAnimation {
                velocity: -1
                duration: 500
                easing.type: Easing.OutCirc
                easing.amplitude: 2.0
                easing.period: 1.5

            }

        }

        Rectangle {
            anchors.fill: parent
            color: "#00000000"

            ImageColor {
                id: image_logo
                img_height: dp(29)
                img_source: "../../img/mwc-logo.svg"
                img_color: Theme.textPrimary
                anchors.top: parent.top
                anchors.topMargin: dp(70)
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                id: text_account
                text: qsTr("TOTAL WORTH")
                color: Theme.textSecondary
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
                color: Theme.textPrimary
                font.pixelSize: dp(20)
                //font.letterSpacing: dp(1)
                anchors.top: text_account.bottom
                anchors.topMargin: dp(3)
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                id: text_secondary_currency
                text: (hiddenAmount? hidden : String((currencyPrice * totalAmount).toFixed(currencyPriceRound))) + " %1".arg(currencyTicker.toUpperCase())
                color: Theme.textSecondary
                font.pixelSize: dp(12)
                font.letterSpacing: dp(1)
                anchors.top: text_spend_balance.bottom
                anchors.topMargin: dp(2)
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                id: text_acc_list
                text: qsTr("MY ACCOUNTS")
                color: Theme.textSecondary
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
                width: height*1.3
                color: "#353237"
                radius: width*0.1
                anchors.right: rect_phrase.right
                anchors.rightMargin: dp(25)
                anchors.bottom: rect_phrase.top
                anchors.bottomMargin: dp(8)
                visible: true
                ImageColor {
                    id: img_editButton
                    img_height: rect_edit.height/1.7
                    img_source: "../../img/edit.svg"
                    img_color: Theme.textPrimary
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
                color: "#00000000"
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
                        Layout.minimumWidth: rect_phrase.width

                        Repeater {
                            id: rep
                            model: accountsModel
                            Rectangle {
                                id: rec_acc_balance
                                height: dp(60)
                                width: nav.width
                                color: selectedAccount === account? Theme.cardSelected : "#00000000"

                                ImageColor {
                                    id: img_check
                                    img_height: parent.height/2.5
                                    img_source: isEdit && nav.open? "../../img/remove.svg"  : "../../img/check.svg"
                                    img_color: isEdit && nav.open? "#00000000" : Theme.iconSelected
                                    visible: (isEdit && nav.open)? (account !== "default" && selectedAccount !== account ? true : false) : (selectedAccount === account? true : false)
                                    anchors.left: parent.left
                                    anchors.leftMargin: dp(25)
                                    anchors.verticalCenter: parent.verticalCenter
                                }

                                TextEdit {
                                    id: acc_name
                                    readOnly: true
                                    text: "%1".arg(account)
                                    color: selectedAccount === account? "#f5f5f5": "gray"
                                    anchors.left: img_check.right
                                    anchors.leftMargin: dp(25)
                                    font.pixelSize: dp(15)
                                    font.weight: Font.Light
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.verticalCenterOffset: -height/2
                                    onEditingFinished: {
                                        const newAccountName = acc_name.text
                                        if (newAccountName === "" || newAccountName === selectedAccount) {
                                            acc_name.text = selectedAccount
                                            acc_name.focus = false
                                            return
                                        }

                                       let err = accState.validateNewAccountName(newAccountName)
                                        if (err !== "") {
                                            acc_name.text = selectedAccount
                                            acc_name.focus = false
                                            return;
                                        }
                                        wallet.renameAccount(selectedAccount, newAccountName)
                                        acc_name.focus = false
                                        readOnly = true
                                        isEdit = false
                                        startWaiting()


                                    }
                                }

                                Text {
                                    text: (hiddenAmount? hidden : spendable) + " MWC"
                                    color: selectedAccount === account? "#efefef": "gray"
                                    anchors.left: img_check.right
                                    anchors.leftMargin: dp(25)
                                    font.pixelSize: dp(15)
                                    font.italic: true
                                    font.weight: Font.Light
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.verticalCenterOffset: height/2
                                }

                                ImageColor {
                                    id: img_edit
                                    img_height: acc_name.focus && selectedAccount === account? parent.height/2 : parent.height/3
                                    img_source: acc_name.focus && selectedAccount === account ? "../../img/check-edit.svg" :  "../../img/pencil.svg"
                                    img_color: "#f5f5f5"
                                    visible: (isEdit && nav.open)? (account !== "default" ? true : false) : false
                                    anchors.right: parent.right
                                    anchors.rightMargin: dp(25)
                                    anchors.verticalCenter: parent.verticalCenter

                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        acc_name.focus = false
                                        if (selectedAccount !== account && !isEdit) {
                                            selectedAccount = account
                                            selectedAccountIndex = index
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
                                        isInit = true
                                        selectedAccount = account
                                        nav.close()
                                        messagebox.open("Delete account", "Are you sure that you want to delete the \"%1\" account?".arg(account), true, "No", "Yes", "", "", "", onDeleteAccount)
                                    }
                                }

                                MouseArea {
                                    anchors.fill: img_edit
                                    onClicked: {
                                        if (acc_name.focus === true) {
                                            acc_name.focus = false
                                        } else {
                                            isInit = true
                                            selectedAccount = account
                                            acc_name.readOnly = false
                                            acc_name.focus = true
                                        }


                                    }
                                }
                            }
                        }

                        Rectangle {
                            id: rec_new_acc
                            Layout.minimumHeight: dp(60)
                            Layout.minimumWidth: rect_phrase.width
                            color: "#00000000"
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
                                id: create_acc
                                anchors.fill: parent
                                onClicked: {
                                    nav.close()
                                    inputDlg.open(qsTr("Add account"), qsTr("Please specify the name of a new account in your wallet"), "account name", "", 32, onAddAccount)
                                }
                            }
                            ColorOverlay {
                                id: overlay
                                height: dp(60)
                                width: rect_phrase.width
                                source: rec_new_acc
                                opacity: 0
                            }
                            states: [
                                State {
                                    name: "pressed"; when:  create_acc.pressed
                                    PropertyChanges { target: overlay; color: "black"; opacity: 0.5 }
                                },
                                State {
                                    name: "unpressed"; when: !create_acc.pressed
                                    PropertyChanges { target: overlay; color: "black"; opacity: 0 }
                                }]

                            transitions: Transition {
                                NumberAnimation { properties: "opacity"; duration: 100; easing.type: Easing.InOutQuad }
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
                                img_color: Theme.textPrimary
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
                            color: Theme.textPrimary
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
                                img_color: Theme.textPrimary
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    stateMachine.setActionWindow(5)
                                    nav.close()
                                }
                            }
                        }
                        Text {
                            id: text_transfer
                            text: qsTr("Transfer")
                            color: Theme.textPrimary
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
                            width: height*1.3
                            color: Theme.inputError
                            radius: dp(20)
                            anchors.horizontalCenter: parent.horizontalCenter
                            ImageColor {
                                id: img_logout
                                img_height: rect_logout.height/1.7
                                img_source:"../../img/logout.svg"
                                img_color: Theme.textPrimary
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    nav.close()
                                    messagebox.open(qsTr("Logout"), qsTr("Are you sure you want to logout?"), true, "No", "Yes", "", "", "", changeInstanceCallback)
                                }
                            }
                        }
                        Text {
                            id: text_logout
                            text: qsTr("Logout")
                            color: Theme.textPrimary
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
