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
    property var message_LEVEL_INFO: 4
    property var message_LEVEL_CRITICAL: 2

    property alias txsModal: txsModal




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
            allTrans = []
            transactions.forEach(tx => allTrans.push(tx))
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

            let type = getTypeAsStr(trans.transactionType, trans.confirmed)
            let txAddr = trans.address === "file" ? "File Transfer" : trans.address
            let date  = getTxTime(trans.creationTime)
            let txCoinNano = util.nano2one(trans.coinNano)
            let txHeight = trans.height <= 0 ? "" : Number(trans.height).toString()


            let timestamp = getTimestamp(trans.creationTime)
            cache.push({
                tx: true,
                txIdx: trans.txIdx,
                txType: type,
                txId: trans.txid,
                txDate: date[0],
                txTime: date[1],
                txCoinNano: txCoinNano,
                txConfirmedStr: transConfirmedStr,
                timestamp: timestamp,
                isConf: trans.confirmed
            })
        }
        if (isInit) {
            cache.forEach(tx => txsModal.append(tx))
            isInit = false
            return
        }

        let listModelsNB = txsModal.count -1
        let cacheModelNB = cache.length -1

        let dist = cacheModelNB - listModelsNB
        for (let i=cacheModelNB; i >= 0; i--) {
                let idx = i-dist
                if (idx < 0){
                    txsModal.insert(0, cache[i])
                } else {
                    let txs = txsModal.get(idx).tx
                    let ctxs = cache[i].txs
                    if (ctxs) {
                        if (txs === ctxs) {
                            let date = txsModal.get(idx).txDate
                            let cdate = cache[i].txDate
                            if (date !== cdate)
                                txsmodals.set(idx, cache[i])
                        } else {
                            txsmodals.set(idx, cache[i])
                        }
                   } else {
                        let txId = txsModal.get(idx).txId
                        let ctxId = cache[i].txId
                        if (txId  === ctxId) {
                            let type = txsModal.get(idx).txType
                            let ctype = cache[i].txType
                            if (type !== ctype)
                                txsModal.set(idx, cache[i])
                        } else {
                            txsModal.set(idx, cache[i])
                        }
                   }
              }
          }
        walletItem.setChart()



        

    }
    function getTimestamp(creationTime){
        const date = Date.fromLocaleString(locale, creationTime, "hh:mm:ss dd-MM-yyyy")
        return date.getTime()
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
        return [date.toLocaleString(locale, "M/d/yy"), date.toLocaleString(locale, "hh:mm ap")]

    }

    function getTxTypeIcon(txType) {
        if (txType === "Cancelled")
            return "../img/close.svg"

        if (txType === "Unconfirmed")
            return "../img/hourglass.svg"

        if (txType === "Sent" || txType === "Received")
            return "../img/export.svg"

        if (txType === "CoinBase")
            return "../img/Transactions_CoinBase@2x.svg"
    }

    function isTxValid(tx) {
        return tx.txIdx >= 0 && tx.transactionType !== type_TRANSACTION_NONE
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
        anchors.fill: container
        color: Theme.bg
        opacity: 0.5
    }



    Rectangle {
        id: container
        anchors.fill: parent
        color: "#00000000"

        //radius: dp(25)
        ListView {
            id: transactionList
            width: parent.width
            height: parent.height
            ScrollBar.vertical: ScrollBar {
                policy: Qt.ScrollBarAsNeeded
            }
            header: Rectangle {
                height: dp(40)
                width: container.width*0.9
                color: "#00000000"
                radius: dp(25)
            }

            footer: Rectangle {
                height: dp(40)
                width: container.width*0.9
                color: "#00000000"
                radius: dp(25)
            }
            /*ListView.view.delegate: Rectangle {
                color: "pink"
            }*/
            section.property: "txDate"
            section.criteria: ViewSection.FullString
            section.delegate: ToolBar {
                height: dp(40)
                width: container.width*0.9
                //color: "#00000000"
                anchors.horizontalCenter: parent.horizontalCenter

                background: Rectangle {
                    anchors.fill: parent
                    color: Theme.card
                    radius: dp(25)
                }

                Label {
                    id: label
                    text: section
                    anchors.left: parent.left
                    anchors.leftMargin: dp(15)
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: dp(18)
                    font.italic: true
                    color: Theme.textPrimary
                }
            }
            model: txsModal
            delegate: transactionDelegate
            clip: true

            focus: true
        }
    }

    ListModel {
        id: txsModal
    }


    Component {
        id: transactionDelegate
        Rectangle {
            height: dp(72)
            width: container.width
            color: "#00000000"

            Rectangle {
                height: dp(70)
                color: "#00000000"
                width: container.width *0.9
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        const account = wallet.getCurrentAccountName()
                        if (account === "") //==|| index < 0 || index >= allTrans.length)
                            return
                        // respond will come at updateTransactionById
                        wallet.requestTransactionById(account, Number(txIdx).toString());
                        console.log("contx : ", txConfirmedStr)
                        transactionDetail.confirmation = txConfirmedStr
                        transactionDetail.state = "Visible"

                    }
                }

                Rectangle {
                    id: rec_img
                    height: parent.height*0.5
                    width: height
                    radius: dp(50)
                    color: txType === "Cancelled"? Theme.red : txType === "Unconfirmed"? Theme.awaiting : txCoinNano.includes("-")? Theme.textSecondary : Theme.confirmed
                    opacity: 0.3
                    anchors.left: parent.left
                    anchors.leftMargin: dp(20)
                    anchors.verticalCenter: parent.verticalCenter

                }

                ImageColor {
                    id: img_status
                    img_height: txType === "Cancelled"? rec_img.height * 0.3 : rec_img.height * 0.5
                    img_color: txType === "Cancelled"? "red" : txType === "Unconfirmed"? Theme.awaiting : txCoinNano.includes("-")? Theme.textSecondary : Theme.confirmed
                    img_source: getTxTypeIcon(txType)
                    img_rotation: txType === "Received"? 180 : 0
                    anchors.verticalCenter: rec_img.verticalCenter
                    anchors.horizontalCenter: rec_img.horizontalCenter
                }


                Text {
                    id: text_uuid
                    color: Theme.textPrimary
                    text: txId
                    font.pixelSize: dp(16)
                    anchors.left: rec_img.right
                    anchors.leftMargin: dp(20)
                    anchors.right: text_amountmwc.left
                    anchors.rightMargin: dp(40)
                    elide: Text.ElideMiddle
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: -dp(10)

                }
                Text {
                    id: text_status
                    color: Theme.textSecondary
                    font.pixelSize:  dp(14)
                    text: txType + " "  + txTime
                    anchors.left: text_uuid.left
                    anchors.top: text_uuid.bottom
                }

                Text {
                    id: text_amountmwc
                    color: txType === "Cancelled"? Theme.textSecondary : txType === "Unconfirmed"? Theme.awaiting : txCoinNano.includes("-")? Theme.textSecondary : Theme.confirmed
                    text: "%1 MWC".arg(hiddenAmount? hidden : txCoinNano)
                    font.pixelSize:  dp(16)
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: -dp(10)
                    anchors.rightMargin: dp(10)
                }
                Text {
                    id: text_amountprice
                    color: Theme.textSecondary
                    font.pixelSize:  dp(14)
                    text: "%1 %2".arg(hiddenAmount? hidden : (Number(txCoinNano)* currencyPrice).toFixed(currencyPriceRound)).arg(currencyTicker)
                    font.capitalization: Font.AllUppercase
                    anchors.right: parent.right
                    anchors.top: text_amountmwc.bottom
                    anchors.rightMargin: dp(10)
                }



            }
        }
    }
}
