import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
import ReceiveBridge 1.0
import UtilBridge 1.0
import QtAndroidService 1.0

Item {
    property string fileNameOrSlatepack

    ReceiveBridge {
        id: receive
    }

    UtilBridge {
        id: util
    }

    QtAndroidService {
        id: qtAndroidService
    }

    Connections {
        target: receive
        onSgnHideProgress: {
            rect_progress.visible = false
        }
    }

    onVisibleChanged: {
        if (visible) {
            if (parent.height > dp(520)) {
                anchors.topMargin = (parent.height - dp(520)) / 2
            }

            const params = JSON.parse(initParams)
            rect_progress.visible = false
            fileNameOrSlatepack = params.fileNameOrSlatepack
            text_amount.text = params.amount + " MWC"
            text_txid.text = params.transactionId
            text_lockheight.text = params.lockHeight
            text_senderAddress.text = params.senderAddress
            text_senderDescription.text = params.senderMessage
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            textarea_message.focus = false
        }
    }

    Image {
        id: image_finalize
        width: dp(70)
        height: dp(70)
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: dp(70)
        fillMode: Image.PreserveAspectFit
        source: "../img/FinalizeL@2x.svg"
    }

    Text {
        id: text_title
        anchors.left: image_finalize.right
        anchors.leftMargin: dp(20)
        anchors.verticalCenter: image_finalize.verticalCenter
        color: "#ffffff"
        text: qsTr("Receive Transaction")
        font.pixelSize: dp(24)
        font.bold: true
    }

    Text {
        id: label_amount
        anchors.left: parent.left
        anchors.leftMargin: dp(160) - width
        anchors.top: image_finalize.bottom
        anchors.topMargin: dp(50)
        color: "#ffffff"
        text: qsTr("Amount:")
        font.pixelSize: dp(15)
    }

    Text {
        id: text_amount
        anchors.left: label_amount.right
        anchors.leftMargin: dp(20)
        anchors.verticalCenter: label_amount.verticalCenter
        color: "#ffffff"
        text: qsTr("1 MWC")
        font.pixelSize: dp(15)
    }

    Text {
        id: label_txid
        anchors.left: parent.left
        anchors.leftMargin: dp(160) - width
        anchors.top: label_amount.bottom
        anchors.topMargin: dp(20)
        color: "#ffffff"
        text: qsTr("Transaction ID:")
        font.pixelSize: dp(15)
    }

    Text {
        id: text_txid
        anchors.left: label_txid.right
        anchors.leftMargin: dp(20)
        anchors.verticalCenter: label_txid.verticalCenter
        color: "#ffffff"
        text: qsTr("11111111-1111-1111-1111-111111111111")
        font.pixelSize: dp(15)
    }

    Text {
        id: label_lockheight
        anchors.left: parent.left
        anchors.leftMargin: dp(160) - width
        anchors.top: label_txid.bottom
        anchors.topMargin: dp(20)
        color: "#ffffff"
        text: qsTr("Lock Height:")
        font.pixelSize: dp(15)
    }

    Text {
        id: text_lockheight
        anchors.left: label_lockheight.right
        anchors.leftMargin: dp(20)
        anchors.verticalCenter: label_lockheight.verticalCenter
        color: "#ffffff"
        text: qsTr("-")
        font.pixelSize: dp(15)
    }

    Text {
        id: label_senderAddress
        anchors.left: parent.left
        anchors.leftMargin: dp(160) - width
        anchors.top: label_lockheight.bottom
        anchors.topMargin: dp(20)
        color: "#ffffff"
        text: qsTr("Sender Address:")
        font.pixelSize: dp(15)
    }

    Text {
        id: text_senderAddress
        anchors.left: label_senderAddress.right
        anchors.leftMargin: dp(20)
        anchors.right: parent.right
        anchors.rightMargin: dp(10)
        anchors.verticalCenter: label_senderAddress.verticalCenter
        color: "#ffffff"
        text: qsTr("-")
        elide: Text.ElideMiddle
        font.pixelSize: dp(15)
    }

    Text {
        id: label_senderDescription
        anchors.left: parent.left
        anchors.leftMargin: dp(160) - width
        anchors.top: label_senderAddress.bottom
        anchors.topMargin: dp(20)
        color: "#ffffff"
        text: qsTr("Sender Description:")
        font.pixelSize: dp(15)
    }

    Text {
        id: text_senderDescription
        anchors.left: label_senderDescription.right
        anchors.leftMargin: dp(20)
        anchors.verticalCenter: label_senderDescription.verticalCenter
        color: "#ffffff"
        text: qsTr("")
        font.pixelSize: dp(15)
    }

    Text {
        id: label_message
        anchors.left: parent.left
        anchors.leftMargin: dp(160) - width
        anchors.top: label_senderDescription.bottom
        anchors.topMargin: dp(20)
        color: "#ffffff"
        text: qsTr("My Description:")
        font.pixelSize: dp(15)
    }

    TextArea {
        id: textarea_message
        height: dp(100)
        padding: dp(10)
        font.pixelSize: dp(15)
        color: "white"
        text: ""
        anchors.left: label_message.right
        anchors.leftMargin: dp(20)
        anchors.top: label_message.top
        anchors.right: parent.right
        anchors.rightMargin: dp(20)
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        background: Rectangle {
            color: "#8633E0"
            radius: dp(5)
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                textarea_message.focus = true
            }
        }
    }

    Button {
        id: button_ok
        width: dp(200)
        height: dp(50)
        anchors.top: textarea_message.bottom
        anchors.topMargin: dp(30)
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
            const description = textarea_message.text.trim().replace('\n', ' ')
            const valRes = util.validateMwc713Str(description);
            if (valRes !== "") {
                messagebox.open(qsTr("Incorrect Input"), valRes)
                textarea_message.focus = true
                return
            }

            rect_progress.visible = true

            if (fileNameOrSlatepack.substring(0, 10) === "BEGINSLATE") {
                receive.receiveSlatepack(fileNameOrSlatepack, description)
            } else {
                if (qtAndroidService.requestPermissions()) {
                    receive.receiveFile(fileNameOrSlatepack,  description)
                } else {
                    messagebox.open("Failure", "Permission Denied")
                }
            }
        }
    }

    Button {
        id: button_cancel
        width: dp(150)
        height: dp(50)
        anchors.verticalCenter: button_ok.verticalCenter
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
            receive.cancelReceive()
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
}
