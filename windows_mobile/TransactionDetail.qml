import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.0
import QtQuick.Layouts 1.15
import WalletBridge 1.0
import ConfigBridge 1.0
import UtilBridge 1.0
import Clipboard 1.0
import QtAndroidService 1.0
import "./models"

Item {
    id: root

    property int confirmation
    property int nbCommit
    visible: false
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.verticalCenter: parent.verticalCenter
    height: parent.height
    width: parent.width

    /*states: [
        State{
            name: "Visible"
            PropertyChanges{target: root; opacity: 1.0; x: 0}
            PropertyChanges{target: root; visible: true}


        },
        State{
            name:"Invisible"
            PropertyChanges{target: root; opacity: 0.0; x: height}
            PropertyChanges{target: root; visible: false}
        }
    ]

    transitions: [
            Transition {
                from: "Visible"
                to: "Invisible"

                SequentialAnimation{
                   PropertyAnimation {
                       target: root
                       properties: "x, y, opacity"
                       duration: 2500
                       easing.type: Easing.InOutQuad
                   }
                   NumberAnimation {
                       target: root
                       property: "visible"
                       duration: 0
                   }
                }
            },
            Transition {
                from: "Invisible"
                to: "Visible"

                SequentialAnimation{
                   NumberAnimation {
                       target: root
                       property: "visible"
                       duration: 0
                   }
                   PropertyAnimation {
                       target: root
                       properties: "x, y, opacity"
                       duration: 2500
                       easing.type: Easing.InOutQuad
                   }
                }

            }
        ]*/



    property var downloadPath
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

    Clipboard {
        id: clipboard
    }

    UtilBridge {
        id: util
    }

    WalletBridge {
        id: wallet
    }

    QtAndroidService {
        id: qtAndroidService
    }

    Connections {
        target: wallet
        onSgnExportProofResult: (success, fn, msg) => {
            console.log("Get onSgnVerifyProofResult with " + success + " " + fn  + " " + msg)
            if (success) {
                const proof = showProofDlg.parseProofText(msg)
                if (proof) {
                    showProofDlg.open(fn, proof, true)
                } else {
                    messagebox.open("Failure", "Internal error. Unable to decode the results of the proof located at  " + fn + "\n\n" + msg)
                }
            } else {
                messagebox.open("Failure", msg)
            }
        }
    }

    Connections {
        target: qtAndroidService
        onSgnOnFileReady: (eventCode, path) => {
            if (eventCode === 201 && path) {
                console.log("fileName to save proof: " + path)
                const validation = util.validateMwc713Str(path)
                if (validation) {
                    messagebox.open(qsTr("File Path"), qsTr("This file path is not acceptable.\n" + validation))
                    return
                }
                wallet.generateTransactionProof(Number(tx2process.txIdx).toString(), path)
            }
        }
    }

    function getTxStatus(transactionType, confirmed, txHeight) {
        if (transactionType & type_TRANSACTION_CANCELLED) {
            text_confirmation.color = Theme.red
            return qsTr("Cancelled")
        }
        if (!confirmed) {
            text_confirmation.color = Theme.orange
            return qsTr("Unconfirmed")
        }
        if (transactionType & type_TRANSACTION_SEND || transactionType & type_TRANSACTION_RECEIVE) {
            text_confirmation.color = Theme.green
            return qsTr("Confirmed") + " (%1)".arg(walletItem.heightBlock + 1 - txHeight)
        }
        if (transactionType & type_TRANSACTION_COIN_BASE) {
            if (walletItem.heightBlock + 1 - txHeight < 1440) {
                text_confirmation.color = Theme.orange
                return qsTr("Confirmed") + " (%1<1440)".arg(walletItem.heightBlock + 1 - txHeight)
            }
            text_confirmation.color = Theme.green
            return qsTr("Confirmed") + " (%1)".arg(walletItem.heightBlock + 1 - txHeight)

        }
    }

    function init(account, txinfo, _outputs, messages, txnNote) {
        outputs = _outputs
        txUuid = txinfo.txid
        blockExplorerUrl = config.getBlockExplorerUrl(config.getNetwork());
        image_txtype.img_source = getTxTypeIcon(txinfo.transactionType, txinfo.confirmed)
        text_txdate.text = getTxTime(txinfo.creationTime)
        text_txdate.text = getTxTime(txinfo.creationTime)
        text_account.text = selectedAccount
        text_txid.text = txinfo.txid
        text_address.text = txinfo.address


        text_amount.text = getAmountAsStr(txinfo)

        let strMessage = "None";
        if (messages.length) {
            strMessage = messages[0]
            for (let t = 1; t < messages.length; t++) {
                strMessage += "; "
                strMessage += messages[t]
            }
        }

        text_confirmation.text = getTxStatus(txinfo.transactionType, txinfo.confirmed, txinfo.height)

        text_kernel.text = txinfo.kernel

        let addr = txinfo.address
        if (addr === "file") {
            label_address.text = "Method"
            text_address.text = "File"
            img_copy_slate.visible = false
        } else if (addr === "") {
            text_address.text  = "Method"
            text_address.text = "Slatepack"
            img_copy_slate.visible = false
        } else {
            text_address.text  = "Recipient Address"
            text_address.text = addr
            img_copy_slate.visible = true
        }

        let txFees = Number(util.nano2one(txinfo.fee))
        if (txFees === 0) {
            label_txfee.visible = false
            text_txfee.visible = false
            text_txfee_currency.visible = false
        } else {
            label_txfee.visible = true
            text_txfee.visible = true
            text_txfee_currency.visible = true
            text_txfee.text = txFees + " MWC"
            text_txfee_currency.text = "~ %1 %2".arg((txFees*currencyPrice).toFixed(currencyPriceRound)).arg(currencyTicker)
        }


        list_commitments.clear()
        for (let i = 0; i < outputs.length; i++) {
            let commitType = ""
            if (i < txinfo.numInputs) {
                commitType = "Input " + Number(i+1).toString() + ": ";
            } else {
                commitType = "Output " + Number(i-txinfo.numInputs+1) + ": ";
            }

            list_commitments.append({
                value: commitType + JSON.parse(outputs[i]).outputCommitment
            });
        }

        nbCommit = txinfo.numInputs + outputs.length

        if (nbCommit >0) {

            combobox_commitments.visible = true
            rect_output.visible = true
            label_commitments.visible = true

        } else {
            combobox_commitments.visible = false
            rect_output.visible = false
            label_commitments.visible = false

        }

        if (txinfo.transactionType & type_TRANSACTION_SEND) {
            combobox_commitments.currentIndex = 0
        }
        // Selecting first output
        else if (txinfo.numInputs < outputs.length) {
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
        if (transactionType & type_TRANSACTION_CANCELLED)
            return "../img/Transactions_Cancelled_Blue@2x.svg"

        if (!confirmed)
            return "../img/Transactions_Unconfirmed_Blue@2x.svg"

        if (transactionType & type_TRANSACTION_SEND)
            return "../img/Transactions_Sent_Blue@2x.svg"

        if (transactionType & type_TRANSACTION_RECEIVE)
            return "../img/Transactions_Received_Blue@2x.svg"

        if (transactionType & type_TRANSACTION_COIN_BASE)
            return "../img/Transactions_CoinBase_Blue@2x.svg"
    }

    function getAmountAsStr(txinfo) {
        if (txinfo.transactionType & type_TRANSACTION_SEND) {
            text_txtype.text = qsTr("Sent")
            text_currency.text = "- %1 %2".arg((Number(util.nano2one(txinfo.debited))*currencyPrice).toFixed(currencyPriceRound)).arg(currencyTicker)
            return "- " + util.nano2one(txinfo.debited) + " MWC"
        }
        if (txinfo.transactionType & type_TRANSACTION_RECEIVE) {
            text_txtype.text = qsTr("Received")
            text_currency.text = "%1 %2".arg((Number(util.nano2one(txinfo.credited))*currencyPrice).toFixed(currencyPriceRound)).arg(currencyTicker)
            return "" + util.nano2one(txinfo.credited) + " MWC"
        }
        if (txinfo.transactionType & type_TRANSACTION_COIN_BASE) {
            text_txtype.text = qsTr("Coinbase")
            text_currency.text = "%1 %2".arg((Number(util.nano2one(txinfo.credited))*currencyPrice).toFixed(currencyPriceRound)).arg(currencyTicker)
            return "" +util.nano2one(txinfo.credited) + " MWC"
        }
    }

    function getTxTime(creationTime) {
        const date = Date.fromLocaleString(locale, creationTime, "hh:mm:ss dd-MM-yyyy")
        return date.toLocaleString(locale, "MM/dd/yyyy, hh:mm")
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
        list_info_io.clear()
        if (combobox_commitments.currentIndex < 0) {
            for (let i = 0; i< 6; i++) {
                list_info_io.append({
                    label: "",
                    value: "",
                    visibility: false
                })
            }
            return
        }
        const out = JSON.parse(outputs[combobox_commitments.currentIndex])
        let label_list = [qsTr("Status"),qsTr("Amount"), qsTr("Height"), qsTr("Confirmation"), qsTr("Coinbase"), qsTr("Txs Index")]
        let value_list = [out.status, util.nano2one(out.valueNano),out.blockHeight, out.numOfConfirms, out.coinbase ? qsTr("Yes") : qsTr("No"), out.txIdx < 0 ? qsTr("None") : Number(out.txIdx+1).toString()]
        for (let i = 0; i< label_list.length; i++) {
            list_info_io.append({
                label: label_list[i],
                value: value_list [i],
                visibility: true
            })
        }
    }

    function txCancelCallback(ok) {
        if (ok) {
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
        color: Theme.gradientTop
        /*gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop {
                position: 0
                color: "#4d1d4f"
            }

            GradientStop {
                position: 0.3
                color: "#181818"
            }
        }*/
    }

    Rectangle {
        id: container
        height: parent.height
        width: parent.width
        anchors.bottom: parent.bottom
        color: Theme.bg

        Rectangle {
            id: rect_header
            height: parent.height/14
            color: Theme.gradientTop
            anchors.top: parent.top
            anchors.topMargin: 0
            anchors.right: parent.right
            anchors.rightMargin: 0
            anchors.left: parent.left
            anchors.leftMargin: 0
            ImageColor {
                id: image_return
                img_source: "../img/arrow.svg"
                img_height: parent.height/2
                img_color: "white"
                img_rotation: 180
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: dp(15)
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        root.visible = false
                    }
                }
            }

            Text {
                id: text_txtype
                color: "#ffffff"
                text: qsTr("Received")
                font.bold: true
                font.letterSpacing: dp(0.5)
                font.capitalization: Font.AllUppercase
                font.pixelSize: dp(17)
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        ScrollView {
            id: view_txinfo
            clip: true
            ScrollBar.vertical.policy: ScrollBar.AsNeeded
            contentHeight: (text_kernel.y - rec_img.y) + dp(80)
            anchors.top: rect_header.bottom
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.left: parent.left


            MouseArea {
                anchors.fill: parent
                onClicked: {
                    //textfield_note.focus = false
                }
            }

                Rectangle {
                    id: rec_img
                    anchors.top: parent.top
                    anchors.topMargin: dp(30)
                    height: dp(50)
                    width: height
                    radius: dp(150)
                    anchors.horizontalCenter: parent.horizontalCenter
                    ImageColor {
                        id: image_txtype
                        img_height: dp(10)
                        img_source: "../../img/Transactions_Sent_Blue@2x.svg"
                        anchors.top: parent.top
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }


                Text {
                    id: text_amount
                    color: "white"
                    text: "0 MWC"
                    anchors.top: rec_img.bottom
                    anchors.topMargin: dp(10)
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.bold: true
                    font.pixelSize: dp(17)
                }

                Text {
                    id: text_currency
                    color: "gray"
                    text: ""
                    font.capitalization: Font.AllUppercase
                    anchors.top: text_amount.bottom
                    anchors.topMargin: dp(2)
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: dp(14)
                }

                Text {
                    id: text_confirmation
                    color: Theme.green
                    text: qsTr("Confirmed")
                    anchors.top: text_currency.bottom
                    anchors.topMargin: dp(5)
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.bold: true
                    font.pixelSize: dp(17)
                }

                Text {
                    id: label_account

                    text: qsTr("Account")
                    color: "white"
                    font.bold: true
                    anchors.left: parent.left
                    anchors.leftMargin: dp(20)
                    anchors.top: text_confirmation.bottom
                    anchors.topMargin: dp(25)
                    font.pixelSize: dp(15)
                }

                Text {
                    id: text_account
                    color: "#ffffff"
                    text: "default"
                    anchors.left: parent.left
                    anchors.leftMargin: dp(20)
                    anchors.top: label_account.bottom
                    anchors.topMargin: dp(3)
                    font.pixelSize: dp(15)
                }

                Button {
                    id: button_tx_cancel
                    height: dp(35)
                    width: dp(80)
                    anchors.right: button_tx_repost.visible ? button_tx_repost.left : parent.right
                    anchors.rightMargin: dp(20)
                    anchors.bottom: text_account.bottom
                    visible: false
                    background: Rectangle {
                        color: "#00000000"
                        radius: dp(4)
                        border.color: "#ffffff"
                        border.width: dp(2)
                        Text {
                            text: qsTr("Cancel")
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            font.pixelSize: dp(14)
                            color: "#ffffff"
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
                    anchors.bottom: text_account.bottom
                    visible: false
                    background: Rectangle {
                        color: "#00000000"
                        radius: dp(4)
                        border.color: "#ffffff"
                        border.width: dp(2)
                        Text {
                            text: qsTr("Repost")
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            font.pixelSize: dp(14)
                            color: "#ffffff"
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
                    anchors.rightMargin: button_tx_cancel.visible && button_tx_repost.visible ? dp(220) : (button_tx_cancel.visible || button_tx_repost.visible ? dp(120) : dp(20))
                    anchors.bottom: text_account.bottom
                    visible: false
                    background: Rectangle {
                        color: "#00000000"
                        radius: dp(4)
                        border.color: "#ffffff"
                        border.width: dp(2)
                        Text {
                            text: qsTr("Proof")
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            font.pixelSize: dp(14)
                            color: "#ffffff"
                        }
                    }

                    onClicked: {
                        console.log("Proof button is clicked")
                        if (!tx2process.proof) {
                            messagebox.open("Need info", "Please select qualify transaction to generate a proof.")
                        } else {

                            if (qtAndroidService.requestPermissions()) {
                                qtAndroidService.createFile(downloadPath, "*/*", tx2process.txid + ".proof", 201);
                            } else {
                                messagebox.open("Failure", "Permission Denied")
                            }
                        }
                    }
                }

                Text {
                    id: label_txdate
                    color: "white"
                    font.bold: true
                    text: qsTr("Date")
                    anchors.left: parent.left
                    anchors.topMargin: dp(20)
                    anchors.top: text_account.bottom
                    font.pixelSize: dp(15)
                    anchors.leftMargin: dp(20)
                }

                Text {
                    id: text_txdate
                    color: "#ffffff"
                    text: ""
                    anchors.left: parent.left
                    anchors.topMargin: dp(3)
                    anchors.top: label_txdate.bottom
                    font.pixelSize: dp(15)
                    anchors.leftMargin: dp(20)
                }

                Text {
                    id: label_txid
                    color: "white"
                    font.bold: true
                    text: qsTr("Transaction ID")
                    anchors.left: parent.left
                    anchors.topMargin: dp(20)
                    anchors.top: text_txdate.bottom
                    font.pixelSize: dp(15)
                    anchors.leftMargin: dp(20)
                }

                Text {
                    id: text_txid
                    color: "#ffffff"
                    text: "49570294750498750249875049875"
                    anchors.left: parent.left
                    anchors.topMargin: dp(3)
                    anchors.top: label_txid.bottom
                    font.pixelSize: dp(15)
                    anchors.leftMargin: dp(20)
                }

                Text {
                    id: label_address
                    color: "white"
                    font.bold: true
                    text: qsTr("Recipient Address")
                    anchors.left: parent.left
                    anchors.topMargin: dp(20)
                    anchors.top: text_txid.bottom
                    anchors.leftMargin: dp(20)
                    font.pixelSize: dp(15)

                    ImageColor {
                        id: img_copy_slate
                        img_height: parent.height
                        img_source: "../../img/copy.svg"
                        img_color: "#ffffff"
                        anchors.left: label_address.right
                        anchors.leftMargin: dp(10)
                        anchors.verticalCenter: label_address.verticalCenter
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                clipboard.text = text_address.text
                                notification.text = "Address copied"
                                notification.open()
                            }
                        }
                        //anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Text {
                    id: text_address
                    color: "#ffffff"
                    text: "address"
                    wrapMode: Text.WrapAnywhere
                    anchors.left: parent.left
                    anchors.topMargin: dp(3)
                    anchors.top: label_address.bottom
                    anchors.leftMargin: dp(20)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(20)
                    font.pixelSize: dp(15)
                }



                Text {
                    id: label_txfee
                    color: "#ffffff"
                    text: qsTr("Network Fees")
                    anchors.left: parent.left
                    anchors.leftMargin: dp(20)
                    anchors.topMargin: dp(20)
                    font.bold: true
                    anchors.top: text_address.bottom
                    font.pixelSize: dp(15)
                }

                Text {
                    id: text_txfee
                    color: "#ffffff"
                    text: ""
                    anchors.left: parent.left
                    anchors.leftMargin: dp(20)
                    anchors.topMargin: dp(3)
                    anchors.top: label_txfee.bottom
                    font.pixelSize: dp(15)
                }
                Text {
                    id: text_txfee_currency
                    color: "gray"
                    text: ""
                    font.capitalization: Font.AllUppercase
                    anchors.left: text_txfee.right
                    anchors.leftMargin: dp(20)
                    anchors.verticalCenter: text_txfee.verticalCenter
                    font.pixelSize: dp(13)
                }

                Text {
                    id: label_commitments
                    color: "#ffffff"
                    text: qsTr("Input/Output Commitments")
                    anchors.left: parent.left
                    anchors.topMargin: dp(30)
                    font.bold: true
                    anchors.top: text_txfee.visible? text_txfee.bottom : text_address.visible? text_address.bottom : text_txid.bottom
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
                            color: "#ffffff"
                            font: combobox_commitments.font
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            color: combobox_commitments.highlightedIndex === index ? "#343434" : "#00000000"
                            radius: dp(5)
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
                            function onPressedChanged() {
                                canvas.requestPaint()
                            }
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
                            context.fillStyle = "#ffffff"
                            context.fill()
                        }
                    }

                    contentItem: Text {
                        text: combobox_commitments.currentIndex >= 0 ? list_commitments.get(combobox_commitments.currentIndex).value : ""
                        font: combobox_commitments.font
                        color: "#ffffff"
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        leftPadding: dp(20)
                        rightPadding: dp(20)
                    }

                    background: Rectangle {
                        implicitHeight: dp(40)
                        radius: dp(10)
                        color: "#232323"
                    }

                    popup: Popup {
                        id: popup
                        y: combobox_commitments.height - 1
                        width: combobox_commitments.width
                        height: contentItem.implicitHeight
                        padding: dp(1)
                        topPadding: dp(5)
                        bottomPadding: dp(5)

                        /*onOpened: {
                        timer.start()
                    }
                    Timer {
                        id: timer
                        interval: 2000 // milliseconds
                        running: true
                        repeat: false
                        onTriggered: {
                            popup.close()
                        }
                    }*/

                        enter: Transition {
                            NumberAnimation {
                                property: "height"; from: 0; to: popup.height; duration: 250
                            }
                        }
                        exit: Transition {
                            NumberAnimation {
                                property: "height"; from: popup.height; to: 0; duration: 250
                            }
                        }

                        contentItem: ListView {
                            clip: true
                            implicitHeight: contentHeight
                            model: combobox_commitments.popup.visible ? combobox_commitments.delegateModel : null
                            currentIndex: combobox_commitments.highlightedIndex
                            ScrollIndicator.vertical: ScrollIndicator {}
                        }

                        background: Rectangle {
                            radius: dp(10)
                            color: "#303030"

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
                    width: parent.width*0.9
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: dp(15)
                }

                /*Button {
                id: button_commitments_view
                height: dp(35)
                width: dp(80)
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                anchors.verticalCenter: combobox_commitments.verticalCenter
                background: Rectangle {
                    color: "#00000000"
                    radius: dp(4)
                    border.color: "#ffffff"
                    border.width: dp(2)
                    Text {
                        text: qsTr("View")
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: dp(14)
                        color: "#ffffff"
                    }
                }

                onClicked: {
                    if (combobox_commitments.currentIndex >= 0) {
                        Qt.openUrlExternally("https://" + blockExplorerUrl + "/#o" + JSON.parse(outputs[combobox_commitments.currentIndex]).outputCommitment)
                    }
                }
            }*/




                Rectangle {
                    id: rect_output
                    anchors.top: combobox_commitments.bottom
                    anchors.topMargin: dp(30)
                    height: grid_seed.Layout.minimumHeight
                    width: parent.width*0.9
                    radius: dp(25)
                    color: "#252525"
                    anchors.horizontalCenter: parent.horizontalCenter
                    visible: combobox_commitments.currentIndex >= 0
                    GridLayout {
                        id: grid_seed
                        columns: 3
                        rowSpacing: 0
                        width: parent.width
                        Repeater {
                            id: rep
                            model:list_info_io
                            delegate: items_info_io
                        }
                    }
                }

                ListModel {
                    id: list_info_io

                }

                Component {
                    id: items_info_io
                    Rectangle {
                        height: label_txnum.height + text_txnum.height + dp(10)
                        width: rect_output.width/3 -dp(25)
                        color: "#181818"
                        radius: dp(15)
                        Layout.alignment: Qt.AlignHCenter
                        visible: visibility
                        Layout.topMargin: dp(10)
                        Layout.bottomMargin: dp(10)
                        Text {
                            id: label_txnum
                            color: "#ffffff"
                            text: label
                            anchors.topMargin: dp(5)
                            font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.top: parent.top
                            font.pixelSize: dp(15)
                        }

                        Text {
                            id: text_txnum
                            color: "#ffffff"
                            text: value
                            anchors.topMargin: dp(3)
                            anchors.top: label_txnum.bottom
                            anchors.horizontalCenter: parent.horizontalCenter
                            font.pixelSize: dp(15)
                        }
                    }

                }
                /* onClicked: {
                 saveTransactionNote(textfield_note.text)
                 transactionDetail.visible = false
             }*/


                Text {
                    id: label_kernel
                    color: "#ffffff"
                    text: qsTr("Transaction Kernel")
                    anchors.left: parent.left
                    anchors.topMargin: dp(20)
                    font.bold: true
                    anchors.top: nbCommit>0? rect_output.bottom : text_txfee.visible? text_txfee.bottom : text_address.bottom
                    font.pixelSize: dp(15)
                    anchors.leftMargin: dp(20)
                }

                Text {
                    id: text_kernel
                    color: "#ffffff"
                    text: ""
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
                        border.color: "#ffffff"
                        border.width: dp(2)
                        Text {
                            text: qsTr("View")
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            font.pixelSize: dp(14)
                            color: "#ffffff"
                        }
                    }

                    onClicked: {
                        Qt.openUrlExternally("https://" + blockExplorerUrl + "/#k" + text_kernel.text)
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
