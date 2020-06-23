import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
import WalletBridge 1.0
import SendBridge 1.0

Item {
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    WalletBridge {
        id: wallet
    }

    SendBridge {
        id: send
    }

    Connections {
        target: wallet
        onSgnWalletBalanceUpdated: {
            const accountInfo = wallet.getWalletBalance(true, true, false)
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
    }

    onVisibleChanged: {
        if (visible) {
            wallet.requestWalletBalanceUpdate()
            rect_online.color = "#8633E0"
            text_online_selected.visible = true
            rect_file.color = "#00000000"
            text_file_selected.visible = false
            textfield_amount.text = ""
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            textfield_amount.focus = false
        }
    }

    Rectangle {
        id: rect_online
        width: dp(150)
        height: dp(210)
        color: "#00000000"
        radius: dp(10)
        anchors.left: parent.left
        anchors.leftMargin: (parent.width / 2 - rect_online.width) / 1.5
        anchors.bottom: text_description1 .top
        anchors.bottomMargin: dp(30)
        border.color: "#ffffff"
        border.width: dp(2)

        Image {
            id: image_online
            width: dp(80)
            height: dp(80)
            anchors.top: text_online_selected.bottom
            anchors.topMargin: dp(10)
            anchors.horizontalCenter: parent.horizontalCenter
            fillMode: Image.PreserveAspectFit
            source: "../img/SendOnline@2x.svg"
        }

        Text {
            id: text_online_selected
            color: "#ffffff"
            text: qsTr("SELECTED")
            anchors.top: parent.top
            anchors.topMargin: dp(20)
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(15)
            visible: false
        }

        Text {
            id: element1
            color: "#ffffff"
            text: qsTr("Address")
            font.bold: true
            anchors.top: image_online.bottom
            anchors.topMargin: dp(10)
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(21)
        }

        Rectangle {
            id: rect_splitter1
            height: 1
            color: "#ffffff"
            anchors.top: element1.bottom
            anchors.topMargin: dp(5)
            anchors.right: parent.right
            anchors.rightMargin: dp(20)
            anchors.left: parent.left
            anchors.leftMargin: dp(20)
        }

        Text {
            id: element2
            color: "#ffffff"
            text: qsTr("Confirmed Online")
            anchors.top: rect_splitter1.bottom
            anchors.topMargin: dp(5)
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(15)
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                rect_online.color = "#8633E0"
                rect_file.color = "#00000000"
                text_online_selected.visible = true
                text_file_selected.visible = false
            }
        }
    }

    Rectangle {
        id: rect_file
        width: dp(150)
        height: dp(210)
        color: "#00000000"
        radius: dp(10)
        anchors.verticalCenter: rect_online.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: (parent.width / 2 - rect_file.width) / 1.5
        border.color: "#ffffff"
        border.width: dp(2)

        Image {
            id: image_file
            width: dp(80)
            height: dp(80)
            anchors.topMargin: dp(10)
            fillMode: Image.PreserveAspectFit
            source: "../img/File@2x.svg"
            anchors.top: text_file_selected.bottom
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            id: text_file_selected
            color: "#ffffff"
            text: qsTr("SELECTED")
            anchors.topMargin: dp(20)
            font.pixelSize: dp(15)
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            visible: false
        }

        Text {
            id: element3
            color: "#ffffff"
            text: qsTr("File")
            anchors.topMargin: dp(10)
            font.bold: true
            font.pixelSize: dp(21)
            anchors.top: image_file.bottom
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Rectangle {
            id: rect_splitter2
            height: 1
            color: "#ffffff"
            anchors.topMargin: dp(5)
            anchors.rightMargin: dp(20)
            anchors.leftMargin: dp(20)
            anchors.right: parent.right
            anchors.top: element3.bottom
            anchors.left: parent.left
        }

        Text {
            id: element4
            color: "#ffffff"
            text: qsTr("Manual Process")
            anchors.topMargin: dp(5)
            font.pixelSize: dp(15)
            anchors.top: rect_splitter2.bottom
            anchors.horizontalCenter: parent.horizontalCenter
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                rect_online.color = "#00000000"
                rect_file.color = "#8633E0"
                text_online_selected.visible = false
                text_file_selected.visible = true
            }
        }
    }

    Text {
        id: text_description1
        color: "#ffffff"
        text: qsTr("- Transaction will not be finalized if the destination wallet is offline and not listening for the destination address.")
        anchors.bottom: text_description2.top
        anchors.bottomMargin: dp(5)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        font.pixelSize: dp(15)
    }

    Text {
        id: text_description2
        color: "#ffffff"
        text: qsTr("- Your funds at that output will be blocked until your transaction is finalized")
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        font.pixelSize: dp(15)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.verticalCenter: parent.verticalCenter
    }

    Text {
        id: text_description3
        color: "#ffffff"
        text: qsTr("- You can cancel any non finalized transaction to unblock your funds at 'Transactions'")
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        font.pixelSize: dp(15)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.top: text_description2.bottom
        anchors.topMargin: dp(5)
    }

    TextField {
        id: textfield_amount
        width: parent.width - dp(180)
        height: dp(50)
        padding: dp(5)
        leftPadding: dp(10)
        font.pixelSize: dp(18)
        placeholderText: qsTr("Amount")
        color: "white"
        text: ""
        anchors.top: accountComboBox.bottom
        anchors.topMargin: dp(20)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        horizontalAlignment: Text.AlignLeft


        background: Rectangle {
            color: "#8633E0"
            radius: dp(5)
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                textfield_amount.focus = true
            }
        }
    }

    Button {
        id: button_all
        width: dp(100)
        height: dp(50)
        anchors.verticalCenter: textfield_amount.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("All")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
            if (accountComboBox.currentIndex >= 0) {
                const account = accountItems.get(accountComboBox.currentIndex).account
                const amount = send.getSpendAllAmount(account)
                textfield_amount.text = amount
            }
        }
    }

    Button {
        id: button_next
        width: dp(150)
        height: dp(50)
        anchors.top: button_all.bottom
        anchors.topMargin: dp(50)
        anchors.horizontalCenter: parent.horizontalCenter
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Next")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
            if (accountComboBox.currentIndex >= 0) {
                const account = accountItems.get(accountComboBox.currentIndex).account
                const sendAmount = textfield_amount.text.trim()
                const res = send.initialSendSelection( text_online_selected.visible, account, sendAmount );
                if (res === 1)
                    accountComboBox.focus = true
                else if (res === 2)
                    textfield_amount.focus = true
            }
        }
    }

    ComboBox {
        id: accountComboBox
        height: dp(50)
        anchors.top: text_description3.bottom
        anchors.topMargin: dp(30)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        leftPadding: dp(20)
        rightPadding: dp(40)
        font.pixelSize: dp(18)

        onCurrentIndexChanged: {
            if (accountComboBox.currentIndex >= 0) {
                const account = accountItems.get(accountComboBox.currentIndex).account
                wallet.switchAccount(account)
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
}
