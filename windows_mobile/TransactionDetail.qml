import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import WalletBridge 1.0
import ConfigBridge 1.0
import UtilBridge 1.0

Item {
    id: transactionDetail
    visible: false

    property var tx2process
    property var outputs
    property string blockExplorerUrl
    property string originalTransactionNote
    property string txUuid
    property var type_TRANSACTION_CANCELLED: 0x8000
    property var type_TRANSACTION_COIN_BASE: 4
    property var type_TRANSACTION_RECEIVE: 2
    property var type_TRANSACTION_SEND: 1
    property var locale: Qt.locale()

    ConfigBridge {
        id: config
    }

    UtilBridge {
        id: util
    }

    WalletBridge {
        id: wallet
    }

    Connections {
        target: wallet
        onSgnExportProofResult: (success, fn, msg) => {
            if (success) {
                const proof = showProofDlg.parseProofText(msg)
                if (proof) {
                    showProofDlg.open(fn, proof, true)
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

    function init(account, txinfo, _outputs, messages, txnNote) {
        outputs = _outputs
        txUuid = txinfo.txid
        blockExplorerUrl = config.getBlockExplorerUrl(config.getNetwork());
        image_txtype.source = getTxTypeIcon(txinfo.transactionType, txinfo.confirmed)
        text_txtype_amount.text = getTypeAsStr(txinfo.transactionType) + " " + util.nano2one(txinfo.coinNano) + " MWC"
        text_txdate.text = getTxTime(txinfo.creationTime)
        text_confirmed.text = txinfo.confirmed ? "Yes" : "No"
        text_txid.text = txinfo.txid
        text_address.text = txinfo.address

        let strMessage = "None";
        if (messages.length) {
            strMessage = messages[0]
            for (let t = 1; t < messages.length; t++) {
                strMessage += "; "
                strMessage += messages[t]
            }
        }
        text_message.text = strMessage

        textfield_note.text = txnNote
        originalTransactionNote = txnNote
        text_kernel.text = txinfo.kernel
        text_inputs.text = Number(txinfo.numInputs).toString()
        text_debited.text = util.nano2one(txinfo.debited)
        text_outputs.text = Number(txinfo.numOutputs).toString()
        text_credited.text = util.nano2one(txinfo.credited)
        text_txfee.text = util.nano2one(txinfo.fee)

        list_commitments.clear()
        for (let i = 0; i < outputs.length; i++) {
            let commitType = ""
            if (i < txinfo.numInputs) {
                commitType = "Input " + Number(i+1).toString() + ": ";
            }
            else {
                commitType = "Output " + Number(i-txinfo.numInputs+1) + ": ";
            }

            list_commitments.append({ value: commitType + JSON.parse(outputs[i]).outputCommitment });
        }

        // Selecting first output
        if (txinfo.numInputs < outputs.length) {
            combobox_commitments.currentIndex = txinfo.numInputs
            updateOutputData()
        } else {
            combobox_commitments.currentIndex = -1
        }

        const amount = util.nano2one(txinfo.coinNano)

        if (canBeCancelled(txinfo.transactionType, txinfo.confirmed)) {
            button_tx_cancel.visible = true
        } else {
            button_tx_cancel.visible = false
        }

        if (txinfo.transactionType === type_TRANSACTION_SEND && !txinfo.confirmed) {
            button_tx_repost.visible = true
        } else {
            button_tx_repost.visible = false
        }

        if (txinfo.proof) {
            button_tx_proof.visible = true
        } else {
            button_tx_proof.visible = false
        }

        tx2process = txinfo
    }

    function canBeCancelled(transactionType, confirmed) {
        return (transactionType & type_TRANSACTION_CANCELLED) === 0 && !confirmed
    }

    function getTxTypeIcon(transactionType, confirmed) {
        if ( transactionType & type_TRANSACTION_CANCELLED )
            return "../img/Transactions_Cancelled_Blue@2x.svg"

        if ( !confirmed )
            return "../img/Transactions_Unconfirmed_Blue@2x.svg"

        if ( transactionType & type_TRANSACTION_SEND )
            return "../img/Transactions_Sent_Blue@2x.svg"

        if ( transactionType & type_TRANSACTION_RECEIVE )
            return "../img/Transactions_Received_Blue@2x.svg"

        if ( transactionType & type_TRANSACTION_COIN_BASE )
            return "../img/Transactions_CoinBase_Blue@2x.svg"
    }

    function getTypeAsStr(transactionType) {
        if ( transactionType & type_TRANSACTION_SEND )
            return "Sent"

        if ( transactionType & type_TRANSACTION_RECEIVE )
            return "Received"

        if ( transactionType & type_TRANSACTION_COIN_BASE )
            return "CoinBase"
    }

    function getTxTime(creationTime) {
        const date = Date.fromLocaleString(locale, creationTime, "hh:mm:ss dd-MM-yyyy")
        return date.toLocaleString(locale, "MMM dd, yyyy")
    }

    function saveTransactionNote(newNote) {
        if (originalTransactionNote !== newNote) {
            originalTransactionNote = newNote
            if (newNote === "") {
                config.deleteTxNote(txUuid)
            } else {
                config.updateTxNote(txUuid, newNote)
            }
        }
    }

    function updateOutputData() {
        if (combobox_commitments.currentIndex < 0) {
            label_status.visible = false
            text_status.visible = false
            label_mwc.visible = false
            text_mwc.visible = false
            label_height.visible = false
            text_height.visible = false
            label_confirms.visible = false
            text_confirms.visible = false
            label_coinbase.visible = false
            text_coinbase.visible = false
            label_txnum.visible = false
            text_txnum.visible = false
            return
        }

        const out = JSON.parse(outputs[combobox_commitments.currentIndex])
        label_status.visible = true
        text_status.visible = true
        text_status.text = out.status
        label_mwc.visible = true
        text_mwc.visible = true
        text_mwc.text = util.nano2one(out.valueNano)
        label_height.visible = true
        text_height.visible = true
        text_height.text = out.blockHeight
        label_confirms.visible = true
        text_confirms.visible = true
        text_confirms.text = out.numOfConfirms
        label_coinbase.visible = true
        text_coinbase.visible = true
        text_coinbase.text = out.coinbase ? "Yes" : "No"
        label_txnum.visible = true
        text_txnum.visible = true
        text_txnum.text = out.txIdx < 0 ? "None" : Number(out.txIdx+1).toString()
    }

    function txCancelCallback(ok) {
        if(ok) {
            transactionsItem.txCancel(tx2process.txIdx)
            transactionDetail.visible = false
        }
    }

    onVisibleChanged: {
        if (visible) {
            view_txinfo.contentItem.contentY = 0
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#00000000"

        MouseArea {
            anchors.fill: parent
        }
    }

    Rectangle {
        anchors.fill: parent
        anchors.topMargin: dp(100)
        anchors.leftMargin: dp(30)
        anchors.rightMargin: dp(30)
        anchors.bottomMargin: dp(90)

        Rectangle {
            anchors.fill: parent
            color: "white"

            Rectangle {
                id: rect_header
                height: dp(110)
                color: "#ffffff"
                anchors.top: parent.top
                anchors.topMargin: 0
                anchors.right: parent.right
                anchors.rightMargin: 0
                anchors.left: parent.left
                anchors.leftMargin: 0

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        textfield_note.focus = false
                    }
                }

                Image {
                    id: image_txtype
                    width: dp(20)
                    height: dp(20)
                    anchors.verticalCenter: text_txtype_amount.verticalCenter
                    anchors.right: text_txtype_amount.left
                    anchors.rightMargin: dp(5)
                    source: "../img/Transactions_Sent_Blue@2x.svg"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    id: text_txtype_amount
                    color: "#3600c9"
                    text: qsTr("Sent 90 MWC")
                    anchors.top: parent.top
                    anchors.topMargin: dp(25)
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.bold: true
                    font.pixelSize: dp(20)
                }

                Text {
                    id: text_txdate
                    color: "#3600c9"
                    text: qsTr("Jun 25, 2020")
                    anchors.top: text_txtype_amount.bottom
                    anchors.topMargin: dp(10)
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: dp(20)
                }

                Image {
                    id: image_close
                    width: dp(38)
                    height: dp(38)
                    anchors.top: parent.top
                    anchors.topMargin: dp(25)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(25)
                    source: "../img/MessageBox_Close@2x.svg"
                    fillMode: Image.PreserveAspectFit

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            transactionDetail.visible = false
                        }
                    }
                }
            }

            ScrollView {
                id: view_txinfo
                clip: true
                ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                contentHeight: rect_output.visible ? dp(830) + text_message.height : dp(720) + text_message.height
                anchors.top: rect_header.bottom
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.left: parent.left

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        textfield_note.focus = false
                    }
                }

                Text {
                    id: label_confirmed
                    color: "#3600c9"
                    text: qsTr("Confirmed")
                    font.bold: true
                    anchors.left: parent.left
                    anchors.leftMargin: dp(20)
                    anchors.top: parent.top
                    anchors.topMargin: 0
                    font.pixelSize: dp(15)
                }

                Text {
                    id: text_confirmed
                    color: "#3600c9"
                    text: qsTr("Yes")
                    anchors.left: parent.left
                    anchors.leftMargin: dp(20)
                    anchors.top: label_confirmed.bottom
                    anchors.topMargin: dp(3)
                    font.pixelSize: dp(15)
                }

                Button {
                    id: button_tx_cancel
                    height: dp(35)
                    width: dp(80)
                    anchors.right: button_tx_repost.visible ? button_tx_repost.left : parent.right
                    anchors.rightMargin: dp(20)
                    anchors.bottom: text_confirmed.bottom
                    visible: false
                    background: Rectangle {
                        color: "#00000000"
                        radius: dp(4)
                        border.color: "#3600C9"
                        border.width: dp(2)
                        Text {
                            text: qsTr("Cancel")
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            font.pixelSize: dp(14)
                            color: "#3600C9"
                        }
                    }

                    onClicked: {
                        messagebox.open("Transaction cancellation", "Are you sure you want to cancel transaction #" +
                                        Number(tx2process.txIdx + 1).toString() +
                                        ", TXID " + tx2process.txid, true, "No", "Yes", "", "", "", txCancelCallback)
                    }
                }

                Button {
                    id: button_tx_repost
                    height: dp(35)
                    width: dp(80)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(20)
                    anchors.bottom: text_confirmed.bottom
                    visible: false
                    background: Rectangle {
                        color: "#00000000"
                        radius: dp(4)
                        border.color: "#3600C9"
                        border.width: dp(2)
                        Text {
                            text: qsTr("Repost")
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            font.pixelSize: dp(14)
                            color: "#3600C9"
                        }
                    }

                    onClicked: {
                        transactionsItem.txRepost(tx2process.txIdx)
                        transactionDetail.visible = false
                    }
                }

                Button {
                    id: button_tx_proof
                    height: dp(35)
                    width: dp(80)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(20)
                    anchors.bottom: text_confirmed.bottom
                    visible: false
                    background: Rectangle {
                        color: "#00000000"
                        radius: dp(4)
                        border.color: "#3600C9"
                        border.width: dp(2)
                        Text {
                            text: qsTr("Proof")
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            font.pixelSize: dp(14)
                            color: "#3600C9"
                        }
                    }

                    onClicked: {
                        if (!tx2process.proof) {
                            messagebox.open("Need info", "Please select qualify transaction to generate a proof.")
                        } else {
//                            const fileName = util.getSaveFileName("Create transaction proof file", "Transactions", "transaction proof (*.proof)", ".proof")
//                            if (fileName === "") return
                            const fileName = "/storage/emulated/0/Android/data/mw.mwc.wallet/files/Download/" + tx2process.txid + ".proof"
                            wallet.generateTransactionProof(Number(tx2process.txIdx).toString(), fileName)
                        }
                    }
                }

                Text {
                    id: label_txid
                    color: "#3600c9"
                    text: qsTr("tx ID")
                    anchors.left: parent.left
                    anchors.topMargin: dp(20)
                    font.bold: true
                    anchors.top: text_confirmed.bottom
                    font.pixelSize: dp(15)
                    anchors.leftMargin: dp(20)
                }

                Text {
                    id: text_txid
                    color: "#3600c9"
                    text: qsTr("49570294750498750249875049875")
                    anchors.left: parent.left
                    anchors.topMargin: dp(3)
                    anchors.top: label_txid.bottom
                    font.pixelSize: dp(15)
                    anchors.leftMargin: dp(20)
                }

                Text {
                    id: label_address
                    color: "#3600c9"
                    text: qsTr("address")
                    anchors.left: parent.left
                    anchors.topMargin: dp(20)
                    font.bold: true
                    anchors.top: text_txid.bottom
                    anchors.leftMargin: dp(20)
                    font.pixelSize: dp(15)
                }

                Text {
                    id: text_address
                    color: "#3600c9"
                    text: qsTr("https://tokok.co/coin/mwc/7295hghhgh4bdfb65")
                    elide: Text.ElideMiddle
                    anchors.left: parent.left
                    anchors.topMargin: dp(3)
                    anchors.top: label_address.bottom
                    anchors.leftMargin: dp(20)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(20)
                    font.pixelSize: dp(15)
                }

                Text {
                    id: text_message
                    color: "#3600c9"
                    text: qsTr("This is a longer message that goes onto multiple lines...")
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    anchors.right: parent.right
                    anchors.rightMargin: dp(30)
                    anchors.left: parent.left
                    anchors.topMargin: dp(3)
                    anchors.top: label_message.bottom
                    font.pixelSize: dp(15)
                    anchors.leftMargin: dp(20)
                }

                Text {
                    id: label_message
                    color: "#3600c9"
                    text: qsTr("message")
                    anchors.left: parent.left
                    anchors.topMargin: dp(20)
                    font.bold: true
                    anchors.top: text_address.bottom
                    font.pixelSize: dp(15)
                    anchors.leftMargin: dp(20)
                }

                Text {
                    id: label_note
                    color: "#3600c9"
                    text: qsTr("Transaction Note")
                    anchors.left: parent.left
                    anchors.topMargin: dp(20)
                    font.bold: true
                    anchors.top: text_message.bottom
                    anchors.leftMargin: dp(20)
                    font.pixelSize: dp(15)
                }

                Text {
                    id: label_note_addition
                    color: "#3600c9"
                    text: qsTr("Notes are private and saved locally, only visible to you.")
                    anchors.left: parent.left
                    anchors.topMargin: dp(3)
                    anchors.top: label_note.bottom
                    font.pixelSize: dp(15)
                    anchors.leftMargin: dp(20)
                }

                TextField {
                    id: textfield_note
                    height: dp(44)
                    padding: dp(10)
                    leftPadding: dp(20)
                    font.pixelSize: dp(15)
                    placeholderText: qsTr("note")
                    color: "#3600C9"
                    text: ""
                    anchors.right: parent.right
                    anchors.rightMargin: dp(20)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(20)
                    anchors.top: label_note_addition.bottom
                    anchors.topMargin: dp(10)
                    horizontalAlignment: Text.AlignLeft


                    background: Rectangle {
                        color: "white"
                        border.color: "#E2CCF7"
                        border.width: dp(2)
                        radius: dp(4)
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            textfield_note.forceActiveFocus()
                        }
                    }
                }

                Text {
                    id: label_kernel
                    color: "#3600c9"
                    text: qsTr("Transaction Kernel")
                    anchors.left: parent.left
                    anchors.topMargin: dp(20)
                    font.bold: true
                    anchors.top: textfield_note.bottom
                    font.pixelSize: dp(15)
                    anchors.leftMargin: dp(20)
                }

                Text {
                    id: text_kernel
                    color: "#3600c9"
                    text: qsTr("dlfladfladufladnfladnfalfnaldfkdjshfkna3...")
                    elide: Text.ElideRight
                    anchors.right: parent.right
                    anchors.rightMargin: dp(110)
                    anchors.left: parent.left
                    anchors.topMargin: dp(3)
                    anchors.top: label_kernel.bottom
                    anchors.leftMargin: dp(20)
                    font.pixelSize: dp(15)
                }

                Button {
                    id: button_kernel_view
                    height: dp(35)
                    width: dp(80)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(20)
                    anchors.bottom: text_kernel.bottom
                    background: Rectangle {
                        color: "#00000000"
                        radius: dp(4)
                        border.color: "#3600C9"
                        border.width: dp(2)
                        Text {
                            text: qsTr("View")
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            font.pixelSize: dp(14)
                            color: "#3600C9"
                        }
                    }

                    onClicked: {
                        Qt.openUrlExternally("https://" + blockExplorerUrl + "/#k" + text_kernel.text)
                    }
                }

                Text {
                    id: label_inputs
                    color: "#3600c9"
                    text: qsTr("Inputs")
                    anchors.left: parent.left
                    anchors.topMargin: dp(20)
                    font.bold: true
                    anchors.top: text_kernel.bottom
                    anchors.leftMargin: dp(20)
                    font.pixelSize: dp(15)
                }

                Text {
                    id: text_inputs
                    color: "#3600c9"
                    text: qsTr("2")
                    anchors.left: parent.left
                    anchors.topMargin: dp(3)
                    anchors.top: label_inputs.bottom
                    font.pixelSize: dp(15)
                    anchors.leftMargin: dp(20)
                }

                Text {
                    id: label_debited
                    color: "#3600c9"
                    text: qsTr("Debited")
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.horizontalCenterOffset: -label_debited.width / 2
                    anchors.topMargin: dp(20)
                    font.bold: true
                    anchors.top: text_kernel.bottom
                    font.pixelSize: dp(15)
                }

                Text {
                    id: text_debited
                    color: "#3600c9"
                    text: qsTr("190 MWC")
                    anchors.left: label_debited.left
                    anchors.topMargin: dp(3)
                    anchors.top: label_debited.bottom
                    font.pixelSize: dp(15)
                }

                Text {
                    id: label_outputs
                    color: "#3600c9"
                    text: qsTr("Change Outputs")
                    anchors.right: parent.right
                    anchors.rightMargin: dp(40)
                    anchors.topMargin: dp(20)
                    font.bold: true
                    anchors.top: text_kernel.bottom
                    font.pixelSize: dp(15)
                }

                Text {
                    id: text_outputs
                    color: "#3600c9"
                    text: qsTr("1")
                    anchors.left: label_outputs.left
                    anchors.topMargin: dp(3)
                    anchors.top: label_outputs.bottom
                    font.pixelSize: dp(15)
                }

                Text {
                    id: label_credited
                    color: "#3600c9"
                    text: qsTr("Credited")
                    anchors.left: parent.left
                    anchors.topMargin: dp(20)
                    font.bold: true
                    anchors.top: text_inputs.bottom
                    font.pixelSize: dp(15)
                    anchors.leftMargin: dp(20)
                }

                Text {
                    id: text_credited
                    color: "#3600c9"
                    text: qsTr("100 MWC")
                    anchors.left: parent.left
                    anchors.topMargin: dp(3)
                    anchors.top: label_credited.bottom
                    anchors.leftMargin: dp(20)
                    font.pixelSize: dp(15)
                }

                Text {
                    id: label_txfee
                    color: "#3600c9"
                    text: qsTr("Tx Fee")
                    anchors.left: label_debited.left
                    anchors.topMargin: dp(20)
                    font.bold: true
                    anchors.top: text_inputs.bottom
                    font.pixelSize: dp(15)
                }

                Text {
                    id: text_txfee
                    color: "#3600c9"
                    text: qsTr(".005 MWC")
                    anchors.left: label_txfee.left
                    anchors.topMargin: dp(3)
                    anchors.top: label_txfee.bottom
                    font.pixelSize: dp(15)
                }

                Rectangle {
                    id: rect_splitter
                    height: dp(2)
                    color: "#e2ccf7"
                    anchors.top: text_txfee.bottom
                    anchors.topMargin: dp(30)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(50)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(50)
                }

                Text {
                    id: label_commitments
                    color: "#3600c9"
                    text: qsTr("Input/Output Commitments")
                    anchors.left: parent.left
                    anchors.topMargin: dp(30)
                    font.bold: true
                    anchors.top: rect_splitter.bottom
                    anchors.leftMargin: dp(20)
                    font.pixelSize: dp(15)
                }

                ComboBox {
                    id: combobox_commitments

                    onCurrentIndexChanged: {
                        if (combobox_commitments.currentIndex >= 0) {
                            updateOutputData()
                        }
                    }

                    delegate: ItemDelegate {
                        width: combobox_commitments.width
                        contentItem: Text {
                            text: value
                            color: "#3600C9"
                            font: combobox_commitments.font
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                        }
                        highlighted: combobox_commitments.highlightedIndex === index
                        topPadding: dp(10)
                        bottomPadding: dp(10)
                        leftPadding: dp(20)
                        rightPadding: dp(30)
                    }

                    indicator: Canvas {
                        id: canvas
                        x: combobox_commitments.width - width - dp(10)
                        y: combobox_commitments.topPadding + (combobox_commitments.availableHeight - height) / 2
                        width: dp(14)
                        height: dp(7)
                        contextType: "2d"

                        Connections {
                            target: combobox_commitments
                            function onPressedChanged() { canvas.requestPaint() }
                        }

                        onPaint: {
                            context.reset()
                            if (combobox_commitments.popup.visible) {
                                context.moveTo(0, height)
                                context.lineTo(width / 2, 0)
                                context.lineTo(width, height)
                            } else {
                                context.moveTo(0, 0)
                                context.lineTo(width / 2, height)
                                context.lineTo(width, 0)
                            }
                            context.fillStyle = "#3600C9"
                            context.fill()
                        }
                    }

                    contentItem: Text {
                        text: combobox_commitments.currentIndex >= 0 ? list_commitments.get(combobox_commitments.currentIndex).value : ""
                        font: combobox_commitments.font
                        color: "#3600C9"
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }

                    background: Rectangle {
                        implicitHeight: dp(40)
                        radius: dp(4)
                        border.color: "#E2CCF7"
                        border.width: dp(2)
                        color: "white"
                    }

                    popup: Popup {
                        y: combobox_commitments.height - 1
                        width: combobox_commitments.width
                        implicitHeight: contentItem.implicitHeight
                        padding: dp(1)

                        contentItem: ListView {
                            clip: true
                            implicitHeight: contentHeight
                            model: combobox_commitments.popup.visible ? combobox_commitments.delegateModel : null
                            currentIndex: combobox_commitments.highlightedIndex

                            ScrollIndicator.vertical: ScrollIndicator { }
                        }

                        background: Rectangle {
                            color: "white"
                            border.color: "#3600C9"
                            border.width: dp(2)
                        }

                        onVisibleChanged: {
                            if (!combobox_commitments.popup.visible) {
                                canvas.requestPaint()
                            }
                        }
                    }

                    model: ListModel {
                        id: list_commitments
                    }
                    anchors.top: label_commitments.bottom
                    anchors.topMargin: dp(10)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(110)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(20)
                    leftPadding: dp(20)
                    rightPadding: dp(20)
                    font.pixelSize: dp(15)
                }

                Button {
                    id: button_commitments_view
                    height: dp(35)
                    width: dp(80)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(20)
                    anchors.verticalCenter: combobox_commitments.verticalCenter
                    background: Rectangle {
                        color: "#00000000"
                        radius: dp(4)
                        border.color: "#3600C9"
                        border.width: dp(2)
                        Text {
                            text: qsTr("View")
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            font.pixelSize: dp(14)
                            color: "#3600C9"
                        }
                    }

                    onClicked: {
                        if (combobox_commitments.currentIndex >= 0) {
                            Qt.openUrlExternally("https://" + blockExplorerUrl + "/#o" + JSON.parse(outputs[combobox_commitments.currentIndex]).outputCommitment )
                        }
                    }
                }

                Rectangle {
                    id: rect_output
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: combobox_commitments.bottom
                    anchors.topMargin: dp(30)
                    height: dp(90)
                    visible: combobox_commitments.currentIndex >= 0

                    Text {
                        id: label_status
                        color: "#3600c9"
                        text: qsTr("Status")
                        anchors.left: parent.left
                        font.bold: true
                        anchors.top: parent.top
                        anchors.leftMargin: dp(20)
                        font.pixelSize: dp(15)
                    }

                    Text {
                        id: text_status
                        color: "#3600c9"
                        text: qsTr("Unspent")
                        anchors.left: parent.left
                        anchors.topMargin: dp(3)
                        anchors.top: label_status.bottom
                        font.pixelSize: dp(15)
                        anchors.leftMargin: dp(20)
                    }

                    Text {
                        id: label_mwc
                        color: "#3600c9"
                        text: qsTr("MWC")
                        font.bold: true
                        anchors.top: parent.top
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.horizontalCenterOffset: -label_debited.width / 2
                        font.pixelSize: dp(15)
                    }

                    Text {
                        id: text_mwc
                        color: "#3600c9"
                        text: qsTr("858.4443333332")
                        anchors.left: label_mwc.left
                        anchors.topMargin: dp(3)
                        anchors.top: label_mwc.bottom
                        font.pixelSize: dp(15)
                    }

                    Text {
                        id: label_height
                        color: "#3600c9"
                        text: qsTr("Height")
                        anchors.right: parent.right
                        anchors.rightMargin: dp(70)
                        font.bold: true
                        anchors.top: parent.top
                        font.pixelSize: dp(15)
                    }

                    Text {
                        id: text_height
                        color: "#3600c9"
                        text: qsTr("345345")
                        anchors.left: label_height.left
                        anchors.topMargin: dp(3)
                        anchors.top: label_height.bottom
                        font.pixelSize: dp(15)
                    }

                    Text {
                        id: label_confirms
                        color: "#3600c9"
                        text: qsTr("Confirms")
                        anchors.left: parent.left
                        anchors.topMargin: dp(20)
                        font.bold: true
                        anchors.top: text_status.bottom
                        font.pixelSize: dp(15)
                        anchors.leftMargin: dp(20)
                    }

                    Text {
                        id: text_confirms
                        color: "#3600c9"
                        text: qsTr("1235")
                        anchors.left: parent.left
                        anchors.topMargin: dp(3)
                        anchors.top: label_confirms.bottom
                        anchors.leftMargin: dp(20)
                        font.pixelSize: dp(15)
                    }

                    Text {
                        id: label_coinbase
                        color: "#3600c9"
                        text: qsTr("Coinbase")
                        anchors.left: label_mwc.left
                        anchors.topMargin: dp(20)
                        font.bold: true
                        anchors.top: text_status.bottom
                        font.pixelSize: dp(15)
                    }

                    Text {
                        id: text_coinbase
                        color: "#3600c9"
                        text: qsTr("No")
                        anchors.left: label_coinbase.left
                        anchors.topMargin: dp(3)
                        anchors.top: label_coinbase.bottom
                        font.pixelSize: dp(15)
                    }

                    Text {
                        id: label_txnum
                        color: "#3600c9"
                        text: qsTr("Tx#")
                        anchors.left: label_height.left
                        anchors.topMargin: dp(20)
                        font.bold: true
                        anchors.top: text_status.bottom
                        font.pixelSize: dp(15)
                    }

                    Text {
                        id: text_txnum
                        color: "#3600c9"
                        text: qsTr("14")
                        anchors.left: label_txnum.left
                        anchors.topMargin: dp(3)
                        anchors.top: label_txnum.bottom
                        font.pixelSize: dp(15)
                    }
                }

                Button {
                    id: button_ok
                    height: dp(40)
                    width: dp(135)
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: rect_output.visible ? rect_output.bottom : combobox_commitments.bottom
                    anchors.topMargin: dp(30)
                    background: Rectangle {
                        color: "#6F00D6"
                        radius: dp(4)
                        Text {
                            text: qsTr("OK")
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            font.pixelSize: dp(15)
                            color: "white"
                        }
                    }

                    onClicked: {
                        saveTransactionNote(textfield_note.text)
                        transactionDetail.visible = false
                    }
                }
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:700;width:400}D{i:5;anchors_y:63}D{i:2;anchors_width:200;anchors_x:56;anchors_y:71}
D{i:6;anchors_height:200;anchors_width:200;anchors_x:100;anchors_y:290}
}
##^##*/
