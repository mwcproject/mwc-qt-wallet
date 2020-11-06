import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
import FileTransactionBridge 1.0
import ConfigBridge 1.0
import WalletBridge 1.0

Item {
    property string transactionFileName
    property string resTxFN
    property bool isFinalize
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    FileTransactionBridge {
        id: fileTransaction
    }

    ConfigBridge {
        id: config
    }

    WalletBridge {
        id: wallet
    }

    Connections {
        target: fileTransaction
        onSgnHideProgress: {
            rect_progress.visible = false
        }
    }

    function confirmationCallback(ret) {
        if (ret) {
            rect_progress.visible = true
            if (isFinalize) {
                fileTransaction.ftContinue( transactionFileName, resTxFN, config.isFluffSet() )
            } else {
                const filepath = decodeURIComponent(transactionFileName)
                const path = "/mnt/user/0/primary/" + filepath.substring(filepath.search("primary:") + 8, filepath.length)
                fileTransaction.ftContinue( path, resTxFN, config.isFluffSet() )
            }
        }
    }

    onVisibleChanged: {
        if (visible) {
            const params = JSON.parse(initParams)
            transactionFileName = params.fileName
            fileTransaction.callerId = params.callerId
            rect_progress.visible = false
            text_title.text = params.transactionType
            text_button_ok.text = params.processButtonName
            text_amount.text = params.amount + " MWC"
            text_txid.text = params.transactionId
            text_lockheight.text = params.lockHeight
            text_message.text = params.message
            isFinalize = params.isFinalize
        }
    }

    Image {
        id: image_finalize
        width: dp(80)
        height: dp(80)
        anchors.top: parent.top
        anchors.topMargin: dp(100)
        anchors.left: parent.left
        anchors.leftMargin: dp(60)
        fillMode: Image.PreserveAspectFit
        source: "../img/FinalizeL@2x.svg"
    }

    Text {
        id: text_title
        anchors.left: image_finalize.right
        anchors.leftMargin: dp(20)
        anchors.verticalCenter: image_finalize.verticalCenter
        color: "#ffffff"
        text: qsTr("Receive File Transaction")
        font.pixelSize: dp(25)
        font.bold: true
    }

    Text {
        id: label_amount
        anchors.left: parent.left
        anchors.leftMargin: dp(150) - width
        anchors.bottom: label_txid.top
        anchors.bottomMargin: dp(30)
        color: "#ffffff"
        text: qsTr("Amount:")
        font.pixelSize: dp(20)
    }

    Text {
        id: text_amount
        anchors.left: label_amount.right
        anchors.leftMargin: dp(20)
        anchors.verticalCenter: label_amount.verticalCenter
        color: "#ffffff"
        text: qsTr("1 MWC")
        font.pixelSize: dp(20)
    }

    Text {
        id: label_txid
        anchors.left: parent.left
        anchors.leftMargin: dp(150) - width
        anchors.bottom: label_lockheight.top
        anchors.bottomMargin: dp(30)
        color: "#ffffff"
        text: qsTr("Transaction ID:")
        font.pixelSize: dp(20)
    }

    Text {
        id: text_txid
        anchors.left: label_txid.right
        anchors.leftMargin: dp(20)
        anchors.verticalCenter: label_txid.verticalCenter
        color: "#ffffff"
        text: qsTr("11111111-1111-1111-1111-111111111111")
        font.pixelSize: dp(18)
    }

    Text {
        id: label_lockheight
        anchors.left: parent.left
        anchors.leftMargin: dp(150) - width
        anchors.verticalCenter: parent.verticalCenter
        color: "#ffffff"
        text: qsTr("Lock Height:")
        font.pixelSize: dp(20)
    }

    Text {
        id: text_lockheight
        anchors.left: label_lockheight.right
        anchors.leftMargin: dp(20)
        anchors.verticalCenter: label_lockheight.verticalCenter
        color: "#ffffff"
        text: qsTr("-")
        font.pixelSize: dp(20)
    }

    Text {
        id: label_message
        anchors.left: parent.left
        anchors.leftMargin: dp(150) - width
        anchors.top: label_lockheight.bottom
        anchors.topMargin: dp(30)
        color: "#ffffff"
        text: qsTr("Message:")
        font.pixelSize: dp(20)
    }

    Rectangle {
        id: rect_message
        height: text_message.text === "" ? dp(150) : text_message.height + dp(20)
        color: "#33bf84ff"
        anchors.left: label_message.right
        anchors.leftMargin: dp(20)
        anchors.top: label_message.top
        anchors.right: parent.right
        anchors.rightMargin: dp(20)
        radius: dp(5)

        Text {
            id: text_message
            text: qsTr("")
            color: "#ffffff"
            font.pixelSize: dp(20)
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            anchors.fill: parent
            anchors.margins: dp(10)
        }
    }

    Button {
        id: button_ok
        width: dp(200)
        height: dp(50)
        anchors.bottom: parent.bottom
        anchors.bottomMargin: dp(100)
        anchors.right: parent.right
        anchors.rightMargin: parent.width / 2 - dp(200)

        background: Rectangle {
            color: "#00000000"
            radius: dp(5)
            border.width: dp(2)
            border.color: "white"
            Text {
                id: text_button_ok
                text: qsTr("Generate Response")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
            if (!fileTransaction.isNodeHealthy()) {
                messagebox.open(qsTr("Unable to finalize"), qsTr("Your MWC Node, that wallet connected to, is not ready to finalize transactions.\nMWC Node need to be connected to few peers and finish blocks synchronization process"))
                return
            }
            const walletPasswordHash = wallet.getPasswordHash()
            if (walletPasswordHash !== "") {
                if (isFinalize) {
                    sendConfirmationItem.open(qsTr("Confirm Finalize Request"), qsTr("You are finalizing transaction for " + text_amount.text), walletPasswordHash, confirmationCallback)
                    return
                }
            }
            confirmationCallback(true)
        }
    }

    Button {
        id: button_cancel
        width: dp(150)
        height: dp(50)
        anchors.bottom: parent.bottom
        anchors.bottomMargin: dp(100)
        anchors.left: parent.left
        anchors.leftMargin: parent.width / 2 - dp(200)

        background: Rectangle {
            color: "#00000000"
            radius: dp(5)
            border.width: dp(2)
            border.color: "white"
            Text {
                text: qsTr("Cancel")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
            initParams = ""
            fileTransaction.ftBack()
        }
    }

    Rectangle {
        id: rect_progress
        width: dp(60)
        height: dp(30)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: button_ok.bottom
        anchors.topMargin: dp(20)
        color: "#00000000"
        visible: false
        AnimatedImage {
            id: animation
            source: "../img/loading.gif"
        }
    }

    SendConfirmation {
        id: sendConfirmationItem
        anchors.verticalCenter: parent.verticalCenter
    }

    MessageBox {
        id: messagebox
        anchors.verticalCenter: parent.verticalCenter
    }
}
