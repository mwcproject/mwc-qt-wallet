import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
import WalletBridge 1.0
import AccountTransferBridge 1.0

Item {
    WalletBridge {
        id: wallet
    }

    AccountTransferBridge {
        id: accountTransfer
    }

    Connections {
        target: accountTransfer
        onSgnShowTransferResults: (ok, errMsg) => {
            rect_progress.visible = false
            if (ok) {
                messagebox.open(qsTr("Success"), qsTr("Your funds were successfully transferred"))
                // reset state
                textfield_amount.text = ""
                updateAccounts()
            } else {
                messagebox.open(qsTr("Transfer failure"), qsTr("Funds transfer request has failed.\n" + errMsg))
            }
        }

        onSgnUpdateAccounts: {
            updateAccounts()
        }

        onSgnHideProgress: {
            rect_progress.visible = false
        }
    }

    function updateAccounts() {
        const accountInfo = wallet.getWalletBalance(true, false, true)

        const fromAcc = combobox_account_from.currentIndex >= 0 ? listmodel_account_from.get(combobox_account_from.currentIndex).account : ""
        const toAcc   = combobox_account_to.currentIndex >= 0 ? listmodel_account_to.get(combobox_account_to.currentIndex).account : ""

        listmodel_account_from.clear()
        listmodel_account_to.clear()

        let fromI = -1
        let toI = -1

        let idx = 0
        for (let t = 1; t < accountInfo.length; t += 2) {
            listmodel_account_from.append({ info: accountInfo[t], account: accountInfo[t-1]})
            listmodel_account_to.append({ info: accountInfo[t], account: accountInfo[t-1]})
            if (accountInfo[t-1] === fromAcc)
                fromI = idx
            if (accountInfo[t-1] === toAcc)
                toI = idx
            idx++
        }
        combobox_account_from.currentIndex = fromI
        combobox_account_to.currentIndex = toI
    }

    onVisibleChanged: {
        rect_progress.visible = false
        updateAccounts()
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            textfield_amount.focus = false
        }
    }

    Text {
        id: label_from_account
        text: qsTr("Transfer MWC from account:")
        color: "white"
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.bottom: combobox_account_from.top
        anchors.bottomMargin: dp(10)
        font.pixelSize: dp(14)
    }

    ComboBox {
        id: combobox_account_from
        anchors.bottom: label_to_account.top
        anchors.bottomMargin: dp(50)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        leftPadding: dp(20)
        rightPadding: dp(20)
        font.pixelSize: dp(18)

        onCurrentIndexChanged: {
            const fromAcc = combobox_account_from.currentIndex >= 0 ? listmodel_account_from.get(combobox_account_from.currentIndex).account : ""
            const toAcc   = combobox_account_to.currentIndex >= 0 ? listmodel_account_to.get(combobox_account_to.currentIndex).account : ""

            if (fromAcc === toAcc && fromAcc !== "") {
                combobox_account_to.currentIndex = -1
            }
        }

        delegate: ItemDelegate {
            width: combobox_account_from.width
            height: dp(40)
            contentItem: Text {
                text: info
                color: "white"
                font: combobox_account_from.font
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                color: combobox_account_from.highlightedIndex === index ? "#955BDD" : "#8633E0"
            }
            topPadding: dp(10)
            bottomPadding: dp(10)
            leftPadding: dp(20)
            rightPadding: dp(20)
        }

        indicator: Canvas {
            id: canvas_from
            x: combobox_account_from.width - width - combobox_account_from.rightPadding
            y: combobox_account_from.topPadding + (combobox_account_from.availableHeight - height) / 2
            width: dp(14)
            height: dp(7)
            contextType: "2d"

            Connections {
                target: combobox_account_from
                function onPressedChanged() { canvas_from.requestPaint() }
            }

            onPaint: {
                context.reset()
                if (combobox_account_from.popup.visible) {
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
            text: combobox_account_from.currentIndex >= 0 && listmodel_account_from.get(combobox_account_from.currentIndex).info
            font: combobox_account_from.font
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
            y: combobox_account_from.height + dp(3)
            width: combobox_account_from.width
            implicitHeight: contentItem.implicitHeight + dp(20)
            topPadding: dp(10)
            bottomPadding: dp(10)
            leftPadding: dp(0)
            rightPadding: dp(0)

            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: combobox_account_from.popup.visible ? combobox_account_from.delegateModel : null
                currentIndex: combobox_account_from.highlightedIndex

                ScrollIndicator.vertical: ScrollIndicator { }
            }

            background: Rectangle {
                color: "#8633E0"
                radius: dp(4)
            }

            onVisibleChanged: {
                if (!combobox_account_from.popup.visible) {
                    canvas_from.requestPaint()
                }
            }
        }

        model: ListModel {
            id: listmodel_account_from
        }
    }

    Text {
        id: label_to_account
        text: qsTr("To account:")
        color: "white"
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.bottom: combobox_account_to.top
        anchors.bottomMargin: dp(10)
        font.pixelSize: dp(14)
    }

    ComboBox {
        id: combobox_account_to
        anchors.bottom: textfield_amount.top
        anchors.bottomMargin: dp(50)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        leftPadding: dp(20)
        rightPadding: dp(20)
        font.pixelSize: dp(18)

        onCurrentIndexChanged: {
            const fromAcc = combobox_account_from.currentIndex >= 0 ? listmodel_account_from.get(combobox_account_from.currentIndex).account : ""
            const toAcc   = combobox_account_to.currentIndex >= 0 ? listmodel_account_to.get(combobox_account_to.currentIndex).account : ""

            if (fromAcc === toAcc && toAcc !== "") {
                combobox_account_from.currentIndex = -1
            }
        }

        delegate: ItemDelegate {
            width: combobox_account_to.width
            height: dp(40)
            contentItem: Text {
                text: info
                color: "white"
                font: combobox_account_to.font
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                color: combobox_account_to.highlightedIndex === index ? "#955BDD" : "#8633E0"
            }
            topPadding: dp(10)
            bottomPadding: dp(10)
            leftPadding: dp(20)
            rightPadding: dp(20)
        }

        indicator: Canvas {
            id: canvas_to
            x: combobox_account_to.width - width - combobox_account_to.rightPadding
            y: combobox_account_to.topPadding + (combobox_account_to.availableHeight - height) / 2
            width: dp(14)
            height: dp(7)
            contextType: "2d"

            Connections {
                target: combobox_account_to
                function onPressedChanged() { canvas_to.requestPaint() }
            }

            onPaint: {
                context.reset()
                if (combobox_account_to.popup.visible) {
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
            text: combobox_account_to.currentIndex >= 0 && listmodel_account_to.get(combobox_account_to.currentIndex).info
            font: combobox_account_to.font
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
            y: combobox_account_to.height + dp(3)
            width: combobox_account_to.width
            implicitHeight: contentItem.implicitHeight + dp(20)
            topPadding: dp(10)
            bottomPadding: dp(10)
            leftPadding: dp(0)
            rightPadding: dp(0)

            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: combobox_account_to.popup.visible ? combobox_account_to.delegateModel : null
                currentIndex: combobox_account_to.highlightedIndex

                ScrollIndicator.vertical: ScrollIndicator { }
            }

            background: Rectangle {
                color: "#8633E0"
                radius: dp(4)
            }

            onVisibleChanged: {
                if (!combobox_account_to.popup.visible) {
                    canvas_to.requestPaint()
                }
            }
        }

        model: ListModel {
            id: listmodel_account_to
        }
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
        anchors.verticalCenter: parent.verticalCenter
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
            textfield_amount.text = "All"
        }
    }

    Rectangle {
        id: rect_progress
        width: dp(60)
        height: dp(30)
        anchors.top: button_all.bottom
        anchors.topMargin: dp(35)
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#00000000"
        visible: false
        AnimatedImage {
            id: animation
            source: "../img/loading.gif"
        }
    }

    Button {
        id: button_back
        width: dp(150)
        height: dp(50)
        anchors.top: button_all.bottom
        anchors.topMargin: dp(100)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Back")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
            accountTransfer.goBack()
        }
    }

    Button {
        id: button_settings
        width: dp(150)
        height: dp(50)
        anchors.top: button_all.bottom
        anchors.topMargin: dp(100)
        anchors.horizontalCenter: parent.horizontalCenter
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Settings")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
            settingsItem.visible = true
        }
    }

    Button {
        id: button_transfer
        width: dp(150)
        height: dp(50)
        anchors.top: button_all.bottom
        anchors.topMargin: dp(100)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Transfer")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
            const fromAcc = combobox_account_from.currentIndex >= 0 ? listmodel_account_from.get(combobox_account_from.currentIndex).account : ""
            const toAcc   = combobox_account_to.currentIndex >= 0 ? listmodel_account_to.get(combobox_account_to.currentIndex).account : ""

            if (fromAcc === "") {
                messagebox.open(qsTr("Incorrect Input"), qsTr("Please select pair of accounts to transfer coins."))
                combobox_account_from.focus = true
                return;
            }

            if (toAcc === "") {
                messagebox.open(qsTr("Incorrect Input"), qsTr("Please select pair of accounts to transfer coins."))
                combobox_account_to.focus = true
                return;
            }

            if (fromAcc === toAcc) {
                messagebox.open(qsTr("Incorrect Input"), qsTr("Please select pair of different accounts to transfer coins."))
                return;
            }

            if (accountTransfer.transferFunds(fromAcc, toAcc, textfield_amount.text.trim())) {
                rect_progress.visible = true
            }
        }
    }


    SendSettings {
        id: settingsItem
        anchors.verticalCenter: parent.verticalCenter
        visible: false
    }

    MessageBox {
        id: messagebox
        anchors.verticalCenter: parent.verticalCenter
    }
}
