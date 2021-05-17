import QtQuick 2.12
import QtQuick.Window 2.12
import Qt.labs.platform 1.1
import QtQuick.Controls 2.13
import WalletBridge 1.0
import TransactionsBridge 1.0
import ConfigBridge 1.0
import UtilBridge 1.0
import NotificationBridge 1.0
import QtAndroidService 1.0

Item {
    id: transactionsItem

    property var allTrans: []
    property var node_height: 0
    property var type_TRANSACTION_CANCELLED: 0x8000
    property var type_TRANSACTION_COIN_BASE: 4
    property var type_TRANSACTION_RECEIVE: 2
    property var type_TRANSACTION_SEND: 1
    property var type_TRANSACTION_NONE: 0
    property var number_COIN_BASE_CONFIRM: 1440
    property var locale: Qt.locale()
    property var message_LEVEL_INFO: 4
    property var message_LEVEL_CRITICAL: 2

    WalletBridge {
        id: wallet
    }

    TransactionsBridge {
        id: transaction
    }

    ConfigBridge {
        id: config
    }

    UtilBridge {
        id: util
    }

    NotificationBridge {
        id: notification
    }

    QtAndroidService {
        id: qtAndroidService
    }

    Connections {
        target: wallet
        onSgnNodeStatus: {
            if (online) {
                node_height = nodeHeight
            }
        }

        onSgnTransactions: {
            if (account !== wallet.getCurrentAccountName() )
                return

            rect_progress.visible = false
            transactionList.visible = true

            allTrans = []
            transactions.forEach(tx => allTrans.push(tx))
            updateData()
        }

        onSgnTransactionById: {
            rect_progress.visible = false
            transactionList.visible = true

            if (!success) {
                messagebox.open(qsTr("Transaction details"), qsTr("Internal error. Transaction details are not found."))
                return
            }
            const txinfo = JSON.parse(transaction)
            const outputsInfo = []
            outputs.forEach(json => {
                outputsInfo.push(json)
            })
            const txnNote = config.getTxNote(txinfo.txid);
            transactionDetail.init(account, txinfo, outputsInfo, messages, txnNote)
            transactionDetail.visible = true
        }

        onSgnNewNotificationMessage: {
            if (message.includes("Changing transaction")) {
                requestTransactions()
            }
        }

        onSgnRepost: (idx, err) => {
            rect_progress.visible = false
            transactionList.visible = true

            if (err === "") {
                notification.appendNotificationMessage(message_LEVEL_INFO, "Transaction #" + Number(idx+1).toString() + " was successfully reposted.")
                messagebox.open("Repost", "Transaction #" + Number(idx + 1).toString() + " was successfully reposted.")
            } else {
                notification.appendNotificationMessage(message_LEVEL_CRITICAL, "Failed to repost transaction #" + Number(idx+1).toString() + ". " + err)
                messagebox.open("Repost", "Failed to repost transaction #" + Number(idx+1).toString() + ".\n\n" + err)
            }
        }

        onSgnCancelTransacton: (success, account, trIdxStr, errMessage) => {
            rect_progress.visible = true
            transactionList.visible = false
            const trIdx = parseInt(trIdxStr)
            if (success) {
                requestTransactions()
            } else {
                messagebox.open("Failed to cancel transaction", "Cancel request for transaction number " + Number(trIdx + 1).toString() + " has failed.\n\n")
            }
        }

        onSgnVerifyProofResult: (success, fn, msg) => {
            console.log("Get onSgnVerifyProofResult with " + success + " " + fn  + " msg")
            if (success) {
                const proof = showProofDlg.parseProofText(msg)
                if (proof) {
                    showProofDlg.open(fn, proof)
                }
                else {
                    messagebox.open("Failure", "Internal error. Unable to decode the results of the proof located at  " + fn + "\n\n" + msg)
                }
            }
            else {
                messagebox.open("Failure", msg)
            }
        }

        onSgnWalletBalanceUpdated: {
            updateAccountsData()
        }
    }

    function txRepost(txIdx) {
        wallet.repost(wallet.getCurrentAccountName(), txIdx, config.isFluffSet())
        rect_progress.visible = true
        transactionList.visible = false
    }

    function txCancel(txIdx) {
        wallet.requestCancelTransacton(wallet.getCurrentAccountName(), Number(txIdx).toString())
    }

    function requestTransactions() {
        transactionModel.clear()
        allTrans = []
        const account = wallet.getCurrentAccountName()
        if (account === "") {
            return
        }
        rect_progress.visible = true
        transactionList.visible = false
        wallet.requestNodeStatus()
        wallet.requestTransactions(account, true)
        updateData()
    }

    function updateData() {
        transactionModel.clear()
        const expectedConfirmNumber = config.getInputConfirmationNumber()
        for ( let idx = allTrans.length - 1; idx >= 0; idx--) {
            const trans = JSON.parse(allTrans[idx])
            const selection = 0.0

//            if (canBeCancelled(trans.transactionType, trans.confirmed)) {
//                const age = calculateTransactionAge(trans.creationTime)
//                // 1 hours is a 1.0
//                selection = age > 60 * 60 ? 1.0 : (Number(age) / Number(60 * 60))
//            }

            let transConfirmedStr = trans.confirmed ? "YES" : "NO"
            // if the node is online and in sync, display the number of confirmations instead
            // node_height will be 0 if the node is offline or out of sync
            if (node_height > 0 && trans.height > 0) {
                const needConfirms = isCoinBase(trans.transactionType) ? number_COIN_BASE_CONFIRM : expectedConfirmNumber
                // confirmations are 1 more than the difference between the node and transaction heights
                const confirmations = node_height - trans.height + 1
                transConfirmedStr = Number(confirmations).toString()
                if (needConfirms >= confirmations) {
                    transConfirmedStr += "/" + Number(needConfirms).toString()
                }
            }

            transactionModel.append({
                txIdx: trans.txIdx,
                txType: getTypeAsStr(trans.transactionType, trans.confirmed),
                txId: "ID: " + trans.txid,
                txAddress: trans.address === "file" ? "File Transfer" : trans.address,
                txTime: getTxTime(trans.creationTime),
                txCoinNano: util.nano2one(trans.coinNano) + " MWC",
                txConfirmedStr: transConfirmedStr,
                txHeight: trans.height <= 0 ? "" : Number(trans.height).toString(),
//                selection: selection
            })
        }
    }

    function canBeCancelled(transactionType, confirmed) {
        return (transactionType & type_TRANSACTION_CANCELLED) === 0 && !confirmed
    }

    function calculateTransactionAge(creationTime) {
        const now = new Date()
        const txCreationTime = new Date(creationTime)
        return (now.getTime() - txCreationTime.getTime()) / 1000
    }

    function isCoinBase(transactionType) {
        return transactionType === type_TRANSACTION_COIN_BASE
    }

    function getTypeAsStr(transactionType, confirmed) {
        if ( transactionType & type_TRANSACTION_CANCELLED )
            return "Cancelled";

        if (!confirmed)
            return "Unconfirmed"

        if ( transactionType & type_TRANSACTION_SEND )
            return "Sent"

        if ( transactionType & type_TRANSACTION_RECEIVE )
            return "Received"

        if ( transactionType & type_TRANSACTION_COIN_BASE )
            return "CoinBase"
    }

    function getTxTime(creationTime) {
        const date = Date.fromLocaleString(locale, creationTime, "hh:mm:ss dd-MM-yyyy")
        return date.toLocaleString(locale, "MMM dd, yyyy / hh:mm ap")
    }

    function getTxTypeIcon(txType) {
        if (txType === "Cancelled")
            return "../img/Transactions_Cancelled@2x.svg"

        if (txType === "Unconfirmed")
            return "../img/Transactions_Unconfirmed@2x.svg"

        if (txType === "Sent")
            return "../img/Transactions_Sent@2x.svg"

        if (txType === "Received")
            return "../img/Transactions_Received@2x.svg"

        if (txType === "CoinBase")
            return "../img/Transactions_CoinBase@2x.svg"
    }

    function isTxValid(tx) {
        return tx.txIdx >= 0 && tx.transactionType !== type_TRANSACTION_NONE
    }

    function updateAccountsData() {
        const accounts = wallet.getWalletBalance(true, false, true)
        const selectedAccount = wallet.getCurrentAccountName()
        let selectedAccIdx = 0

        accountItems.clear()

        let idx = 0
        for (let i = 1; i < accounts.length; i += 2) {
            if (accounts[i-1] === selectedAccount)
                selectedAccIdx = idx

            accountItems.append({ info: accounts[i], account: accounts[i-1]})
            idx++
        }
        accountComboBox.currentIndex = selectedAccIdx
    }

    function currentSelectedAccount() {
        if (accountComboBox.currentIndex >= 0)
            return accountItems.get(accountComboBox.currentIndex).account
    }

    onVisibleChanged: {
        if (visible) {
            rect_progress.visible = true
            updateAccountsData()
            requestTransactions()
            updateData()
        }
    }


    ListModel {
        id: transactionModel
    }

    ListView {
        id: transactionList
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: rect_accounts.bottom
        model: transactionModel
        delegate: transactionDelegate
        focus: true
    }

    Component {
        id: transactionDelegate
        Rectangle {
            height: dp(170)
            color: "#00000000"
            anchors.left: parent.left
            anchors.right: parent.right

            Rectangle {
                height: dp(160)
                color: "#33bf84ff"
                anchors.top: parent.top
                anchors.topMargin: dp(10)
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                anchors.left: parent.left
                anchors.leftMargin: dp(20)

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        const account = wallet.getCurrentAccountName()
                        if (account === "" || index < 0 || index >= allTrans.length)
                            return
                        // respond will come at updateTransactionById
                        wallet.requestTransactionById(account, Number(txIdx).toString());
                        rect_progress.visible = true
                        transactionList.visible = false
                    }
                }

                Rectangle {
                    width: dp(10)
                    height: parent.height
                    anchors.top: parent.top
                    anchors.left: parent.left
                    color: "#BCF317"
                    visible: txType === "Unconfirmed"
                }

                Image {
                    width: dp(17)
                    height: dp(17)
                    anchors.top: parent.top
                    anchors.topMargin: dp(20)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    fillMode: Image.PreserveAspectFit
                    source: getTxTypeIcon(txType)
                }

                Text {
                    color: "#ffffff"
                    text: txType
                    font.bold: true
                    font.pixelSize: dp(15)
                    anchors.top: parent.top
                    anchors.topMargin: dp(22)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(71)
                }

                Text {
                    width: dp(200)
                    height: dp(15)
                    color: "#bf84ff"
                    text: txTime
                    horizontalAlignment: Text.AlignRight
                    font.pixelSize: dp(15)
                    anchors.top: parent.top
                    anchors.topMargin: dp(22)
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
                    text: txCoinNano
                    font.bold: true
                    anchors.top: parent.top
                    anchors.topMargin: dp(70)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    font.pixelSize: dp(15)
                }

                Text {
                    color: "#ffffff"
                    text: txId
                    anchors.top: parent.top
                    anchors.topMargin: dp(95)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    font.pixelSize: dp(15)
                }

                Text {
                    color: "#ffffff"
                    text: txAddress
                    elide: Text.ElideMiddle
                    anchors.top: parent.top
                    anchors.topMargin: dp(120)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(35)
                    font.pixelSize: dp(15)
                }
            }
        }
    }

    Rectangle {
        id: rect_accounts
        height: dp(170)
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
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

        Text {
            id: label_combobox
            color: "#ffffff"
            text: qsTr("Showing Transactions From This Account")
            anchors.left: parent.left
            anchors.leftMargin: dp(30)
            anchors.top: parent.top
            anchors.topMargin: dp(20)
            font.pixelSize: dp(14)
        }

        ComboBox {
            id: accountComboBox
            height: dp(60)
            anchors.right: parent.right
            anchors.rightMargin: dp(30)
            anchors.left: parent.left
            anchors.leftMargin: dp(30)
            anchors.top: label_combobox.bottom
            anchors.topMargin: dp(10)
            leftPadding: dp(20)
            rightPadding: dp(40)
            font.pixelSize: dp(15)

            onCurrentIndexChanged: {
                // Selecting the active account
                const selectedAccount = currentSelectedAccount()
                if (selectedAccount !== "") {
                    wallet.switchAccount(selectedAccount)
                    requestTransactions()
                }
            }

            delegate: ItemDelegate {
                width: accountComboBox.width
                height: dp(60)
                contentItem: Text {
                    text: info
                    color: "white"
                    font: accountComboBox.font
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
                background: Rectangle {
                    color: accountComboBox.highlightedIndex === index ? "#955BDD" : "#8633E0"
                }
                topPadding: dp(10)
                bottomPadding: dp(10)
                leftPadding: dp(20)
                rightPadding: dp(20)
            }

            indicator: Canvas {
                id: canvas
                x: accountComboBox.width - width - accountComboBox.rightPadding / 2
                y: accountComboBox.topPadding + (accountComboBox.availableHeight - height) / 2
                width: dp(14)
                height: dp(7)
                contextType: "2d"

                Connections {
                    target: accountComboBox
                    function onPressedChanged() { canvas.requestPaint() }
                }

                onPaint: {
                    context.reset()
                    if (accountComboBox.popup.visible) {
                        context.moveTo(0, height)
                        context.lineTo(width / 2, 0)
                        context.lineTo(width, height)
                    } else {
                        context.moveTo(0, 0)
                        context.lineTo(width / 2, height)
                        context.lineTo(width, 0)
                    }
                    context.strokeStyle = "white"
                    context.lineWidth = 2
                    context.stroke()
                }
            }

            contentItem: Text {
                text: accountComboBox.currentIndex >= 0 && accountItems.get(accountComboBox.currentIndex).info
                font: accountComboBox.font
                color: "white"
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            background: Rectangle {
                implicitHeight: dp(60)
                radius: dp(4)
                color: "#8633E0"
            }

            popup: Popup {
                y: accountComboBox.height + dp(3)
                width: accountComboBox.width
                implicitHeight: contentItem.implicitHeight + dp(20)
                topPadding: dp(10)
                bottomPadding: dp(10)
                leftPadding: dp(0)
                rightPadding: dp(0)

                contentItem: ListView {
                    clip: true
                    implicitHeight: contentHeight
                    model: accountComboBox.popup.visible ? accountComboBox.delegateModel : null
                    currentIndex: accountComboBox.highlightedIndex

                    ScrollIndicator.vertical: ScrollIndicator { }
                }

                background: Rectangle {
                    color: "#8633E0"
                    radius: dp(5)
                }

                onVisibleChanged: {
                    if (!accountComboBox.popup.visible) {
                        canvas.requestPaint()
                    }
                }
            }

            model: ListModel {
                id: accountItems
            }
        }

        Rectangle {
            width: dp(250)
            height: dp(40)
            anchors.top: accountComboBox.bottom
            anchors.topMargin: dp(15)
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#00000000"

            Image {
                id: image_validate
                anchors.left: parent.left
                anchors.top: parent.top
                width: dp(40)
                height: dp(40)
                fillMode: Image.PreserveAspectFit
                source: "../img/Validate@2x.svg"

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (qtAndroidService.requestPermissions()) {
                            qtAndroidService.openFile( config.getPathFor("fileGen"), "*/*", 125 )
                        } else {
                            messagebox.open("Failure", "Permission Denied")
                        }
                    }
                }
            }

//            Image {
//                id: image_export
//                anchors.top: parent.top
//                anchors.horizontalCenter: parent.horizontalCenter
//                width: dp(40)
//                height: dp(40)
//                fillMode: Image.PreserveAspectFit
//                source: "../img/Validate1@2x.svg"

//                MouseArea {
//                    anchors.fill: parent
//                    onClicked: {
//                        if (allTrans.length === 0) {
//                            messagebox.open("Export Error", "You don't have any transactions to export.")
//                            return
//                        }
//                        const exportingFileName = util.getSaveFileName("Export Transactions", "TxExportCsv", "Export Options (*.csv)", ".csv")
//                        if (exportingFileName === "")
//                            return

//                        // Starting request process...
//                        rect_progress.visible = true
//                        transactionList.visible = false
//                        const td = allTrans[0]
//                        if(isTxValid(td.trans)) {
//                            wallet.requestTransactionById(account, Number(td.trans.txIdx).toString())
//                            td.tx_note = config.getTxNote(td.trans.txid);
//                            // Now waiting for transaction requests response at onSgnTransactionById
//                        } else {
//                            messagebox.open("Export Error", "Invalid Wallet")
//                        }
//                    }
//                }
//            }

            Image {
                id: image_refresh
                anchors.right: parent.right
                anchors.top: parent.top
                width: dp(40)
                height: dp(40)
                fillMode: Image.PreserveAspectFit
                source: "../img/Refresh@2x.svg"

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        requestTransactions()
                    }
                }
            }
        }
    }

    Connections {
        target: qtAndroidService
        onSgnOnFileReady: (eventCode, path ) => {
            if (eventCode == 125 && path) {
                        // path = decodeURIComponent(path)
                        console.log("Open proof transaction file: " + path)
                        const validation = util.validateMwc713Str(path)
                        if (validation) {
                            messagebox.open(qsTr("File Path"), qsTr("This file path is not acceptable.\n" + validation))
                            return
                        }
                        config.updatePathFor("fileGen", path)
                        wallet.verifyTransactionProof(path)
            }
        }
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
}
