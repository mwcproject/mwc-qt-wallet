import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import UtilBridge 1.0
import WalletBridge 1.0
import "./models"

Item {
    property var callback
    property string spInProgress
    property bool isSpValid
    property string expectedContent
    property string expectedContentDescription
    property int txType
    property string pSlatepack
    property string pSlateJson
    property string pSender
    property alias content: textarea_content
    property alias width_content: rect_content

    id: dlgItem
    //visible: false

    onVisibleChanged: {
        if(!visible) {
            textarea_content.focus= false
        }
    }

    function open(_expectedContent, _expectedContentDescription, _txType, _callback) {
        callback = _callback
        expectedContent = _expectedContent
        expectedContentDescription = _expectedContentDescription
        txType = _txType
        text_status.visible = false
        textarea_content.text = ""

        updateButtons()
        dlgItem.visible = true
    }

    function slatepackCallback(ok, slatepack, slateJson, sender) {
            if (ok) {
                receive.signSlatepackTransaction(slatepack, slateJson, sender)
            }
        }

    UtilBridge {
        id: util
    }

    WalletBridge {
        id: wallet
    }

    Connections {
        target: wallet
        onSgnDecodeSlatepack: (tag, error, slatepack, slateJson, content, sender, recipient) => {
            if (tag !== "InputSlatepackDlg") return

            isSpValid = false
            spInProgress = ""
            text_status.visible = false

            if (error !== "") {
                text_status.visible = true
                text_status.text = error
            } else {
                if (expectedContent !== content) {
                    text_status.visible = true
                    text_status.text = "Wrong slatepack content, expected " + expectedContentDescription
                }
                else {
                    // Validating Json
                    const slateParseRes = util.parseSlateContent(slateJson, txType, sender)
                    // Q_ASSERT(slateParseRes.size() == 1 || slateParseRes.size() >= 2);
                    if (slateParseRes.length === 1) {
                        // parser reported error
                        text_status.visible = true
                        text_status.text = slateParseRes[0]
                    }
                    else {
                        let spDesk = ""
                        let senderStr = ""
                        if (sender === "None") {
                            spDesk = "non encrypted Slatepack"
                        } else {
                            spDesk = "encrypted Slatepack"
                            if (txType === 1) // util::FileTransactionType::RECEIVE
                                senderStr = " from " + sender
                            else if (txType === 2) // util::FileTransactionType::FINALIZE
                                senderStr = ", receiver address " + sender
                            else {
                                // Q_ASSERT(false)
                            }
                        }

                        // mwc is on nano units
                        const mwcStr = util.nano2one(slateParseRes[1])

                        if (txType === 1) { // util::FileTransactionType::RECEIVE
                            text_status.text = "You receive " + spDesk + " for " + mwcStr + " MWC" + senderStr
                        }
                        else if (txType === 2) { // util::FileTransactionType::FINALIZE
                            text_status.text = "Finalizing " + spDesk + ", transaction " + slateParseRes[0] + senderStr
                        }
                        isSpValid = true
                        pSlatepack = slatepack;
                        pSlateJson = slateJson;
                        pSender = sender
                        text_status.visible = true
                    }
                }
            }

            updateButtons()
        }
    }

    function initiateSlateVerification(slate2check) {
        if (slate2check === "") {
            text_status.visible = false
            return
        }
        spInProgress = slate2check
        wallet.decodeSlatepack(slate2check, "InputSlatepackDlg")
    }

    function updateButtons() {
        //button_continue.enabled = isSpValid
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            textarea_content.focus = false
        }
    }
    Rectangle {
        id: rect_content
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        height: dp(200)
        color: "#252525"
        radius: dp(15)
        //border.color: "#3600c9"



        Flickable {
            id: flickable_content
            anchors.fill: parent
            clip: true

            TextArea.flickable: TextArea {
                id: textarea_content
                padding: dp(10)
                font.pixelSize: dp(18)
                color: "gray"
                text: ""
                placeholderText: qsTr("Please paste the slatepack content below")
                horizontalAlignment: Text.AlignLeft
                wrapMode: TextArea.WrapAnywhere
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        textarea_content.focus = true
                    }
                }
                onTextChanged: {
                    console.log("inputSlatePack Field")
                    isSpValid = false
                    updateButtons()

                    if (spInProgress === "") {
                        const sp = textarea_content.text.trim()
                        initiateSlateVerification(sp)
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {}
        }
    }

    Text {
        id: text_status
        text: ""
        font.pixelSize: dp(14)
        color: "#3600c9"
        anchors.top: rect_content.bottom
        anchors.topMargin: dp(10)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        wrapMode: Text.WrapAnywhere
    }

    ConfirmButton {
        id: button_continue
        title: "Next"
        anchors.top: rect_content.bottom
        anchors.topMargin: dp(40)
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: {
            open("SendInitial", "Initial Send Slate", 1, slatepackCallback)
            callback(true, pSlatepack, pSlateJson, pSender)
        }
    }
}
