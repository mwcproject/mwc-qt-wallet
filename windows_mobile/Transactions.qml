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
import QtQuick.Layouts 1.3
import "./models"

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
            console.log("onSgnTransactions: ",txsList)
            allTrans = []
            transactions.forEach(tx => allTrans.push(tx))
            console.log("onSgnTransactions AllTrans: ", allTrans)
            updateData()
        }

        onSgnTransactionById: {
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
            if (err === "") {
                notification.appendNotificationMessage(message_LEVEL_INFO, "Transaction #" + Number(idx+1).toString() + " was successfully reposted.")
                messagebox.open("Repost", "Transaction #" + Number(idx + 1).toString() + " was successfully reposted.")
            } else {
                notification.appendNotificationMessage(message_LEVEL_CRITICAL, "Failed to repost transaction #" + Number(idx+1).toString() + ". " + err)
                messagebox.open("Repost", "Failed to repost transaction #" + Number(idx+1).toString() + ".\n\n" + err)
            }
        }

        onSgnCancelTransacton: (success, account, trIdxStr, errMessage) => {
            const trIdx = parseInt(trIdxStr)
            if (success) {
                requestTransactions()
            } else {
                messagebox.open("Failed to cancel transaction", "Cancel request for transaction number " + Number(trIdx + 1).toString() + " has failed.\n\n")
            }
        }

        onSgnVerifyProofResult: (success, fn, msg) => {
            console.log("Get onSgnVerifyProofResult with " + success + " " + fn  + " " + msg)
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
    }

    function txRepost(txIdx) {
        wallet.repost(wallet.getCurrentAccountName(), txIdx, config.isFluffSet())
    }

    function txCancel(txIdx) {
        wallet.requestCancelTransacton(wallet.getCurrentAccountName(), Number(txIdx).toString())
    }

    function requestTransactions() {

        allTrans = []
        const account = wallet.getCurrentAccountName()
        if (account === "") {
            return
        }
        wallet.requestNodeStatus()
        wallet.requestTransactions(account, true)
        updateData()
    }

    function updateData() {
        let cache =  []
        const expectedConfirmNumber = config.getInputConfirmationNumber()
        let currentDate = 0
        for (let idx = allTrans.length - 1; idx >= 0; idx--) {
            const trans = JSON.parse(allTrans[idx])
            const selection = 0.0
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

            let txDate = getTxTime(trans.creationTime, true)

            if (currentDate !== txDate) {
                currentDate = txDate
                cache.push({
                    tx: false,
                    txDate: txDate,

                 })
            }

            let type = getTypeAsStr(trans.transactionType, trans.confirmed)
            let txAddr = trans.address === "file" ? "File Transfer" : trans.address
            let txTime = getTxTime(trans.creationTime, false)
            let txCoinNano = util.nano2one(trans.coinNano) + " MWC"
            let txHeight = trans.height <= 0 ? "" : Number(trans.height).toString()
            let txUUID = shortUUID(trans.txid)


            cache.push({
                tx: true,
                txIdx: trans.txIdx,
                txType: type,
                txId: txUUID,
                txDate: txDate,
                txTime: txTime,
                txCoinNano: txCoinNano,
                txConfirmedStr: transConfirmedStr,
            })
        }
        /*if (txsListCache != cache) {
            txsListCache = cache
            console.log("cache: ", cache, "\n txsListCache: ", txsListCache)
            txsModal.clear()
            cache.forEach(tx => txsModal.append(tx))
        }*/
        //txsModal.clear()
        cache.forEach(tx => txsModal.append(tx))

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

    function getTxTime(creationTime, isDate) {
        const date = Date.fromLocaleString(locale, creationTime, "hh:mm:ss dd-MM-yyyy")
        if (isDate) {
            return date.toLocaleString(locale, "M/d/yy")
        }
        return date.toLocaleString(locale, "hh:mm ap")

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

    function shortUUID(uuid) {
        let tx = uuid.slice(0, 8) + "..." + uuid.slice(uuid.length-8, uuid.length)
        return tx
    }

    onVisibleChanged: {
        if (visible) {
            requestTransactions()
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            search.field_focus = false
        }
    }

    Rectangle {
        id: container
        anchors.fill: parent
        color: dark.bgGradientBottom
        //radius: dp(25)
        ListView {
            id: transactionList
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.top: parent.top
            ScrollBar.vertical: ScrollBar {
                policy: Qt.ScrollBarAsNeeded
            }
            clip: true
            model: txsModal
            delegate: transactionDelegate
            focus: true
        }
    }

    ListModel {
        id: txsModal
    }






    Component {
        id: transactionDelegate
        Rectangle {
            height: tx? dp(72) : dp(40)
            width: container.width
            color: "#00000000"

            Text {
                text: txDate
                visible: tx? false : true
                font.pixelSize: parent.height*0.4
                font.family: barlow.medium
                color: "gray"
                anchors.left: parent.left
                anchors.leftMargin: dp(20)
                anchors.verticalCenter: parent.verticalCenter
            }

            Rectangle {
                height: dp(70)
                color: "#252525"
                visible: tx? true : false
                width: container.width
                anchors.verticalCenter: parent.verticalCenter
                //anchors.top: parent.top
                //anchors.topMargin: dp(20)

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                         console.log("local: ", locale.uiLanguages )
                        const account = wallet.getCurrentAccountName()
                        if (account === "") //==|| index < 0 || index >= allTrans.length)
                            return
                        // respond will come at updateTransactionById
                        wallet.requestTransactionById(account, Number(txIdx).toString());
                    }
                }

                Image {
                    id: img_status
                    width: dp(17)
                    height: dp(17)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    fillMode: Image.PreserveAspectFit
                    anchors.verticalCenter: parent.verticalCenter
                    source: tx?getTxTypeIcon(txType): ""
                    visible: tx? true : false
                }

                Text {
                    id: text_uuid
                    color: "#ffffff"
                    text: tx? txId : ""
                    font.pixelSize: parent.height*0.25
                    anchors.left: img_status.right
                    anchors.leftMargin: dp(10)
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: -dp(10)
                    visible: tx? true : false

                }
                Text {
                    id: text_status
                    color: "gray"
                    font.pixelSize: parent.height*0.25
                    text: txType + " "  + txTime
                    anchors.left: img_status.right
                    anchors.leftMargin: dp(10)
                    anchors.top: text_uuid.bottom
                    visible: tx? true : false
                }

                Text {
                    id: text_amountmwc
                    color: "gray"
                    text: tx? txCoinNano : ""
                     font.pixelSize: parent.height*0.25
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: -dp(10)
                    anchors.rightMargin: dp(10)
                    visible: tx? true : false
                }
                Text {
                    id: text_amountprice
                    color: "gray"
                    font.pixelSize: parent.height*0.25
                    text: txCoinNano *2.6
                    anchors.right: parent.right
                    anchors.top: text_amountmwc.bottom
                    anchors.rightMargin: dp(10)
                    visible: tx? true : false
                }



            }
        }
    }
}
