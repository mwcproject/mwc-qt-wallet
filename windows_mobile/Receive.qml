import QtQuick 2.15
import QtQuick.Controls 2.13
import Qt.labs.platform 1.1
import QtQuick.Window 2.0
import ConfigBridge 1.0
import ReceiveBridge 1.0
import WalletBridge 1.0
import UtilBridge 1.0
import Clipboard 1.0
import QtAndroidService 1.0
import "./models"

Item {
    property string mwcmqAddr
    property string torAddr
    property string proofAddr

    property bool mqsInProgress
    property bool torInProgress
    property bool warnMsgShown: false

    property int selectedReceiveMethod: 1


    anchors.fill: parent

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

    QtAndroidService {
        id: qtAndroidService
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
        target: qtAndroidService
        onSgnOnFileReady: (eventCode, path ) => {
            if (eventCode === 123 && path) {
                        console.log("Open initial transaction file: " + path)
                        const validation = util.validateMwc713Str(path)
                        if (validation) {
                            messagebox.open(qsTr("File Path"), qsTr("This file path is not acceptable.\n" + validation))
                            return
                        }
                        config.updatePathFor("fileGen", path)
                        receive.signTransaction(path)
            }
        }
    }


    Connections {
        target: wallet

        onSgnMwcAddressWithIndex: (mwcmq, _index) => {
            mwcmqAddr = "mwcmqs://%1".arg(mwcmq)
            updateStatus()
        }

        onSgnTorAddress: {
            updateStatus()
        }

        onSgnUpdateListenerStatus: {
            updateStatus()
        }

        onSgnListenerStartStop: (mqs, tor) => {
            if (mqs)
                mqsInProgress = false
            if (tor)
                torInProgress = false

            updateStatuses()
        }

        onSgnFileProofAddress: (proofAddress) => {
            proofAddr = proofAddress
            torAddr = qsTr("http://%1.onion").arg(proofAddr)

        }


    }

    onVisibleChanged: {
        if (visible) {
            if (config.hasTls()) {
                text_http.text = qsTr("Https")
            }
            requestAddress()
        } else {
            text_mqs_address.focus = false
            text_tor_address.focus = false
            text_slate_address.focus = false
        }
    }

    function requestAddress() {
        wallet.requestFileProofAddress()
        wallet.requestMqsAddress()
        updateStatus()
    }

    function updateStatus() {
        indicator_mwcmq.color = wallet.getMqsListenerStatus() ? "green" : "red"
        indicator_tor.color = wallet.getTorListenerStatus() ? "green" : "red"
    }


    function slatepackCallback(ok, slatepack, slateJson, sender) {
        if (ok) {
            receive.signSlatepackTransaction(slatepack, slateJson, sender)
        }
    }

    function updateStatuses() {
        const mqsStatus = wallet.getMqsListenerStatus()
        const torStatus = wallet.getTorListenerStatus()
        const mqsStarted = wallet.isMqsListenerStarted()
        const torStarted = wallet.isTorListenerStarted()

        // MWC MQ
        indicator_mwcmq.color = wallet.getMqsListenerStatus() ? "green" : "red"


        if (mqsStarted) {
            if (mqsStatus) {
                button_mqs.title = mqsInProgress ? "Stopping..." : "Stop"
            } else {
                button_mqs.title= "Stop to retry"
            }
        } else {
            button_mqs.title = mqsInProgress ? "Starting..." : "Start"
        }
        //button_next_address.enabled = !mqsStarted && !torStarted
       //button_to_index.enabled = !mqsStarted && !torStarted



        // TOR
        indicator_tor.color = wallet.getTorListenerStatus() ? "green" : "red"

        if (torStarted) {
            if (torStatus) {
                button_tor.title = torInProgress ? "Stopping..." : "Stop"
            } else {
                button_tor.title = "Stop to retry"
            }
        } else {
            button_tor.title = torInProgress ? "Starting" : "Start"
        }
    }

    function selectReceiveMethod(receiveMethod) {
        selectedReceiveMethod = receiveMethod
        text_mqs_address.focus = false
        text_tor_address.focus = false
        text_slate_address.focus = false
        switch (selectedReceiveMethod) {
            case 1: // online_id
                rec_address.color = "#252525"
                rec_file.color = "#00000000"
                rec_slate.color = "#00000000"

                break;
            case 2: // file_id
                rec_address.color = "#00000000"
                rec_file.color = "#252525"
                rec_slate.color = "#00000000"
                break;
            case 3: // slatepack_id
                rec_address.color = "#00000000"
                rec_file.color = "#00000000"
                rec_slate.color = "#252525"
                 break;
        }

    }

    MouseArea{
        anchors.fill: parent
        onClicked: {
            text_mqs_address.focus = false
            text_tor_address.focus = false
            text_slate_address.focus = false
        }
    }


    Rectangle {
        id: rect_receiveType
        anchors.top: parent.top
        anchors.topMargin: parent.height/14
        anchors.horizontalCenter: parent.horizontalCenter
        radius: dp(25)
        color: "#191919"
        height: parent.height/6
        width: parent.width/1.15

        Rectangle {
            id: rec_address
            height: parent.height - dp(16)
            width: parent.width/3 - dp(16)
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: dp(8)
            color: "#252525"
            radius: dp(25)
            SendType {
                id: address
                height: parent.height/1.7
                width: parent.width/1.7
                img_height: address.height/2
                img_source: "../../img/online.svg"
                mainText: qsTr("Address")
                secondaryText: qsTr("Automatic")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                img_color: "#ffffff"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    selectReceiveMethod(1)
                }
            }
        }

        Rectangle {
            id: rec_file
            height: parent.height - dp(16)
            width: parent.width/3 - dp(16)
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#00000000"
            radius: dp(25)
            SendType {
                id: file
                height: parent.height/1.7
                width: parent.width/1.7
                img_height: file.height/2
                img_source: "../../img/file.svg"
                mainText: qsTr("File")
                secondaryText: qsTr("Manual")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                img_color: "#ffffff"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    selectReceiveMethod(2)
                }
            }
        }

        Rectangle {
            id: rec_slate
            height: parent.height - dp(16)
            width: parent.width/3 - dp(16)
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: dp(8)
            color: "#00000000"
            radius: dp(25)
            SendType {
                id: slate
                height: parent.height/1.7
                width: parent.width/1.7
                img_height: slate.height/2
                img_source: "../../img/slate.svg"
                mainText: qsTr("Slatepack")
                secondaryText: qsTr("Manual")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                img_color: "#ffffff"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    selectReceiveMethod(3)
                }
            }
        }
    }


    /// 1

    Rectangle {
        color: "#00000000"
        visible: selectedReceiveMethod === 1? true : false
        anchors.top: rect_receiveType.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right


        Text {
            id: text_mwcmq
            text: "MWCMQS"
            color: "white"
            font.pixelSize: dp(17)
            font.letterSpacing: dp(0.5)
            anchors.top: parent.top
            anchors.topMargin: dp(40)
            anchors.left: rect_mqs_address.left
            anchors.leftMargin: dp(10)
            visible: selectedReceiveMethod !== 2? true: false
            //anchors.horizontalCenter: parent.horizontalCenter
        }

        Rectangle {
            id: indicator_mwcmq
            color: "white"
            height: text_mwcmq.height/2
            width: height
            radius: dp(50)
            anchors.verticalCenter: text_mwcmq.verticalCenter
            anchors.left: text_mwcmq.right
            anchors.leftMargin: dp(10)
        }
        ImageColor {
            id: img_qr
            img_height: text_mwcmq.height
            img_source: "../../img/qr.svg"
            img_color: "#ffffff"
            anchors.right: img_copy.left
            anchors.rightMargin: dp(10)
            anchors.bottom: rect_mqs_address.top
            anchors.bottomMargin: dp(10)
            //anchors.verticalCenter: parent.verticalCenter
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    receive.showQrCode("MWCMQS", mwcmqAddr)
                }
            }
        }
        ImageColor {
            id: img_copy
            img_height: text_mwcmq.height
            img_source: "../../img/copy.svg"
            img_color: "#ffffff"
            anchors.right: rect_mqs_address.right
            anchors.rightMargin: dp(10)
            anchors.bottom: rect_mqs_address.top
            anchors.bottomMargin: dp(10)
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    clipboard.text = mwcmqAddr
                    notification.text = "MWCMQ Address copied"
                    notification.open()
                }
            }
        }

        Rectangle {
            id: rect_mqs_address
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: text_mwcmq.bottom
            anchors.topMargin: dp(10)
            color: "#252525"
            radius: dp(15)
            height: dp(16) + text_mqs_address.height
            width: rect_receiveType.width

            TextEdit {
                id: text_mqs_address
                readOnly: true
                anchors.top: parent.top
                anchors.topMargin: dp(8)
                anchors.left: parent.left
                anchors.leftMargin: dp(10)
                anchors.right: parent.right
                anchors.rightMargin: dp(10)
                wrapMode: Text.WrapAnywhere
                font.pixelSize: dp(15)
                color: "white"
                text: mwcmqAddr
            }
        }

        SecondaryButton {
            id: button_mqs
            width: rect_mqs_address.width/3
            height: dp(30)
            anchors.right: rect_mqs_address.right
            anchors.topMargin: dp(10)
            anchors.top: rect_mqs_address.bottom
            title: qsTr("Start")

            onClicked: {
                if (mqsInProgress)
                    return;
                mqsInProgress = true

                if (wallet.isMqsListenerStarted()) {
                    button_mqs.title = "Stopping..."
                    wallet.requestStopMqsListener()
                }
                else {
                    button_mqs.title = "Starting..."
                    wallet.requestStartMqsListener()
                }
            }
        }



        Text {
            id: text_tor
            text: "TOR"
            color: "white"
            font.pixelSize: dp(17)
            font.letterSpacing: dp(0.5)
            anchors.top: rect_mqs_address.bottom
            anchors.topMargin: dp(70)
            anchors.left: rect_mqs_address.left
            anchors.leftMargin: dp(10)
            visible: selectedReceiveMethod  !== 2? true: false
            //anchors.horizontalCenter: parent.horizontalCenter
        }

        Rectangle {
            id: indicator_tor
            color: "white"
            height: text_tor.height/2
            width: height
            radius: dp(50)
            anchors.verticalCenter: text_tor.verticalCenter
            anchors.left: text_tor.right
            anchors.leftMargin: dp(10)
        }
        ImageColor {
            id: img_qr_tor
            img_height: text_mwcmq.height
            img_source: "../../img/qr.svg"
            img_color: "#ffffff"
            anchors.right: img_copy_tor.left
            anchors.rightMargin: dp(10)
            anchors.bottom: rect_tor_address.top
            anchors.bottomMargin: dp(10)
            //anchors.verticalCenter: parent.verticalCenter
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    receive.showQrCode("Tor", torAddr)
                }
            }
        }
        ImageColor {
            id: img_copy_tor
            img_height: text_mwcmq.height
            img_source: "../../img/copy.svg"
            img_color: "#ffffff"
            anchors.right: rect_tor_address.right
            anchors.rightMargin: dp(10)
            anchors.bottom: rect_tor_address.top
            anchors.bottomMargin: dp(10)
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    clipboard.text = torAddr
                    notification.text = "TOR Address copied"
                    notification.open()
                }
            }
        }




        Rectangle {
            id: rect_tor_address
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: text_tor.bottom
            anchors.topMargin: dp(10)
            color: "#252525"
            radius: dp(15)
            height: dp(16) + text_tor_address.height
            width: rect_receiveType.width

            TextEdit {
                id: text_tor_address
                readOnly: true
                anchors.top: parent.top
                anchors.topMargin: dp(8)
                anchors.left: parent.left
                anchors.leftMargin: dp(10)
                anchors.right: parent.right
                anchors.rightMargin: dp(10)
                wrapMode: Text.WrapAnywhere
                font.pixelSize: dp(15)
                color: "white"
                text: torAddr
            }
        }

        ImageColor {
            id: test
            img_height: dp(250)
            img_color: "#00000000"
            anchors.fill: parent
            anchors.horizontalCenter: parent.horizontalCenter

        }
        SecondaryButton {
            id: button_tor
            width: rect_tor_address.width/3
            height: dp(30)
            anchors.right: rect_tor_address.right
            anchors.top: rect_tor_address.bottom
            anchors.topMargin: dp(10)
            title: qsTr("Start")

            onClicked: {
                if (torInProgress)
                    return;
                torInProgress = true;

                if (wallet.isTorListenerStarted()) {
                    button_tor.title = "Stopping..."
                    wallet.requestStopTorListener()
                }
                else {
                    button_tor.title = "Starting..."
                    wallet.requestStartTorListener()
                }
            }
        }
    }






    /// 3
    Rectangle {
        color: "#00000000"
        visible: selectedReceiveMethod === 3? true : false
        anchors.top: rect_receiveType.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right


        Text {
            id: text_slate
            text: "SlatePack"
            color: "white"
            font.pixelSize: dp(17)
            font.letterSpacing: dp(0.5)
            anchors.top: parent.top
            anchors.topMargin: dp(40)
            anchors.left: rect_slate_address.left
            anchors.leftMargin: dp(10)
            visible: selectedReceiveMethod  !== 2? true: false
            //anchors.horizontalCenter: parent.horizontalCenter
        }
        ImageColor {
            id: img_qr_slate
            img_height: text_slate.height
            img_source: "../../img/qr.svg"
            img_color: "#ffffff"
            anchors.right: img_copy_slate.left
            anchors.rightMargin: dp(10)
            anchors.bottom: rect_slate_address.top
            anchors.bottomMargin: dp(10)
            //anchors.verticalCenter: parent.verticalCenter
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    receive.showQrCode("Slatepack", proofAddr)
                }
            }
        }
        ImageColor {
            id: img_copy_slate
            img_height: text_mwcmq.height
            img_source: "../../img/copy.svg"
            img_color: "#ffffff"
            anchors.right: rect_slate_address.right
            anchors.rightMargin: dp(10)
            anchors.bottom: rect_slate_address.top
            anchors.bottomMargin: dp(10)
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    clipboard.text = proofAddr
                    notification.text = "Slatepack Address copied"
                    notification.open()
                }
            }
            //anchors.verticalCenter: parent.verticalCenter
        }

        Rectangle {
            id: rect_slate_address
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: text_slate.bottom
            anchors.topMargin: dp(10)
            color: "#252525"
            radius: dp(15)
            height: dp(16) + text_slate_address.height
            width: rect_receiveType.width

            TextEdit {
                id: text_slate_address
                selectByMouse: true
                readOnly: true
                anchors.top: parent.top
                anchors.topMargin: dp(8)
                anchors.left: parent.left
                anchors.leftMargin: dp(10)
                anchors.right: parent.right
                anchors.rightMargin: dp(10)
                wrapMode: Text.WrapAnywhere
                font.pixelSize: dp(15)
                color: "white"
                text: proofAddr
            }
        }

        InputSlatepack {
            id: inputSlate
            anchors.top: rect_slate_address.bottom
            anchors.topMargin: dp(40)
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
        }
    }


    Button {
        id: button_file
        visible: selectedReceiveMethod === 2? true : false
        height: dp(50)
        width: parent.width/2
        anchors.top: rect_receiveType.bottom
        anchors.topMargin: dp(80)
        anchors.horizontalCenter: parent.horizontalCenter
        background: Rectangle {
            color: "#252525"
            radius: dp(25)
            Text {
                text: qsTr("Receive by File")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(17)
                color: "white"
            }
        }
        onClicked: {
            if (qtAndroidService.requestPermissions()) {
                qtAndroidService.openFile( config.getPathFor("fileGen"), "*/*", 123 )
            } else {
                messagebox.open("Failure", "Permission Denied")
            }
        }
    }


}

/*##^##
Designer {
    D{i:0;autoSize:true;height:640;width:320}
}
##^##*/
