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
//            util::TimeoutLockObject to( "Receive" )
//            ui->progress->hide()
//            control::MessageBox::messageText(this, success ? "Success" : "Failure", message )
            console.log(success, message)
        }
    }

    Connections {
        target: wallet
        onSgnWalletBalanceUpdated: {
            updateAccountList()
        }
    }

    onVisibleChanged: {
        if (visible) {
            const mwcMqAddress = wallet.getMqsAddress()
            text_address.text = mwcMqAddress.slice(0, 9) + " ... " + mwcMqAddress.slice(mwcMqAddress.length - 7, mwcMqAddress.length)
            image_mwcmq.source = wallet.getMqsListenerStatus() ? "../img/StatusOk@2x.svg" : "../img/StatusEmpty@2x.svg"
            image_keybase.source = wallet.getKeybaseListenerStatus() ? "../img/StatusOk@2x.svg" : "../img/StatusEmpty@2x.svg"
            image_http.source = wallet.getHttpListeningStatus() ? "../img/StatusOk@2x.svg" : "../img/StatusEmpty@2x.svg"
            if (config.hasTls()) {
                text_http.text = qsTr("Https")
            }
            updateAccountList()
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

    Text {
        id: text_mwcmq
        text: qsTr("MWC Message Queue")
        anchors.right: image_mwcmq.left
        anchors.rightMargin: dp(16)
        horizontalAlignment: Text.AlignRight
        anchors.verticalCenterOffset: dp(-250)
        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: dp(17)
        color: "white"
    }

    Image {
        id: image_mwcmq
        anchors.horizontalCenterOffset: dp(100)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: text_mwcmq.verticalCenter
        width: dp(20)
        height: dp(20)
    }

    Text {
        id: text_keybase
        text: qsTr("Keybase")
        anchors.right: image_keybase.left
        anchors.rightMargin: dp(16)
        horizontalAlignment: Text.AlignRight
        anchors.top: text_mwcmq.bottom
        anchors.topMargin: dp(30)
        font.pixelSize: dp(17)
        color: "white"
    }

    Image {
        id: image_keybase
        anchors.horizontalCenterOffset: dp(100)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: text_keybase.verticalCenter
        width: dp(20)
        height: dp(20)
    }

    Text {
        id: text_http
        text: qsTr("Http")
        anchors.right: image_http.left
        anchors.rightMargin: dp(16)
        horizontalAlignment: Text.AlignRight
        anchors.top: text_keybase.bottom
        anchors.topMargin: dp(30)
        font.pixelSize: dp(17)
        color: "white"
    }

    Image {
        id: image_http
        anchors.verticalCenter: text_http.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: dp(100)
        width: dp(20)
        height: dp(20)
    }

    Text {
        id: label_combobox
        color: "#ffffff"
        text: qsTr("Choose Account")
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.bottom: accountComboBox.top
        anchors.bottomMargin: dp(10)
        font.pixelSize: dp(13)
    }

    ComboBox {
        id: accountComboBox
        height: dp(72)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        leftPadding: dp(20)
        rightPadding: dp(40)
        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: dp(14)

        onCurrentIndexChanged: {
            if (accountComboBox.currentIndex >= 0) {
                const currentAccount = accountItems.get(accountComboBox.currentIndex).account
                wallet.setReceiveAccount(currentAccount)
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
        id: label_address
        color: "#ffffff"
        text: qsTr("Online (mwcmqs)")
        anchors.top: accountComboBox.bottom
        anchors.topMargin: dp(40)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        font.pixelSize: dp(13)
    }

    Button {
        id: button_copyaddress
        width: parent.width / 2 - dp(40)
        height: dp(70)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.top: label_address.bottom
        anchors.topMargin: dp(14)
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(1)
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

    Text {
        id: text_address
        width: dp(160)
        color: "#BF84FF"
        text: ""
        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenter: button_copyaddress.horizontalCenter
        anchors.top: button_copyaddress.bottom
        anchors.topMargin: dp(14)
        font.pixelSize: dp(13)
    }

    Text {
        id: label_file
        color: "#ffffff"
        text: qsTr("Offline (file)")
        anchors.left: parent.left
        anchors.leftMargin: parent.width / 2 + dp(10)
        anchors.top: accountComboBox.bottom
        anchors.topMargin: dp(40)
        font.pixelSize: dp(13)
    }

    Button {
        id: button_receivebyfile
        width: parent.width / 2 - dp(40)
        height: dp(70)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        anchors.top: label_file.bottom
        anchors.topMargin: dp(14)
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(1)
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
        id: text_receivebyfile
        color: "#BF84FF"
        text: qsTr("(sign transaction)")
        anchors.horizontalCenter: button_receivebyfile.horizontalCenter
        anchors.top: button_receivebyfile.bottom
        anchors.topMargin: dp(14)
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
        anchors.right: parent.right
        anchors.rightMargin: dp(50)
        anchors.left: parent.left
        anchors.leftMargin: dp(50)
        anchors.bottom: parent.bottom
        anchors.bottomMargin: dp(160)
        Notification {
            id: notification
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:640;width:320}
}
##^##*/
