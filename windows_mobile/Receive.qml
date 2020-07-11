import QtQuick 2.0
import QtQuick.Controls 2.13
import Qt.labs.platform 1.1
import QtQuick.Window 2.0
import ConfigBridge 1.0
import ReceiveBridge 1.0
import WalletBridge 1.0
import Clipboard 1.0

Item {
    id: element
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    ConfigBridge {
        id: config
    }

    ReceiveBridge {
        id: receive
    }

    WalletBridge {
        id: wallet
    }

    Clipboard {
        id: clipboard
    }

    Connections {
        target: receive
        onSgnTransactionActionIsFinished: {
//            ui->progress->hide()
            const title = success ? "Success" : "Failure"
            messagebox.open(qsTr(title), qsTr(message))
            console.log(success, message)
        }
    }

    Connections {
        target: wallet
        onSgnWalletBalanceUpdated: {
            updateAccountList()
        }

        onSgnMwcAddressWithIndex: {
            updateStatus()
        }

        onSgnTorAddress: {
            updateStatus()
        }

        onSgnUpdateListenerStatus: {
            updateStatus()
        }
    }

    onVisibleChanged: {
        if (visible) {
            if (config.hasTls()) {
                text_http.text = qsTr("Https")
            }
            updateAccountList()
            updateStatus()
        }
    }

    function updateAccountList() {
        const accountInfo = wallet.getWalletBalance(true, false, true)
        const selectedAccount = wallet.getReceiveAccount()
        let selectedAccIdx = 0

        accountItems.clear()

        let idx = 0
        for (let i = 1; i < accountInfo.length; i += 2) {
            if (accountInfo[i-1] === selectedAccount)
                selectedAccIdx = idx

            accountItems.append({ info: accountInfo[i], account: accountInfo[i-1]})
            idx++
        }
        accountComboBox.currentIndex = selectedAccIdx
    }

    function updateStatus() {
        image_mwcmq.source = wallet.getMqsListenerStatus() ? "../img/StatusOk@2x.svg" : "../img/StatusEmpty@2x.svg"
        image_keybase.source = wallet.getKeybaseListenerStatus() ? "../img/StatusOk@2x.svg" : "../img/StatusEmpty@2x.svg"
        image_http.source = wallet.getHttpListeningStatus() ? "../img/StatusOk@2x.svg" : "../img/StatusEmpty@2x.svg"
        image_tor.source = wallet.getTorListenerStatus() ? "../img/StatusOk@2x.svg" : "../img/StatusEmpty@2x.svg"
    }

    Text {
        id: text_mwcmq
        text: qsTr("MWCMQ")
        anchors.left: parent.left
        anchors.leftMargin: dp(50)
        horizontalAlignment: Text.AlignRight
        anchors.verticalCenterOffset: dp(-250)
        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: dp(17)
        color: "white"
    }

    Image {
        id: image_mwcmq
        anchors.left: text_mwcmq.right
        anchors.leftMargin: dp(16)
        anchors.verticalCenter: text_mwcmq.verticalCenter
        width: dp(20)
        height: dp(20)
    }

    Text {
        id: text_keybase
        text: qsTr("Keybase")
        anchors.verticalCenter: text_mwcmq.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        horizontalAlignment: Text.AlignRight
        font.pixelSize: dp(17)
        color: "white"
    }

    Image {
        id: image_keybase
        anchors.left: text_keybase.right
        anchors.leftMargin: dp(16)
        anchors.verticalCenter: text_keybase.verticalCenter
        width: dp(20)
        height: dp(20)
    }

    Text {
        id: text_http
        text: qsTr("Http")
        anchors.verticalCenter: text_mwcmq.verticalCenter
        anchors.right: image_http.left
        anchors.rightMargin: dp(16)
        horizontalAlignment: Text.AlignRight
        font.pixelSize: dp(17)
        color: "white"
    }

    Image {
        id: image_http
        anchors.verticalCenter: text_http.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: dp(50)
        width: dp(20)
        height: dp(20)
    }

    Text {
        id: text_tor
        text: qsTr("TOR")
        anchors.left: parent.left
        anchors.leftMargin: dp(50)
        horizontalAlignment: Text.AlignRight
        anchors.top: text_mwcmq.bottom
        anchors.topMargin: dp(30)
        anchors.right: text_mwcmq.right
        font.pixelSize: dp(17)
        color: "white"
    }

    Image {
        id: image_tor
        anchors.left: text_tor.right
        anchors.leftMargin: dp(16)
        anchors.verticalCenter: text_tor.verticalCenter
        width: dp(20)
        height: dp(20)
    }

    Text {
        id: label_combobox
        color: "#ffffff"
        text: qsTr("Account to receive incoming MWC coins")
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.top: text_tor.bottom
        anchors.topMargin: dp(45)
        font.pixelSize: dp(14)
    }

    ComboBox {
        id: accountComboBox
        height: dp(72)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.top: label_combobox.bottom
        anchors.topMargin: dp(10)
        leftPadding: dp(20)
        rightPadding: dp(50)
        font.pixelSize: dp(14)

        onCurrentIndexChanged: {
            if (accountComboBox.currentIndex >= 0) {
                const selectedAccount = accountItems.get(accountComboBox.currentIndex).account
                wallet.setReceiveAccount(selectedAccount)
            }
        }

        delegate: ItemDelegate {
            width: accountComboBox.width
            height: dp(40)
            contentItem: Text {
                text: info
                color: "#7579ff"
                font: accountComboBox.font
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
            highlighted: accountComboBox.highlightedIndex === index
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
                context.moveTo(0, 0)
                context.lineTo(width / 2, height)
                context.lineTo(width, 0)
                context.strokeStyle = "white"
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
            radius: dp(4)
            color: "#8633E0"
        }

        popup: Popup {
            y: accountComboBox.height - 1
            width: accountComboBox.width
            implicitHeight: contentItem.implicitHeight
            padding: dp(1)

            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: accountComboBox.popup.visible ? accountComboBox.delegateModel : null
                currentIndex: accountComboBox.highlightedIndex

                ScrollIndicator.vertical: ScrollIndicator { }
            }

            background: Rectangle {
                border.color: "white"
                radius: dp(4)
            }
        }

        model: ListModel {
            id: accountItems
        }
    }

    Text {
        id: label_online_methods
        color: "#ffffff"
        text: qsTr("Online Methods")
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: dp(36)
        font.pixelSize: dp(14)
    }

    Button {
        id: button_mwcmq
        width: parent.width / 2 - dp(40)
        height: dp(72)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.top: label_online_methods.bottom
        anchors.topMargin: dp(17)
        background: Rectangle {
            color: "#00000000"
            radius: dp(5)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Copy Address")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }
        onClicked: {
            clipboard.text = "mwcmqs://" + wallet.getMqsAddress()
            notification.text = "Address copied to the clipboard"
            notification.open()
        }
    }

    Button {
        id: button_tor
        width: parent.width / 2 - dp(40)
        height: dp(72)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        anchors.verticalCenter: button_mwcmq.verticalCenter
        background: Rectangle {
            color: "#00000000"
            radius: dp(5)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("TOR")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }
        onClicked: {
            clipboard.text = wallet.getTorAddress()
            notification.text = "Address copied to the clipboard"
            notification.open()
        }
    }

    Text {
        id: label_copy_clipboard
        color: "#BF84FF"
        text: "(Copies address to clipboard)"
        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: button_mwcmq.bottom
        anchors.topMargin: dp(12)
        font.pixelSize: dp(13)
    }

    Text {
        id: label_offline_methods
        color: "#ffffff"
        text: qsTr("Offline Method")
        anchors.left: parent.left
        anchors.leftMargin: dp(36)
        anchors.top: label_copy_clipboard.bottom
        anchors.topMargin: dp(35)
        font.pixelSize: dp(14)
    }

    Button {
        id: button_file
        width: button_mwcmq.width
        height: dp(72)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.top: label_offline_methods.bottom
        anchors.topMargin: dp(17)
        background: Rectangle {
            color: "#00000000"
            radius: dp(5)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Receive by File")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }
        onClicked: {
            fileDialog.open()
        }
    }

    Text {
        id: text_file
        color: "#BF84FF"
        text: qsTr("(sign transaction)")
        anchors.horizontalCenter: button_file.horizontalCenter
        anchors.top: button_file.bottom
        anchors.topMargin: dp(12)
        font.pixelSize: dp(13)
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Open initial transaction file")
        folder: config.getPathFor("fileGen")
        nameFilters: ["MWC transaction (*.tx *.input);;All files (*.*)"]
        onAccepted: {
            console.log("Accepted: " + fileDialog.file)
//            bridge.updateFileGenerationPath(fileDialog.file)
//            ui->progress->show()
//            receive.signTransaction(fileDialog.file)
        }
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: -notification.width / 2
        anchors.bottom: parent.bottom
        anchors.bottomMargin: dp(50)
        Notification {
            id: notification
        }
    }

    MessageBox {
        id: messagebox
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:640;width:320}
}
##^##*/
