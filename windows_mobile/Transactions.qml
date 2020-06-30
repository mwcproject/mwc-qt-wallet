import QtQuick 2.0
import QtQuick.Window 2.12
import WalletBridge 1.0
import TransactionsBridge 1.0
import ConfigBridge 1.0

Item {
    id: transactionsItem

    var allTrans = []
    var nodeHeight = 0
    var TRANSACTION_CANCELLED = 0x8000
    var TRANSACTION_COIN_BASE = 4
    var COIN_BASE_CONFIRM_NUMBER = 1440

    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    WalletBridge {
        id: wallet
    }

    TransactionsBridge {
        id: transaction
    }

    ConfigBridge {
        id: config
    }

    Connections {
        target: wallet
        onSgnWalletBalanceUpdated: {

        }
    }

    onVisibleChanged: {
        if (visible) {
           requestTransactions()
           updateData()
        }
    }

    function requestTransactions() {
        transactionModel.clear()
        const account = wallet.getCurrentAccountName()
        if (account.length === 0) {
            return
        }
        wallet.requestNodeStatus()
        wallet.requestTransactions(account, true)
        updateData()
    }

    function updateData() {
        const expectedConfirmNumber = config.getInputConfirmationNumber()
        for ( let idx = allTrans.length - 1; idx >= 0; idx--) {
            const trans = allTrans[idx]

            const selection = 0.0

            if ( canBeCancelled(trans) ) {
                const age = calculateTransactionAge(trans)
                // 1 hours is a 1.0
                selection = age > 60 * 60 ?
                            1.0 : (Number(age) / Number(60 * 60))
            }

            const transConfirmedStr = trans.confirmed ? "YES" : "NO"
            // if the node is online and in sync, display the number of confirmations instead
            // nodeHeight will be 0 if the node is offline or out of sync
            if (nodeHeight > 0 && trans.height > 0) {
                const needConfirms = isCoinBase(trans) ? COIN_BASE_CONFIRM_NUMBER : expectedConfirmNumber
                // confirmations are 1 more than the difference between the node and transaction heights
                const confirmations = nodeHeight - trans.height + 1
                transConfirmedStr = Number(confirmations).toString()
                if (needConfirms >= confirmations) {
                    transConfirmedStr += "/" + Number(needConfirms).toString()
                }
            }

            transactionModel.append({
                txNum: Number(trans.txIdx+1).toString(),
                txType: getTypeAsStr(trans),
                txId: trans.txid,
                txAddress: trans.address,
                txTime: trans.creationTime,
                txCoinNano: trans.coinNano,
                txConfirmedStr: transConfirmedStr,
                txHeight: trans.height <= 0 ? "" : Number(trans.height).toString()
            })
        }
    }

    function canBeCancelled(trans) {
        return (trans.transactionType & TRANSACTION_CANCELLED) === 0 && !trans.confirmed
    }

    function calculateTransactionAge(trans) {
        const now = new Date()
        const creationTime = new Date(trans.creationTime)
        return (now.getTime() - creationTime.getTime()) / 1000
    }

    function isCoinBase(trans) {
        return trans.transactionType === TRANSACTION_COIN_BASE
    }

    ListModel {
        id: transactionModel
        ListElement {
            txType: "Sent"
            txTime: "Jun 25, 2020  /  12:57pm"
            txBalance: "-90 MWC"
            txId: "ID: 49570294750498750249875049875"
            txUrl: "https://tokok.co/coin/mwc/7295hghhgh4bdfb654"
        }
    }

    ListView {
        anchors.fill: parent
        anchors.top: parent.top
        model: transactionModel
        delegate: transactionDelegate
        focus: true
    }

    Component {
        id: transactionDelegate
        Rectangle {
            height: dp(215)
            color: "#00000000"
            anchors.left: parent.left
            anchors.right: parent.right

            Rectangle {
                height: dp(200)
                color: "#33bf84ff"
                anchors.top: parent.top
                anchors.topMargin: dp(15)
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                anchors.left: parent.left
                anchors.leftMargin: dp(20)

                Image {
                    width: dp(17)
                    height: dp(17)
                    anchors.top: parent.top
                    anchors.topMargin: dp(35)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    fillMode: Image.PreserveAspectFit
                    source: txType === "Sent" ? "../img/Transactions_Sent@2x.svg" : "../img/Transactions_Received@2x.svg"
                }

                Text {
                    color: "#ffffff"
                    text: txType
                    font.bold: true
                    font.pixelSize: dp(15)
                    anchors.top: parent.top
                    anchors.topMargin: dp(37)
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
                    anchors.topMargin: dp(37)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(35)
                }

                Rectangle {
                    height: dp(1)
                    color: "#ffffff"
                    anchors.top: parent.top
                    anchors.topMargin: dp(70)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(35)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                }

                Text {
                    color: "#ffffff"
                    text: txBalance
                    font.bold: true
                    anchors.top: parent.top
                    anchors.topMargin: dp(90)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    font.pixelSize: dp(15)
                }

                Text {
                    color: "#ffffff"
                    text: txId
                    anchors.top: parent.top
                    anchors.topMargin: dp(120)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    font.pixelSize: dp(15)
                }

                Text {
                    color: "#ffffff"
                    text: txUrl
                    anchors.top: parent.top
                    anchors.topMargin: dp(150)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    font.pixelSize: dp(15)
                }
            }
        }
    }
}
