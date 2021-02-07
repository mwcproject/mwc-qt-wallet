import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import UtilBridge 1.0
import WalletBridge 1.0

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

    id: dlgItem
    visible: false
    height: text_status.visible? dp(410) + text_status.height : dp(410)
    anchors.left: parent.left
    anchors.leftMargin: dp(25)
    anchors.right: parent.right
    anchors.rightMargin: dp(25)

    function open(_expectedContent, _expectedContentDescription, _txType, _callback) {
        callback = _callback
        expectedContent = _expectedContent
        expectedContentDescription = _expectedContentDescription
        txType = _txType
        text_status.visible = false

        updateButtons()
        dlgItem.visible = true
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
        button_continue.enabled = isSpValid
    }

    Rectangle {
        id: rectangle
        color: "#ffffff"
        anchors.rightMargin: dp(1)
        anchors.leftMargin: dp(1)
        anchors.bottomMargin: dp(1)
        anchors.topMargin: dp(1)
        border.width: dp(1)
        anchors.fill: parent

        MouseArea {
            anchors.fill: parent
            onClicked: {
                textarea_content.focus = false
            }
        }

        Image {
            id: image_close
            source: "../img/MessageBox_Close@2x.svg"
            width: dp(38)
            height: dp(38)
            anchors.top: parent.top
            anchors.topMargin: dp(25)
            anchors.right: parent.right
            anchors.rightMargin: dp(25)

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    dlgItem.visible = false
                }
            }
        }

        Text {
            id: text_title
            text: qsTr("Slatepack")
            font.bold: true
            anchors.verticalCenter: image_close.verticalCenter
            anchors.verticalCenterOffset: dp(5)
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(22)
            color: "#3600c9"
        }

        Rectangle {
            id: rect_content
            anchors.top: text_title.bottom
            anchors.topMargin: dp(30)
            anchors.left: parent.left
            anchors.leftMargin: dp(30)
            anchors.right: parent.right
            anchors.rightMargin: dp(30)
            height: dp(200)

                color: "white"
                radius: dp(5)
                border.color: "#3600c9"
                border.width: dp(2)

            Flickable {
                id: flickable_content
                anchors.fill: parent
                clip: true

                TextArea.flickable: TextArea {
                    id: textarea_content
                    padding: dp(10)
                    font.pixelSize: dp(18)
                    color: "#3600c9"
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

        Button {
            id: button_continue
            width: dp(135)
            height: dp(50)
            anchors.bottom: parent.bottom
            anchors.bottomMargin: dp(30)
            anchors.right: parent.right
            anchors.rightMargin: parent.width / 2 - dp(170)

            background: Rectangle {
                color: button_continue.enabled ? "#6F00D6" : "white"
                radius: dp(5)
                border.width: dp(2)
                border.color: button_continue.enabled ? "#6F00D6" : "gray"
                Text {
                    text: qsTr("Continue")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: dp(18)
                    color: button_continue.enabled ? "white" : "gray"
                }
            }

            onClicked: {
                dlgItem.visible = false
                callback(true, pSlatepack, pSlateJson, pSender)
            }
        }

        Button {
            id: button_cancel
            width: dp(135)
            height: dp(50)
            anchors.bottom: parent.bottom
            anchors.bottomMargin: dp(30)
            anchors.left: parent.left
            anchors.leftMargin: parent.width / 2 - dp(170)

            background: Rectangle {
                color: "#ffffff"
                radius: dp(5)
                border.width: dp(2)
                border.color: "#3600C9"
                Text {
                    text: qsTr("Cancel")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: dp(18)
                    color: "#3600C9"
                }
            }

            onClicked: {
                dlgItem.visible = false
                callback(false)
            }
        }
    }

}
