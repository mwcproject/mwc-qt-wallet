import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import ConfigBridge 1.0
import WalletConfigBridge 1.0
import UtilBridge 1.0

Item {
    property string walletInstanceName
    property int inputConfirmationsNumber
    property int changeOutputs
    property bool walletLogsEnabled
    property bool autoStartMQSEnabled
    property bool autoStartTorEnabled

    WalletConfigBridge {
        id: walletConfig
    }

    ConfigBridge {
        id: config
    }

    function updateButtons() {
        const currentWalletLogsEnabled = combobox_wallet_logs.currentIndex === 0 ? true: false
        const sameWithCurrent =
            textfield_wallet_instance_name.text.trim() === walletInstanceName &&
            walletLogsEnabled === currentWalletLogsEnabled &&
            textfield_send_num_confirmation.text.trim() == Number(inputConfirmationsNumber).toString() &&
            textfield_change_outputs.text.trim() === Number(changeOutputs).toString() &&
            autoStartMQSEnabled === checkbox_mqs.checked &&
            autoStartTorEnabled === checkbox_tor.checked

        walletConfig.canApplySettings(!sameWithCurrent);

        const sameWithDefault =
            true === currentWalletLogsEnabled &&
            textfield_send_num_confirmation.text.trim() === Number(walletConfig.getDefaultInputConfirmationsNumber()).toString() &&
            textfield_change_outputs.text.trim() === Number(walletConfig.getDefaultChangeOutputs()).toString() &&
            checkbox_mqs.checked === true &&
            checkbox_tor.checked === true

        button_reset.enabled = !sameWithDefault
        button_apply.enabled = !sameWithCurrent
    }

    function isInteger(value) {
      return /^\d+$/.test(value)
    }

    function logEnableUpdateCallback(needCleanupLogs) {
        walletConfig.updateWalletLogsEnabled(false, needCleanupLogs)
    }

    onVisibleChanged: {
        if (visible) {
            walletLogsEnabled = walletConfig.getWalletLogsEnabled()
            combobox_wallet_logs.currentIndex = walletLogsEnabled ? 0 : 1
            autoStartMQSEnabled = walletConfig.getAutoStartMQSEnabled()
            checkbox_mqs.checked = autoStartMQSEnabled
            autoStartTorEnabled = walletConfig.getAutoStartTorEnabled()
            checkbox_tor.checked = autoStartTorEnabled
            const instanceInfo = config.getCurrentWalletInstance()
            walletInstanceName = instanceInfo[2]
            textfield_wallet_instance_name.text = walletInstanceName
            inputConfirmationsNumber = walletConfig.getInputConfirmationsNumber()
            textfield_send_num_confirmation.text = inputConfirmationsNumber
            changeOutputs = walletConfig.getChangeOutputs()
            textfield_change_outputs.text = changeOutputs
            updateButtons()
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            textfield_wallet_instance_name.focus = false
            textfield_send_num_confirmation.focus = false
            textfield_change_outputs.focus = false
        }
    }

    Text {
        id: label_auto_start
        text: qsTr("Auto Start")
        color: "white"
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        anchors.bottom: rect_auto_start.top
        anchors.bottomMargin: dp(10)
        font.pixelSize: dp(14)
    }

    Rectangle {
        id: rect_auto_start
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.bottom: label_wallet_instance_name.top
        anchors.bottomMargin: dp(15)
        border.width: dp(1)
        border.color: "white"
        color: "#00000000"
        height: dp(140)

        CheckBox {
            id: checkbox_mqs
            text: qsTr("MWC MQS")
            font.pixelSize: dp(17)
            anchors.left: parent.left
            anchors.leftMargin: parent.width / 2 - dp(100)
            anchors.top: parent.top
            anchors.topMargin: dp(35)

            indicator: Rectangle {
                implicitWidth: dp(20)
                implicitHeight: dp(20)
                x: checkbox_mqs.leftPadding
                y: parent.height / 2 - height / 2

                Image {
                    width: dp(20)
                    height: dp(20)
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                    source: checkbox_mqs.checked ? "../img/CheckOn@2x.svg" : "../img/CheckOff@2x.svg"
                }
            }

            contentItem: Text {
                text: checkbox_mqs.text
                font: checkbox_mqs.font
                color: "white"
                verticalAlignment: Text.AlignVCenter
                anchors.left: checkbox_mqs.indicator.right
                anchors.leftMargin: dp(25)
            }

            onCheckStateChanged: {
                updateButtons()
            }
        }

        CheckBox {
            id: checkbox_tor
            text: qsTr("TOR")
            font.pixelSize: dp(17)
            anchors.left: parent.left
            anchors.leftMargin: parent.width / 2 - dp(100)
            anchors.top: checkbox_mqs.bottom
            anchors.topMargin: dp(15)

            indicator: Rectangle {
                implicitWidth: dp(20)
                implicitHeight: dp(20)
                x: checkbox_tor.leftPadding
                y: parent.height / 2 - height / 2

                Image {
                    width: dp(20)
                    height: dp(20)
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                    source: checkbox_tor.checked ? "../img/CheckOn@2x.svg" : "../img/CheckOff@2x.svg"
                }
            }

            contentItem: Text {
                text: checkbox_tor.text
                font: checkbox_tor.font
                color: "white"
                verticalAlignment: Text.AlignVCenter
                anchors.left: checkbox_tor.indicator.right
                anchors.leftMargin: dp(25)
            }

            onCheckStateChanged: {
                updateButtons()
            }
        }
    }

    Text {
        id: label_wallet_instance_name
        text: qsTr("Wallet instance name")
        color: "white"
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        anchors.bottom: textfield_wallet_instance_name.top
        anchors.bottomMargin: dp(10)
        font.pixelSize: dp(14)
    }

    TextField {
        id: textfield_wallet_instance_name
        height: dp(50)
        padding: dp(10)
        leftPadding: dp(20)
        font.pixelSize: dp(18)
        color: "white"
        text: ""
        anchors.bottom: label_send_num_confirmation.top
        anchors.bottomMargin: dp(15)
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        horizontalAlignment: Text.AlignLeft
        background: Rectangle {
            color: "#8633E0"
            radius: dp(5)
        }
        onTextChanged: {
            updateButtons()
        }
    }

    Text {
        id: label_send_num_confirmation
        text: qsTr("Number of Confirmations (for send)")
        color: "white"
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        anchors.bottom: textfield_send_num_confirmation.top
        anchors.bottomMargin: dp(10)
        font.pixelSize: dp(14)
    }

    TextField {
        id: textfield_send_num_confirmation
        height: dp(50)
        padding: dp(10)
        leftPadding: dp(20)
        font.pixelSize: dp(18)
        color: "white"
        text: ""
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        horizontalAlignment: Text.AlignLeft
        background: Rectangle {
            color: "#8633E0"
            radius: dp(5)
        }
        onTextChanged: {
            updateButtons()
        }
    }

    Text {
        id: label_change_outputs
        text: qsTr("Change Outputs")
        color: "white"
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        anchors.top: textfield_send_num_confirmation.bottom
        anchors.topMargin: dp(15)
        font.pixelSize: dp(14)
    }

    TextField {
        id: textfield_change_outputs
        height: dp(50)
        padding: dp(10)
        leftPadding: dp(20)
        font.pixelSize: dp(18)
        color: "white"
        text: ""
        anchors.top: label_change_outputs.bottom
        anchors.topMargin: dp(10)
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        horizontalAlignment: Text.AlignLeft
        background: Rectangle {
            color: "#8633E0"
            radius: dp(5)
        }
        onTextChanged: {
            updateButtons()
        }
    }

    Text {
        id: label_wallet_logs
        text: qsTr("Wallet Logs")
        color: "white"
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        anchors.top: textfield_change_outputs.bottom
        anchors.topMargin: dp(15)
        font.pixelSize: dp(14)
    }

    ComboBox {
        id: combobox_wallet_logs

        onCurrentIndexChanged: {
            if (combobox_wallet_logs.currentIndex >= 0) {
                updateButtons()
            }
        }

        delegate: ItemDelegate {
            width: combobox_wallet_logs.width
            contentItem: Text {
                text: status
                color: "white"
                font: combobox_wallet_logs.font
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                color: combobox_wallet_logs.highlightedIndex === index ? "#955BDD" : "#8633E0"
            }
            topPadding: dp(10)
            bottomPadding: dp(10)
            leftPadding: dp(20)
            rightPadding: dp(20)
        }

        indicator: Canvas {
            id: canvas
            x: combobox_wallet_logs.width - width - combobox_wallet_logs.rightPadding
            y: combobox_wallet_logs.topPadding + (combobox_wallet_logs.availableHeight - height) / 2
            width: dp(14)
            height: dp(7)
            contextType: "2d"

            Connections {
                target: combobox_wallet_logs
                function onPressedChanged() { canvas.requestPaint() }
            }

            onPaint: {
                context.reset()
                if (combobox_wallet_logs.popup.visible) {
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
            text: combobox_wallet_logs.currentIndex >= 0 && listmodel_wallet_logs.get(combobox_wallet_logs.currentIndex).status
            font: combobox_wallet_logs.font
            color: "white"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            elide: Text.ElideRight
        }

        background: Rectangle {
            implicitHeight: dp(50)
            radius: dp(5)
            color: "#8633E0"
        }

        popup: Popup {
            y: combobox_wallet_logs.height + dp(3)
            width: combobox_wallet_logs.width
            implicitHeight: contentItem.implicitHeight + dp(20)
            topPadding: dp(10)
            bottomPadding: dp(10)
            leftPadding: dp(0)
            rightPadding: dp(0)

            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: combobox_wallet_logs.popup.visible ? combobox_wallet_logs.delegateModel : null
                currentIndex: combobox_wallet_logs.highlightedIndex

                ScrollIndicator.vertical: ScrollIndicator { }
            }

            background: Rectangle {
                color: "#8633E0"
                radius: dp(5)
            }

            onVisibleChanged: {
                if (!combobox_wallet_logs.popup.visible) {
                    canvas.requestPaint()
                }
            }
        }

        model: ListModel {
            id: listmodel_wallet_logs

            ListElement {
                status: "Enabled"
            }
            ListElement {
                status: "Disabled"
            }
        }
        anchors.top: label_wallet_logs.bottom
        anchors.topMargin: dp(10)
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        leftPadding: dp(20)
        rightPadding: dp(20)
        font.pixelSize: dp(18)
    }

    Button {
        id: button_apply
        width: dp(150)
        height: dp(50)
        anchors.bottom: parent.bottom
        anchors.bottomMargin: dp(30)
        anchors.right: parent.right
        anchors.rightMargin: dp(100)

        background: Rectangle {
            color: button_apply.enabled ? "white" : "#00000000"
            radius: dp(5)
            border.width: dp(2)
            border.color: button_apply.enabled ? "white" : "gray"
            Text {
                text: qsTr("Apply")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: button_apply.enabled ? "#6F00D6" : "gray"
            }
        }

        onClicked: {
            const newWalletInstanceName = textfield_wallet_instance_name.text.trim()
            if (newWalletInstanceName === "") {
                messagebox.open(qsTr("Input"), qsTr("Please specify non empty wallet insatance name"))
                textfield_wallet_instance_name.focus = true
                return;
            }

            let ok = isInteger(textfield_send_num_confirmation.text.trim())
            let confirmations
            if (ok) {
                confirmations = parseInt(textfield_send_num_confirmation.text.trim())
            }
            if (!ok || confirmations <= 0 || confirmations > 10) {
                messagebox.open(qsTr("Input"), qsTr("Please input the number of confirmations in the range from 1 to 10"))
                textfield_send_num_confirmation.focus = true
                return;
            }

            ok = isInteger(textfield_change_outputs.text.trim())
            let outputs
            if (ok) {
                outputs = parseInt(textfield_change_outputs.text.trim())
            }
            if (!ok || outputs <= 0 || outputs >= 100) {
                messagebox.open(qsTr("Input"), qsTr("Please input the change output number in the range from 1 to 100"))
                textfield_change_outputs.focus = true
                return
            }

            if (!(confirmations === inputConfirmationsNumber && outputs === changeOutputs)) {
                walletConfig.setSendCoinsParams(confirmations, outputs)
                inputConfirmationsNumber = confirmations
                changeOutputs = outputs
            }

            const currentWalletLogsEnabled = combobox_wallet_logs.currentIndex === 0 ? true : false
            const need2updateLogEnabled = walletLogsEnabled !== currentWalletLogsEnabled
            if (need2updateLogEnabled) {
                if (!currentWalletLogsEnabled) {
                    messagebox.open("Wallet Logs",
                           "You just disabled the logs. Log files location:\n~/mwc-qt-wallet/logs\n
                            Please note, the logs can contain private information about your transactions and accounts.\n
                            Do you want to clean up existing logs from your wallet?",
                            true, "Keep the logs", "Clean up", "", "", "", logEnableUpdateCallback)
                } else {
                    messagebox.open(qsTr("Wallet Logs"), qsTr("You just enabled the logs. Log files location:\n~/mwc-qt-wallet/logs\nPlease note, the logs can contain private infromation about your transactions and accounts."))
                }
            }

            walletLogsEnabled = currentWalletLogsEnabled

            const need2updateAutoStartMQSEnabled = autoStartMQSEnabled !== checkbox_mqs.checked
            if (need2updateAutoStartMQSEnabled) {
                walletConfig.updateAutoStartMQSEnabled(checkbox_mqs.checked)
            }
            autoStartMQSEnabled = checkbox_mqs.checked

            const need2updateAutoStartTorEnabled = autoStartTorEnabled !== checkbox_tor.checked
            if (need2updateAutoStartTorEnabled) {
                walletConfig.updateAutoStartTorEnabled(checkbox_tor.checked)
            }
            autoStartTorEnabled = checkbox_tor.checked

            if (newWalletInstanceName !== walletInstanceName) {
                config.updateActiveInstanceName(newWalletInstanceName)
                walletInstanceName = newWalletInstanceName
            }

            updateButtons()
        }
    }

    Button {
        id: button_reset
        width: dp(150)
        height: dp(50)
        anchors.bottom: parent.bottom
        anchors.bottomMargin: dp(30)
        anchors.left: parent.left
        anchors.leftMargin: dp(100)

        background: Rectangle {
            color: "#00000000"
            radius: dp(5)
            border.width: dp(2)
            border.color: button_reset.enabled ? "white" : "grey"
            Text {
                text: qsTr("Reset")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: button_reset.enabled ? "white" : "grey"
            }
        }

        onClicked: {
            textfield_send_num_confirmation.text = walletConfig.getDefaultInputConfirmationsNumber()
            textfield_change_outputs.text = walletConfig.getDefaultChangeOutputs()
            combobox_wallet_logs.currentIndex = 0
            checkbox_mqs.checked = true
            checkbox_tor.checked = true

            updateButtons()
        }
    }

    MessageBox {
        id: messagebox
        anchors.verticalCenter: parent.verticalCenter
    }
}
