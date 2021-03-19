import QtQuick 2.12
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import WalletBridge 1.0
import ConfigBridge 1.0
import OutputsBridge 1.0
import UtilBridge 1.0

Item {
    property bool canLockOutputs
    property bool showAll
    property var allData: []

    WalletBridge {
        id: wallet
    }

    OutputsBridge {
        id: outputs
    }

    ConfigBridge {
        id: config
    }

    UtilBridge {
        id: util
    }

    Connections {
        target: wallet
        onSgnOutputs: (account, showSpent, height, outputs) => {
            if (account !== currentSelectedAccount() || showSpent !== config.isShowOutputAll() )
                return
            rect_progress.visible = false
            allData = []
            outputs.forEach(output => {
                allData.push(JSON.parse(output))
            })
            updateShownData()
        }

        onSgnWalletBalanceUpdated: {
            updateAccountsData()
        }

        onSgnNewNotificationMessage: (level, message) => {
            if (message.includes("Changing status for output")) {
                refreshOutputs()
            }
        }
    }

    function updateAccountsData() {
        const accounts = wallet.getWalletBalance(true, false, true)
        const selectedAccount = wallet.getCurrentAccountName()
        let selectedAccIdx = 0

        accountItems.clear()

        let idx = 0
        for (let i = 1; i < accounts.length; i += 2) {
            if (accounts[i-1] === selectedAccount)
                selectedAccIdx = idx

            accountItems.append({ info: accounts[i], account: accounts[i-1]})
            idx++
        }
        accountComboBox.currentIndex = selectedAccIdx
        return currentSelectedAccount()
    }

    function currentSelectedAccount() {
        if (accountComboBox.currentIndex >= 0)
            return accountItems.get(accountComboBox.currentIndex).account
    }

    function isInteger(value) {
      return /^\d+$/.test(value)
    }

    function updateShownData() {
        outputsModel.clear()
        for ( let idx = allData.length - 1; idx >= Math.max(0, allData.length - 5000); idx--) {
            const output = allData[idx]
            let hok = isInteger(output.blockHeight)
            let height
            if (hok) {
                height = parseInt(output.blockHeight)
            }
            let lok = isInteger(output.lockedUntil)
            let lockH
            if (lok) {
                lockH = parseInt(output.lockedUntil)
            }
            outputsModel.append({
                txIdx: output.txIdx,
                outputStatus: output.status,
                coinbase: output.coinbase ? "Yes" : "No",
                valueNano: util.nano2one(output.valueNano) + " MWC",
                numOfConfirms: output.numOfConfirms,
                outputCommitment: output.outputCommitment,
                blockHeight: output.blockHeight,
                lockedUntil: lok && hok && lockH > height ? output.lockedUntil : "",
            })
        }
    }

    function requestOutputs(account) {
        if (account) {
            allData = []
            outputsModel.clear()
            rect_progress.visible = true
            updateShownData()
            wallet.requestOutputs(account, config.isShowOutputAll(), true)
        }
    }

    function refreshOutputs() {
        requestOutputs(currentSelectedAccount())
    }

    function getOutputTypeIcon(outputStatus, coinbase) {
        switch (outputStatus) {
            case "Unconfirmed":
                return "../img/Transactions_Unconfirmed@2x.svg"
            case "Unspent":
                if (coinbase) {
                    return "../img/Transactions_CoinBase@2x.svg"
                }
                return "../img/Outputs_Unspent@2x.svg"
            case "Locked":
                return "../img/iconLock@2x.svg"
            case "Spent":
                return "../img/Outputs_Spent@2x.svg"
        }
    }

    onVisibleChanged: {
        if (visible) {
            rect_progress.visible = false
            showAll = config.isShowOutputAll()
            const accName = updateAccountsData();
            canLockOutputs = config.isLockOutputEnabled();
            requestOutputs(accName);
        }
    }

    ListModel {
        id: outputsModel
    }

    ListView {
        id: outputsList
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: rect_accounts.bottom
        anchors.topMargin: dp(15)
        model: outputsModel
        delegate: outputsDelegate
        focus: true
    }

    Component {
        id: outputsDelegate
        Rectangle {
            height: dp(215)
            color: "#00000000"
            anchors.left: parent.left
            anchors.right: parent.right

            Rectangle {
                height: dp(205)
                color: "#33bf84ff"
                anchors.top: parent.top
                anchors.topMargin: dp(10)
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                anchors.left: parent.left
                anchors.leftMargin: dp(20)

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (index < 0 || index >= allData.length)
                            return
                        const output = outputsModel.get(index)
                        const outputNote = config.getOutputNote(output.outputCommitment)
                        outputDetailItem.open(output, outputNote)
                    }
                }

                Rectangle {
                    width: dp(10)
                    height: parent.height
                    anchors.top: parent.top
                    anchors.left: parent.left
                    color: "#BCF317"
                    visible: outputStatus === "Unconfirmed"
                }

                Image {
                    width: dp(17)
                    height: dp(17)
                    anchors.top: parent.top
                    anchors.topMargin: dp(20)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    fillMode: Image.PreserveAspectFit
                    source: getOutputTypeIcon(outputStatus, coinbase)
                }

                Text {
                    color: "#ffffff"
                    text: outputStatus
                    font.bold: true
                    font.pixelSize: dp(15)
                    anchors.top: parent.top
                    anchors.topMargin: dp(22)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(71)
                }

                Text {
                    width: dp(200)
                    height: dp(15)
                    color: "#bf84ff"
                    text: "Transaction #:" + txIdx
                    horizontalAlignment: Text.AlignRight
                    font.pixelSize: dp(15)
                    anchors.top: parent.top
                    anchors.topMargin: dp(22)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(35)
                }

                Rectangle {
                    height: dp(1)
                    color: "#ffffff"
                    anchors.top: parent.top
                    anchors.topMargin: dp(55)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(35)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                }

                Text {
                    color: "#ffffff"
                    text: valueNano
                    font.bold: true
                    anchors.top: parent.top
                    anchors.topMargin: dp(70)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    font.pixelSize: dp(15)
                }

                Text {
                    color: "#ffffff"
                    text: "Confirmations: " + numOfConfirms
                    anchors.top: parent.top
                    anchors.topMargin: dp(95)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    font.pixelSize: dp(15)
                }

                Text {
                    color: "#ffffff"
                    text: "Commitment: " + outputCommitment
                    elide: Text.ElideMiddle
                    anchors.top: parent.top
                    anchors.topMargin: dp(120)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(35)
                    font.pixelSize: dp(15)
                }

                Text {
                    color: "#ffffff"
                    text: "Coinbase: " + coinbase
                    anchors.top: parent.top
                    anchors.topMargin: dp(145)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    font.pixelSize: dp(15)
                }

                Text {
                    color: "#ffffff"
                    text: "Height: " + blockHeight
                    anchors.top: parent.top
                    anchors.topMargin: dp(170)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    font.pixelSize: dp(15)
                }

                Text {
                    color: "#ffffff"
                    text: "Lock Height: " + lockedUntil
                    visible: lockedUntil !== ""
                    anchors.top: parent.top
                    anchors.topMargin: dp(170)
                    anchors.left: parent.left
                    anchors.leftMargin: parent.width / 2
                    font.pixelSize: dp(15)
                }
            }
        }
    }

    Rectangle {
        id: rect_accounts
        height: dp(200)
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0
                color: "#9E00E7"
            }

            GradientStop {
                position: 1
                color: "#3600C9"
            }
        }

        Text {
            id: label_combobox
            color: "#ffffff"
            text: qsTr("Showing Outputs From This Account")
            anchors.left: parent.left
            anchors.leftMargin: dp(30)
            anchors.top: parent.top
            anchors.topMargin: dp(20)
            font.pixelSize: dp(14)
        }

        ComboBox {
            id: accountComboBox
            height: dp(60)
            anchors.right: parent.right
            anchors.rightMargin: dp(30)
            anchors.left: parent.left
            anchors.leftMargin: dp(30)
            anchors.top: label_combobox.bottom
            anchors.topMargin: dp(10)
            leftPadding: dp(20)
            rightPadding: dp(40)
            font.pixelSize: dp(15)

            onCurrentIndexChanged: {
                // Selecting the active account
                const selectedAccount = currentSelectedAccount();
                if (selectedAccount !== "") {
                    wallet.switchAccount(selectedAccount);
                    requestOutputs(selectedAccount);
                }
            }

            delegate: ItemDelegate {
                width: accountComboBox.width
                height: dp(60)
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
                implicitHeight: dp(60)
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

        Image {
            id: image_refresh
            anchors.left: parent.left
            anchors.leftMargin: dp(45)
            anchors.top: accountComboBox.bottom
            anchors.topMargin: dp(30)
            width: dp(35)
            height: dp(35)
            fillMode: Image.PreserveAspectFit
            source: "../img/Refresh@2x.svg"

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    refreshOutputs()
                }
            }
        }

        Button {
            id: button_output
            width: dp(220)
            height: dp(40)
            anchors.verticalCenter: image_refresh.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            background: Rectangle {
                color: "#00000000"
                radius: dp(37)
                border.width: dp(2)
                border.color: "white"

                Rectangle {
                    color: showAll ? "white" : "#00000000"
                    radius: dp(37)
                    anchors.top: parent.top
                    anchors.topMargin: dp(4)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(4)
                    width: dp(100)
                    height: dp(32)

                    Text {
                        text: qsTr("All")
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: dp(15)
                        color: showAll ? "#6F00D6" : "white"
                    }
                }

                Rectangle {
                    color: showAll ? "#00000000" : "white"
                    radius: dp(37)
                    anchors.top: parent.top
                    anchors.topMargin: dp(4)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(4)
                    width: dp(120)
                    height: dp(32)

                    Text {
                        text: qsTr("Unspent")
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: dp(15)
                        color: showAll ? "white" : "#6F00D6"
                    }
                }
            }
            onClicked: {
                showAll = !config.isShowOutputAll()
                config.setShowOutputAll(showAll)
                refreshOutputs()
            }
        }
    }

    Rectangle {
        id: rect_progress
        width: dp(60)
        height: dp(30)
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#00000000"
        visible: false
        AnimatedImage {
            id: animation
            source: "../img/loading.gif"
        }
    }
}
