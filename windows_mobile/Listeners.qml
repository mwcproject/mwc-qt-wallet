import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import WalletBridge 1.0

Item {
    property bool mqsInProgress
    property bool torInProgress
    property bool warnMsgShown: false

    WalletBridge {
        id: wallet
    }

    Connections {
        target: wallet

        onSgnUpdateListenerStatus: {
            updateStatuses()
        }

        onSgnHttpListeningStatus: (listening, additionalInfo) => {
            updateStatuses()
        }

        onSgnMwcAddressWithIndex: (mwcAddress, idx) => {
            text_mqs_address.text = mwcAddress
            text_mqs_address_index.text = idx >= 0 ? "Address Index: " + Number(idx).toString() : ""
            text_tor_address_index.text = idx >= 0 ? "Address Index: " + Number(idx).toString() : ""
        }

        onSgnListenerStartStop: (mqs, tor) => {
            if (mqs)
                mqsInProgress = false
            if (tor)
                torInProgress = false

            updateStatuses()
        }

        onSgnFileProofAddress: (proofAddress) => {
            text_tor_address.text = "http://" + proofAddress + ".onion"
        }
    }

    function updateStatuses() {
        const mqsStatus = wallet.getMqsListenerStatus()
        const torStatus = wallet.getTorListenerStatus()
        const mqsStarted = wallet.isMqsListenerStarted()
        const torStarted = wallet.isTorListenerStarted()

        // MWC MQ
        image_mqs_status.source = mqsStatus ? "../img/StatusOk@2x.svg" : "../img/StatusEmpty@2x.svg"
        text_mqs_status.text = mqsStatus ? "Online" : "Offline"

        if (mqsStarted) {
            if (mqsStatus) {
                text_button_mqs.text = mqsInProgress ? "Stopping..." : "Stop"
            } else {
                text_button_mqs.text = "Stop to retry"
            }
        } else {
            text_button_mqs.text = mqsInProgress ? "Starting..." : "Start"
        }
        button_next_address.enabled = !mqsStarted && !torStarted
        button_to_index.enabled = !mqsStarted && !torStarted

        // TOR
        image_tor_status.source = torStatus ? "../img/StatusOk@2x.svg" : "../img/StatusEmpty@2x.svg"
        text_tor_status.text = torStatus ? "Online" : "Offline"

        if (torStarted) {
            if (torStatus) {
                text_button_tor.text = torInProgress ? "Stopping..." : "Stop"
            } else {
                text_button_tor.text = "Stop to retry"
            }
        } else {
            text_button_tor.text = torInProgress ? "Starting" : "Start"
        }
    }

    function nextAddressCallback(ret) {
        if (ret) {
            warnMsgShown = true
            wallet.requestNextMqsAddress()
            wallet.requestFileProofAddress()
        }
    }

    function isInteger(value) {
      return /^\d+$/.test(value)
    }

    function toIndexCallback(ok, index) {
        if (!ok || index === "")
            return;

        ok = isInteger(index)
        if (!ok) {
            messagebox.open(qsTr("Wrong value"), qsTr("Please input integer in the range from 0 to 65000"))
            return;
        }
        const idx = parseInt(index)

        if (idx < 0 || idx > 65000) {
            messagebox.open(qsTr("Wrong value"), qsTr("Please input integer in the range from 0 to 65000"))
            return;
        }

        wallet.requestChangeMqsAddress(idx)
        wallet.requestFileProofAddress()
    }

    onVisibleChanged: {
        if (visible) {
            if (parent.height > dp(440)) {
                anchors.topMargin = (parent.height - dp(440)) / 2
            }

            updateStatuses()
            wallet.requestFileProofAddress()
            wallet.requestMqsAddress()
        }
    }

    Rectangle {
        id: rect_mqs_status
        width: dp(150)
        height: dp(40)
        color: "#00000000"
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        anchors.top: parent.top

        Text {
            id: label_mqs
            text: qsTr("MWC MQS")
            anchors.left: parent.left
            anchors.top: parent.top
            font.pixelSize: dp(20)
            font.bold: true
            color: "white"
        }

        Text {
            id: text_mqs_status
            text: qsTr("(Online)")
            anchors.horizontalCenter: label_mqs.horizontalCenter
            anchors.top: label_mqs.bottom
            anchors.topMargin: dp(5)
            font.pixelSize: dp(15)
            color: "white"
        }

        Image {
            id: image_mqs_status
            width: dp(20)
            height: dp(20)
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: dp(15)
            fillMode: Image.PreserveAspectFit
            source: "../img/StatusOk@2x.svg"
        }
    }

    Button {
        id: button_mqs
        width: dp(100)
        height: dp(40)
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.bottom: rect_mqs_status.bottom
        background: Rectangle {
            color: "#00000000"
            radius: dp(5)
            border.color: "white"
            border.width: dp(2)
            Text {
                id: text_button_mqs
                text: qsTr("Start")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(15)
                font.bold: true
                color: "white"
            }
        }

        onClicked: {
            if (mqsInProgress)
                return;
            mqsInProgress = true

            if (wallet.isMqsListenerStarted()) {
                text_button_mqs.text = "Stopping..."
                wallet.requestStopMqsListener()
            }
            else {
                text_button_mqs.text = "Starting..."
                wallet.requestStartMqsListener()
            }
        }
    }

    Rectangle {
        id: rect_mqs_address
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        anchors.top: button_mqs.bottom
        anchors.topMargin: dp(20)
        color: "#8633E0"
        radius: dp(5)
        height: dp(30) + text_mqs_address.height

        Text {
            id: text_mqs_address
            anchors.top: parent.top
            anchors.topMargin: dp(15)
            anchors.left: parent.left
            anchors.leftMargin: dp(20)
            anchors.right: parent.right
            anchors.rightMargin: dp(20)
            wrapMode: Text.WrapAnywhere
            font.pixelSize: dp(15)
            color: "white"
            text: "xmgKiuQxaPLozdD59hfB1vES3aPBdnTD51oG2CbqGbNStN4kjm1k"
        }
    }

    Text {
        id: text_mqs_address_index
        anchors.top: rect_mqs_address.bottom
        anchors.topMargin: dp(5)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        font.pixelSize: dp(15)
        color: "#e2ccf7"
        text: "Address Index: 0"
    }

    Rectangle {
        id: rect_tor_status
        width: dp(150)
        height: dp(40)
        color: "#00000000"
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        anchors.top: text_mqs_address_index.bottom
        anchors.topMargin: dp(30)

        Text {
            id: label_tor
            text: qsTr("Tor")
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            font.pixelSize: dp(20)
            font.bold: true
            color: "white"
        }

        Text {
            id: text_tor_status
            text: qsTr("(Online)")
            anchors.left: label_tor.right
            anchors.leftMargin: dp(5)
            anchors.bottom: parent.bottom
            font.pixelSize: dp(15)
            color: "white"
        }

        Image {
            id: image_tor_status
            width: dp(20)
            height: dp(20)
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            fillMode: Image.PreserveAspectFit
            source: "../img/StatusOk@2x.svg"
        }
    }

    Button {
        id: button_tor
        width: dp(100)
        height: dp(40)
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.bottom: rect_tor_status.bottom
        background: Rectangle {
            color: "#00000000"
            radius: dp(5)
            border.color: "white"
            border.width: dp(2)
            Text {
                id: text_button_tor
                text: qsTr("Start")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(15)
                font.bold: true
                color: "white"
            }
        }

        onClicked: {
            if (torInProgress)
                return;
            torInProgress = true;

            if (wallet.isTorListenerStarted()) {
                text_button_tor.text = "Stopping..."
                wallet.requestStopTorListener()
            }
            else {
                text_button_tor.text = "Starting..."
                wallet.requestStartTorListener()
            }
        }
    }

    Rectangle {
        id: rect_tor_address
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        anchors.top: rect_tor_status.bottom
        anchors.topMargin: dp(20)
        color: "#8633E0"
        radius: dp(5)
        height: dp(30) + text_tor_address.height

        Text {
            id: text_tor_address
            anchors.top: parent.top
            anchors.topMargin: dp(15)
            anchors.left: parent.left
            anchors.leftMargin: dp(20)
            anchors.right: parent.right
            anchors.rightMargin: dp(20)
            wrapMode: Text.WrapAnywhere
            font.pixelSize: dp(15)
            color: "white"
            text: ""
        }
    }

    Text {
        id: text_tor_address_index
        anchors.top: rect_tor_address.bottom
        anchors.topMargin: dp(5)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        font.pixelSize: dp(15)
        color: "#e2ccf7"
        text: "Address Index: 0"
    }

    Text {
        id: label_next_address
        anchors.top: text_tor_address_index.bottom
        anchors.topMargin: dp(30)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        font.pixelSize: dp(15)
        wrapMode: Text.WordWrap
        color: "white"
        text: qsTr("Please stop TOR and MWC MQS listeners in order to change the wallet address")
    }

    Button {
        id: button_next_address
        width: dp(170)
        height: dp(40)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        anchors.top: label_next_address.bottom
        anchors.topMargin: dp(15)
        background: Rectangle {
            color: "#00000000"
            radius: dp(5)
            border.color: button_next_address.enabled ? "white" : "#e2ccf7"
            border.width: dp(2)
            Text {
                text: qsTr("Next Address")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(15)
                font.bold: true
                color: button_next_address.enabled ? "white" : "#e2ccf7"
            }
        }

        onClicked: {
            if (warnMsgShown)
                return;

            messagebox.open("Warning", "Please note that your wallet will only listen to one address at a time. You are now setting the wallet to listen to different address.\n\nDo you want to continue?",
                                        true, "Cancel", "Continue", "", "", "", nextAddressCallback)
        }
    }

    Button {
        id: button_to_index
        width: dp(150)
        height: dp(40)
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.verticalCenter: button_next_address.verticalCenter
        background: Rectangle {
            color: "#00000000"
            radius: dp(5)
            border.color: button_to_index.enabled ? "white" : "#e2ccf7"
            border.width: dp(2)
            Text {
                text: qsTr("To index")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(15)
                font.bold: true
                color: button_to_index.enabled ? "white" : "#e2ccf7"
            }
        }

        onClicked: {
            inputDlg.open(qsTr("Select MQS address by index"), qsTr("Please specify index of MWC MQS address.\n\nPlease note that your wallet will only listen to one address at a time. You are now setting the wallet to listen to different address."), "integer from 0 to 65000", "", 5, toIndexCallback)
        }
    }

    InputDlg {
        id: inputDlg
        anchors.verticalCenter: parent.verticalCenter
    }
}
