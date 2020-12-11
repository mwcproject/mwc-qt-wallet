import QtQuick 2.0
import QtQuick.Controls 2.13
import Qt.labs.platform 1.1
import QtQuick.Window 2.0
import ConfigBridge 1.0
import ReceiveBridge 1.0
import WalletBridge 1.0
import UtilBridge 1.0
import Clipboard 1.0

Item {
    property string proofAddr
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

    UtilBridge {
        id: util
    }

    Clipboard {
        id: clipboard
    }

    Connections {
        target: receive
        onSgnTransactionActionIsFinished: (success, message) => {
            rect_progress.visible = false
            const title = success ? "Success" : "Failure"
            messagebox.open(qsTr(title), qsTr(message))
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

        onSgnFileProofAddress: (proofAddress) => {
            proofAddr = proofAddress
        }

        onSgnUpdateListenerStatus: {
            updateStatus()
        }
    }

    onVisibleChanged: {
        if (visible) {
            rect_progress.visible = false
            if (config.hasTls()) {
                text_http.text = qsTr("Https")
            }
            updateAccountList()
            wallet.requestFileProofAddress()
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
        image_mwcmq.source = wallet.getMqsListenerStatus() ? "../img/CircGreen@2x.svg" : "../img/CircRed@2x.svg"
        image_http.source = wallet.getHttpListeningStatus() ? "../img/CircGreen@2x.svg" : "../img/CircRed@2x.svg"
        image_tor.source = wallet.getTorListenerStatus() ? "../img/CircGreen@2x.svg" : "../img/CircRed@2x.svg"
    }

    Rectangle {
        id: rect_mwcmq
        width: dp(150)
        height: dp(40)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: dp(-250)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: dp(-100)
        border.color: "white"
        border.width: dp(2)
        radius: dp(50)
        color: "#00000000"

        Image {
            id: image_mwcmq
            anchors.left: parent.left
            anchors.leftMargin: dp(10)
            anchors.verticalCenter: parent.verticalCenter
            width: dp(20)
            height: dp(20)
        }

        Text {
            id: text_mwcmq
            text: qsTr("MWCMQ")
            anchors.left: image_mwcmq.right
            anchors.leftMargin: dp(10)
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: dp(17)
            color: "white"
        }
    }

    Rectangle {
        id: rect_tor
        width: dp(150)
        height: dp(40)
        anchors.top: rect_mwcmq.bottom
        anchors.topMargin: dp(20)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: dp(-100)
        border.color: "white"
        border.width: dp(2)
        radius: dp(50)
        color: "#00000000"

        Image {
            id: image_tor
            anchors.left: parent.left
            anchors.leftMargin: dp(10)
            anchors.verticalCenter: parent.verticalCenter
            width: dp(20)
            height: dp(20)
        }

        Text {
            id: text_tor
            text: qsTr("TOR")
            anchors.left: image_tor.right
            anchors.leftMargin: dp(10)
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: dp(17)
            color: "white"
        }
    }

    Rectangle {
        id: rect_http
        width: dp(150)
        height: dp(40)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: dp(-250)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: dp(100)
        border.color: "white"
        border.width: dp(2)
        radius: dp(50)
        color: "#00000000"

        Image {
            id: image_http
            anchors.left: parent.left
            anchors.leftMargin: dp(10)
            anchors.verticalCenter: parent.verticalCenter
            width: dp(20)
            height: dp(20)
        }

        Text {
            id: text_http
            text: qsTr("HTTP")
            anchors.left: image_http.right
            anchors.leftMargin: dp(10)
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: dp(17)
            color: "white"
        }
    }

    Text {
        id: label_combobox
        color: "#ffffff"
        text: qsTr("Account to receive incoming MWC coins")
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.top: rect_tor.bottom
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
        rightPadding: dp(40)
        font.pixelSize: dp(14)

        onCurrentIndexChanged: {
            if (accountComboBox.currentIndex >= 0) {
                const selectedAccount = accountItems.get(accountComboBox.currentIndex).account
                wallet.setReceiveAccount(selectedAccount)
            }
        }

        delegate: ItemDelegate {
            width: accountComboBox.width
            height: dp(72)
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
            implicitHeight: dp(72)
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
                text: qsTr("MWCMQ")
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

    Button {
        id: button_proof
        width: parent.width / 2 - dp(40)
        height: dp(72)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.top: button_mwcmq.bottom
        anchors.topMargin: dp(20)
        background: Rectangle {
            color: "#00000000"
            radius: dp(5)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Proof Address")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }
        onClicked: {
            clipboard.text = proofAddr
            notification.text = "Address copied to the clipboard"
            notification.open()
        }
    }

    Text {
        id: label_copy_clipboard
        color: "#BF84FF"
        text: "(Copies address to clipboard)"
        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenter: button_tor.horizontalCenter
        anchors.verticalCenter: button_proof.verticalCenter
        font.pixelSize: dp(13)
    }

    Text {
        id: label_offline_methods
        color: "#ffffff"
        text: qsTr("Offline Method")
        anchors.left: parent.left
        anchors.leftMargin: dp(36)
        anchors.top: button_proof.bottom
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
        anchors.horizontalCenter: button_tor.horizontalCenter
        anchors.verticalCenter: button_file.verticalCenter
        font.pixelSize: dp(13)
        visible: !rect_progress.visible
    }

    Rectangle {
        id: rect_progress
        width: dp(60)
        height: dp(30)
        anchors.verticalCenter: button_file.verticalCenter
        anchors.horizontalCenter: button_tor.horizontalCenter
        color: "#00000000"
        visible: false
        AnimatedImage {
            id: animation
            source: "../img/loading.gif"
        }
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Open initial transaction file")
        folder: config.getPathFor("fileGen")
        nameFilters: ["MWC transaction (*.tx *.input);;All files (*.*)"]
        onAccepted: {
            var path = fileDialog.file.toString()

            const validation = util.validateMwc713Str(path)
            if (validation) {
                messagebox.open(qsTr("File Path"), qsTr("This file path is not acceptable.\n" + validation))
                return
            }
            // remove prefixed "file:///"
            path= path.replace(/^(file:\/{3})|(qrc:\/{2})|(http:\/{2})/,"")
            // unescape html codes like '%23' for '#'
            const cleanPath = decodeURIComponent(path);
            config.updatePathFor("fileGen", cleanPath)
            rect_progress.visible = true
            receive.signTransaction(fileDialog.file.toString())
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
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:640;width:320}
}
##^##*/
