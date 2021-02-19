import QtQuick 2.0
import QtQuick.Controls 2.13
import StateMachineBridge 1.0
import ConfigBridge 1.0
import UtilBridge 1.0
import WalletBridge 1.0
import Clipboard 1.0

Item {
    property string initTag: "InitialSP"
    property string finalizeTag: "FinalizeSP"
    property string slatepack
    property int backStateId
    property string txExtension
    property bool enableFinalize
    property string spInProgress
    property string slate2finalize
    property string transactionUUID
    property int lastReportedError: 0

    function init(params) {
        slatepack = params.slatepack
        backStateId = params.backStateId
        txExtension = params.txExtension
        enableFinalize = params.enableFinalize
        textarea_content.text = slatepack
        textarea_finalize.text = ''

        if(!enableFinalize) {
            flickable_finalize.visible = false
            button_finalize.visible = false
        } else {
            flickable_finalize.visible = true
            button_finalize.visible = true
            button_finalize.enabled = false

            // Requesting UUID for this transaction
            initiateSlateVerification(slatepack, initTag)
        }
        flickable_content.contentY = 0
        flickable_finalize.contentY = 0
    }

    StateMachineBridge {
        id: stateMachine
    }

    ConfigBridge {
        id: config
    }

    UtilBridge {
        id: util
    }

    WalletBridge {
        id: wallet
    }

    Clipboard {
        id: clipboard
    }

    Connections {
        target: wallet

        onSgnDecodeSlatepack: (tag, error, slatepack, slateJson, content, sender, recipient) => {
            spInProgress = ""
            button_finalize.enabled = false

            if (error !== "") {
                // we don't report error..
                return
            }

            if (tag === finalizeTag) {
                if (content !== "SendResponse") {
                    if (lastReportedError !== 2) {
                        lastReportedError = 2
                        messagebox.open(qsTr("Wrong Slatepack"), qsTr("Here you can finalize only the Slatepack from this transaction only. This slatepack doesn't have Response data."))
                        textarea_finalize.focus = true
                    }
                    return
                }
            }

            if (slateJson === "") return

            let txType = 1    //util::FileTransactionType::RECEIVE
            if (tag === finalizeTag) {
                txType = 2      //util::FileTransactionType::FINALIZE
            }

            // Normal case:
            // res[0] = transactionId
            // res[1] = amount
            const parseResult = util.parseSlateContent(slateJson, txType, "");
            if (parseResult.length === 1) {
                if (lastReportedError !== 3) {
                    lastReportedError = 3;
                    messagebox.open(qsTr("Wrong Slatepack"), qsTr("Unable to parse the decoded Slate.\n" + parseResult[0]))
                    textarea_finalize.focus = true
                }
                return  // Some parsing error. We don't printing the error message
            }
            // Q_ASSERT(parseResult.size() > 1);

            // From init slate we are storing transaction UUID
            if (tag === initTag) {
                transactionUUID = parseResult[0];
                return
            }

            if (parseResult[0] !== transactionUUID) {
                // It is wrong transaction, let's report it.
                if (lastReportedError !== 1) {
                    lastReportedError = 1;
                    messagebox.open(qsTr("Wrong Slatepack"), qsTr("Here you can finalize only the Slatepack from this transaction only. This slatepack from different transaction"))
                    textarea_finalize.focus = true
                }
                return
            }

            // we are good here, can finalize...
            slate2finalize = slatepack
            button_finalize.enabled = true

            const textSp = textarea_finalize.text.replace("\n", "").trim()

            if (slatepack !== textSp) {
                initiateSlateVerification(textSp, tag);
            }
        }

        onSgnFinalizeSlatepack: (tagId, error, txUuid) => {
            if (tagId !== "ResultedSlatepack") return

            if (error !== "") {
                messagebox.open(qsTr("Finalization error"), qsTr("Unable to finalize the transaction.\n" + error))
                return
            }

            // Q_ASSERT(txUuid == transactionUUID);
            messagebox.open(qsTr("Finalize Transaction"), qsTr("Transaction " + txUuid + " was finalized successfully."))
            // pressing back button. We are done here
            stateMachine.setActionWindow(backStateId)
        }
    }

    function initiateSlateVerification(slate2check, tag) {
        if (slate2check === "") return
        spInProgress = slate2check
        wallet.decodeSlatepack(slate2check, tag)
    }

    onVisibleChanged: {
        if (visible) {
            if (parent.height > dp(560)) {
                anchors.topMargin = (parent.height - dp(560)) / 2
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            textarea_content.focus = false
            textarea_finalize.focus = false
        }
    }

    Flickable {
        id: flickable_content
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        height: flickable_finalize.visible ? dp(200) : dp(400)
        clip: true

        TextArea.flickable: TextArea {
            id: textarea_content
            padding: dp(10)
            font.pixelSize: dp(18)
            color: "white"
            text: ""
            horizontalAlignment: Text.AlignLeft
            wrapMode: TextArea.WrapAnywhere
            background: Rectangle {
                color: "#8633E0"
                radius: dp(5)
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    textarea_content.focus = true
                }
            }
        }

        ScrollBar.vertical: ScrollBar {}
    }

    Button {
        id: button_copy
        width: parent.width / 2 - dp(45)
        height: dp(50)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.top: flickable_content.bottom
        anchors.topMargin: dp(20)
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Copy to Clipboard")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
            clipboard.text = slatepack
            notification.text = "Slatepack is copied into the clipboard"
            notification.open()
        }
    }

    Button {
        id: button_save
        width: parent.width / 2 - dp(45)
        height: dp(50)
        anchors.verticalCenter: button_copy.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Save Slatepack")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
            const fileName = util.getSaveFileName(qsTr("Save Slatepack"), qsTr("ResultedSlatepack"), qsTr("Slatepack tramsaction (*" + txExtension + ")"), txExtension)
            if (fileName === "") return
            if (util.writeTextFile(fileName, {slatepack})) {
                messagebox.open("Success", "Slatepack file saved at " + fileName)
            }
        }
    }

    Flickable {
        id: flickable_finalize
        anchors.top: button_save.bottom
        anchors.topMargin: dp(30)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        height: dp(200)
        clip: true

        TextArea.flickable: TextArea {
            id: textarea_finalize
            padding: dp(10)
            font.pixelSize: dp(18)
            color: "white"
            text: ""
            placeholderText: "Paste reply slatepack here to finalize transaction. Also you can always finalize it at Finalize page."
            horizontalAlignment: Text.AlignLeft
            wrapMode: TextArea.WrapAnywhere
            background: Rectangle {
                color: "#8633E0"
                radius: dp(5)
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    textarea_finalize.focus = true
                }
            }
            onTextChanged: {
                if (spInProgress === "") {
                    const sp = textarea_finalize.text.replace("\n", "").trim()
                    initiateSlateVerification(sp, finalizeTag)
                }
            }
        }

        ScrollBar.vertical: ScrollBar {}
    }

    Button {
        id: button_back
        width: parent.width / 2 - dp(45)
        height: dp(50)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: button_finalize.visible ? -button_back.width/2 - dp(15) : 0
        anchors.top: flickable_finalize.visible ? flickable_finalize.bottom : button_save.bottom
        anchors.topMargin: dp(20)
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
            initParams = ""
            stateMachine.setActionWindow(backStateId)
        }
    }

    Button {
        id: button_finalize
        width: parent.width / 2 - dp(45)
        height: dp(50)
        anchors.verticalCenter: button_back.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: button_finalize.enabled ? "white" : "grey"
            border.width: dp(2)
            Text {
                text: qsTr("Finalize")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: button_finalize.enabled ? "white" : "grey"
            }
        }

        onClicked: {
            if (slate2finalize === "") return

            // disabling to prevent double clicking...
            button_finalize.enabled = false

            wallet.finalizeSlatepack(slate2finalize, false, "ResultedSlatepack")
        }
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: -notification.width / 2
        anchors.bottom: parent.bottom
        anchors.bottomMargin: dp(20)
        Notification {
            id: notification
        }
    }
}
