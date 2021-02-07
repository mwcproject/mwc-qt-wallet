import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
import WalletBridge 1.0
import SendBridge 1.0
import ConfigBridge 1.0

Item {
    property bool showGenProofWarning: false
    property int selectedSendMethod: 0

    WalletBridge {
        id: wallet
    }

    SendBridge {
        id: send
    }

    ConfigBridge {
        id: config
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
            rect_progress.visible = false
        }
    }

    function generateProofCallback(ok) {
        if (ok) {
            showGenProofWarning = true
            config.setGenerateProof(true)
        } else {
            checkbox_tx_proof.checked = false
        }
    }

    function selectSendMethod(sendMethod) {
        selectedSendMethod = sendMethod
        switch (sendMethod) {
            case 1: // online_id
                rect_online.color = "#8633E0"
                text_online_selected.visible = true
                rect_file.color = "#00000000"
                text_file_selected.visible = false
                rect_slatepack.color = "#00000000"
                text_slatepack_selected.visible = false
                break;
            case 2: // file_id
                rect_online.color = "#00000000"
                text_online_selected.visible = false
                rect_file.color = "#8633E0"
                text_file_selected.visible = true
                rect_slatepack.color = "#00000000"
                text_slatepack_selected.visible = false
                break;
            case 3: // slatepack_id
                rect_online.color = "#00000000"
                text_online_selected.visible = false
                rect_file.color = "#00000000"
                text_file_selected.visible = false
                rect_slatepack.color = "#8633E0"
                text_slatepack_selected.visible = true
                break;
        }
    }

    onVisibleChanged: {
        if (visible) {
            if (parent.height > dp(560)) {
                anchors.topMargin = (parent.height - dp(560)) / 2
            }
            rect_progress.visible = true
            wallet.requestWalletBalanceUpdate()
            selectSendMethod(config.getSendMethod())
            textfield_amount.text = ""
            checkbox_tx_proof.checked = config.getGenerateProof()
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
        width: dp(130)
        height: dp(180)
        color: "#00000000"
        radius: dp(10)
        anchors.left: parent.left
        anchors.leftMargin: dp(20)
        anchors.top: parent.top
        border.color: "#ffffff"
        border.width: dp(2)

        Image {
            id: image_online
            width: dp(80)
            height: dp(60)
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
            anchors.topMargin: dp(15)
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(14)
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
            font.pixelSize: dp(18)
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
            font.pixelSize: dp(14)
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                selectSendMethod(1)
            }
        }
    }

    Rectangle {
        id: rect_file
        width: dp(130)
        height: dp(180)
        color: "#00000000"
        radius: dp(10)
        anchors.verticalCenter: rect_online.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        border.color: "#ffffff"
        border.width: dp(2)

        Image {
            id: image_file
            width: dp(80)
            height: dp(60)
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
            anchors.topMargin: dp(15)
            font.pixelSize: dp(14)
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
            font.pixelSize: dp(18)
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
            font.pixelSize: dp(14)
            anchors.top: rect_splitter2.bottom
            anchors.horizontalCenter: parent.horizontalCenter
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                selectSendMethod(2)
            }
        }
    }

    Rectangle {
        id: rect_slatepack
        width: dp(130)
        height: dp(180)
        color: "#00000000"
        radius: dp(10)
        anchors.verticalCenter: rect_online.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: dp(20)
        border.color: "#ffffff"
        border.width: dp(2)

        Image {
            id: image_slatepack
            width: dp(80)
            height: dp(60)
            anchors.topMargin: dp(10)
            fillMode: Image.PreserveAspectFit
            source: "../img/File@2x.svg"
            anchors.top: text_slatepack_selected.bottom
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            id: text_slatepack_selected
            color: "#ffffff"
            text: qsTr("SELECTED")
            anchors.topMargin: dp(15)
            font.pixelSize: dp(14)
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            visible: false
        }

        Text {
            id: element5
            color: "#ffffff"
            text: qsTr("Slatepack")
            anchors.topMargin: dp(10)
            font.bold: true
            font.pixelSize: dp(18)
            anchors.top: image_slatepack.bottom
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Rectangle {
            id: rect_splitter3
            height: 1
            color: "#ffffff"
            anchors.topMargin: dp(5)
            anchors.rightMargin: dp(20)
            anchors.leftMargin: dp(20)
            anchors.right: parent.right
            anchors.top: element5.bottom
            anchors.left: parent.left
        }

        Text {
            color: "#ffffff"
            text: qsTr("Manual Process")
            anchors.topMargin: dp(5)
            font.pixelSize: dp(14)
            anchors.top: rect_splitter3.bottom
            anchors.horizontalCenter: parent.horizontalCenter
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                selectSendMethod(3)
            }
        }
    }

    Text {
        id: text_description1
        color: "#ffffff"
        text: qsTr("- Transaction will not be finalized if the destination wallet is offline and not listening for the destination address.")
        anchors.top: rect_online.bottom
        anchors.topMargin: dp(20)
        anchors.right: parent.right
        anchors.rightMargin: dp(20)
        anchors.left: parent.left
        anchors.leftMargin: dp(20)
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        font.pixelSize: dp(14)
    }

    Text {
        id: text_description2
        color: "#ffffff"
        text: qsTr("- Your funds at that output will be blocked until your transaction is finalized")
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        font.pixelSize: dp(14)
        anchors.right: parent.right
        anchors.rightMargin: dp(20)
        anchors.left: parent.left
        anchors.leftMargin: dp(20)
        anchors.top: text_description1.bottom
        anchors.topMargin: dp(5)
    }

    Text {
        id: text_description3
        color: "#ffffff"
        text: qsTr("- You can cancel any non finalized transaction to unblock your funds at 'Transactions'")
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        font.pixelSize: dp(14)
        anchors.right: parent.right
        anchors.rightMargin: dp(20)
        anchors.left: parent.left
        anchors.leftMargin: dp(20)
        anchors.top: text_description2.bottom
        anchors.topMargin: dp(5)
    }

    TextField {
        id: textfield_amount
        width: parent.width - dp(180)
        height: dp(50)
        padding: dp(5)
        leftPadding: dp(20)
        font.pixelSize: dp(18)
        placeholderText: qsTr("Amount")
        color: "white"
        text: ""
        anchors.top: accountComboBox.bottom
        anchors.topMargin: dp(20)
        anchors.left: parent.left
        anchors.leftMargin: dp(20)
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
        anchors.rightMargin: dp(20)
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

    CheckBox {
        id: checkbox_tx_proof
        text: qsTr("Generate Transaction Proof")
        font.pixelSize: dp(17)
        anchors.left: parent.left
        anchors.leftMargin: dp(20)
        anchors.top: textfield_amount.bottom
        anchors.topMargin: dp(20)

        indicator: Rectangle {
            implicitWidth: dp(20)
            implicitHeight: dp(20)
            x: checkbox_tx_proof.leftPadding
            y: parent.height / 2 - height / 2

            Image {
                width: dp(20)
                height: dp(20)
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: checkbox_tx_proof.checked ? "../img/CheckOn@2x.svg" : "../img/CheckOff@2x.svg"
            }
        }

        contentItem: Text {
            text: checkbox_tx_proof.text
            font: checkbox_tx_proof.font
            color: "white"
            verticalAlignment: Text.AlignVCenter
            anchors.left: checkbox_tx_proof.indicator.right
            anchors.leftMargin: dp(25)
        }

        onCheckStateChanged: {
             if (checkbox_tx_proof.checked && !showGenProofWarning) {
                 messagebox.open("Warning", "Transaction proof generation requires receiver wallet version 1.0.23 or higher.\n\nDo you want to generate proofs for all your send transactions?", true, "Cancel", "Generate", "", "", "", generateProofCallback)
             } else {
                 config.setGenerateProof(checkbox_tx_proof.checked)
             }
        }
    }

    Rectangle {
        id: rect_progress
        width: dp(60)
        height: dp(30)
        anchors.top: checkbox_tx_proof.bottom
        anchors.topMargin: dp(30)
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#00000000"
        visible: false
        AnimatedImage {
            id: animation
            source: "../img/loading.gif"
        }
    }

    Button {
        id: button_next
        width: dp(150)
        height: dp(50)
        anchors.top: checkbox_tx_proof.bottom
        anchors.topMargin: dp(20)
        anchors.horizontalCenter: parent.horizontalCenter
        visible: !rect_progress.visible
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
                config.setSendMethod(selectedSendMethod)
                const res = send.initialSendSelection( selectedSendMethod, account, sendAmount );
                if (res === 1)
                    accountComboBox.focus = true
                else if (res === 2)
                    textfield_amount.focus = true
            }
        }
    }

    ComboBox {
        id: accountComboBox
        anchors.top: text_description3.bottom
        anchors.topMargin: dp(20)
        anchors.right: parent.right
        anchors.rightMargin: dp(20)
        anchors.left: parent.left
        anchors.leftMargin: dp(20)
        leftPadding: dp(20)
        rightPadding: dp(20)
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
                color: "white"
                font: accountComboBox.font
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
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
            x: accountComboBox.width - width - accountComboBox.rightPadding
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
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            elide: Text.ElideRight
        }

        background: Rectangle {
            implicitHeight: dp(50)
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
                radius: dp(4)
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
}
