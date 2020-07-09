import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import WalletBridge 1.0
import ConfigBridge 1.0
import UtilBridge 1.0

Item {
    id: transactionDetail
    property var account
    property var txinfo
    property var outputs
    property var messages
    property var txnNote
    property string blockExplorerUrl
    property string originalTransactionNote
    property string txUuid
    property var type_TRANSACTION_CANCELLED: 0x8000
    property var type_TRANSACTION_COIN_BASE: 4
    property var type_TRANSACTION_RECEIVE: 2
    property var type_TRANSACTION_SEND: 1
    property var locale: Qt.locale()

    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    ConfigBridge {
        id: config
    }

    UtilBridge {
        id: util
    }

    function init(_account, _txinfo, _outputsInfo, _messages, _txnNote) {
        account = _account
        txinfo = _txinfo
        outputs = _outputsInfo
        messages = _messages
        txnNote = _txnNote
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

            list_commitments.append({ text: commitType + outputs[i].outputCommitment });
        }

        // Selecting first output
        if (txinfo.numInputs < outputs.length) {
            combobox_commitments.currentIndex = txinfo.numInputs
        } else {
            combobox_commitments.currentIndex = -1
        }
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
//        const date = Date.fromLocaleString(locale, creationTime, "hh:mm:ss dd-MM-yyyy")
        const date = Date.fromLocaleString(locale, creationTime, "dd-MM-yyyy hh:mm")
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

    Rectangle {
        anchors.fill: parent
        color: "white"

        Rectangle {
            id: rect_header
            height: dp(150)
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
                    saveTransactionNote(textfield_note.text)
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
                anchors.verticalCenterOffset: dp(-20)
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
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
                font.pixelSize: dp(21)
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
                        saveTransactionNote(textfield_note.text)
                    }
                }
            }
        }

        ScrollView {
            id: view_txinfo
            clip: true
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            contentHeight: dp(650) + text_message.height
            anchors.top: rect_header.bottom
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.left: parent.left

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    textfield_note.focus = false
                    saveTransactionNote(textfield_note.text)
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
                anchors.left: parent.left
                anchors.topMargin: dp(3)
                anchors.top: label_address.bottom
                anchors.leftMargin: dp(20)
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
                height: dp(50)
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
                anchors.horizontalCenterOffset: -1 * dp(label_debited.width / 2)
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

                delegate: ItemDelegate {
                    width: combobox_commitments.width
                    contentItem: Text {
                        text: modelData
                        color: "#3600C9"
                        font: combobox_commitments.font
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                    }
                    highlighted: combobox_commitments.highlightedIndex === index
                }

                indicator: Canvas {
                    id: canvas
                    x: combobox_commitments.width - width - combobox_commitments.rightPadding
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
                        context.moveTo(0, 0)
                        context.lineTo(width / 2, height)
                        context.lineTo(width, 0)
                        context.fillStyle = "#3600C9"
                        context.fill()
                    }
                }

                contentItem: Text {
                    text: combobox_commitments.displayText
                    font: combobox_commitments.font
                    color: "#3600C9"
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                background: Rectangle {
                    implicitHeight: dp(50)
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
                        border.color: "white"
                        radius: dp(3)
                    }
                }

                model: ListModel {
                    id: list_commitments
                    ListElement { text: "odifjowo4384938hfjk3fjijifji3nfeunf..." }
                    ListElement { text: "odifjowo4384938hfjk3fjijifji3nfeunf..." }
                    ListElement { text: "odifjowo4384938hfjk3fjijifji3nfeunf..." }
                }
                anchors.top: label_commitments.bottom
                anchors.topMargin: dp(10)
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                anchors.left: parent.left
                anchors.leftMargin: dp(20)
                leftPadding: dp(20)
                rightPadding: dp(20)
                font.pixelSize: dp(15)
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
