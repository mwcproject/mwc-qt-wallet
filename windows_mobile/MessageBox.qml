import QtQuick 2.12
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import QtGraphicalEffects 1.15
import UtilBridge 1.0
import "./models"

Item {
    property var callback
    property string blockingPasswordHash

    id: messagebox
    visible: false

    function open(title, message, isTwoButtons, noBtnText, yesBtnText, passwordHash, blockButton, _ttl_blocks, _callback) {
        text_title.text = title
        text_message.text = message
        //label_password.visible = false
        textfield_password.visible = false
        textfield_password.text = ""

        // adjust rect_messagebox height
        //rect_messagebox.height = text_message.height + dp(200)
        text_message.anchors.verticalCenterOffset = 0

        if (isTwoButtons) {
            button_yes.title = yesBtnText
            button_no.title = noBtnText
            button_ok.visible = false
            button_yes.visible = true
            button_yes.enabled = true
            button_no.visible = true
            button_no.enabled = true
            callback = _callback
            blockingPasswordHash = ""
            overlay.visible = false
            if (passwordHash !== "") {
                // adjust rect_messagebox height
                //rect_messagebox.height = text_message.height + dp(330)
                //text_message.anchors.verticalCenterOffset = dp(-50)

                //label_password.visible = true
                textfield_password.visible = true
                if (blockButton === 0) {
                    button_no.enabled = false
                } else {
                    button_yes.enabled = false
                }
                overlay.visible = true
                blockingPasswordHash = passwordHash
            }
        } else {
            button_ok.visible = true
            button_yes.visible = false
            overlay.visible = false
            button_no.visible = false
        }
        messagebox.visible = true
    }

    UtilBridge {
        id: util
    }

    Rectangle {
        anchors.fill: parent
        color: "#000000"
        opacity: 0.5

        MouseArea {
            anchors.fill: parent
            onClicked: {
                messagebox.visible = false
                textfield_password.focus = false
            }

        }

    }

    Rectangle {
        id: rect_messagebox
        radius: dp(15)
        height: button_yes.height + textfield_password.height + label_password.height + text_message.height + text_title.height + (textfield_password.visible? dp(110) : dp(50))
        color: "#262933"
        width: parent.width/1.3
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        MouseArea {
            anchors.fill: parent
            onClicked: {
                messagebox.visible = true
                textfield_password.focus = false
            }

        }

            Text {
                id: text_title
                text: qsTr("Title")
                font.bold: true
                anchors.top: parent.top
                anchors.topMargin: dp(30)
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "#ffffff"
            }

            Text {
                id: text_message
                text: qsTr("Content")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                anchors.left: parent.left
                anchors.leftMargin: dp(10)
                anchors.right: parent.right
                anchors.rightMargin: dp(10)
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                anchors.top: text_title.bottom
                anchors.topMargin: dp(25)
                font.pixelSize: dp(16)
                color: "#cccccc"
            }

            Text {
                id: label_password
                visible: false

                anchors.top: text_message.bottom
                anchors.topMargin: dp(10)
                text: qsTr("Input your password to continue:")
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: dp(16)
                color: "#cccccc"
            }

            TextFieldCursor {
                id: textfield_password
                height: dp(40)
                width: parent.width/1.3
                padding: dp(10)
                leftPadding: dp(20)
                font.pixelSize: dp(16)
                placeholderText: qsTr("Password")
                color: "white"
                text: ""
                anchors.top: label_password.visible? label_password.bottom : text_message.bottom
                anchors.topMargin: dp(15)
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignLeft
                onTextChanged: {
                                    const ok = util.calcHSA256Hash(textfield_password.text) === blockingPasswordHash
                                    button_yes.enabled = ok
                                    if (ok)
                                        button_yes.focus = true
                                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        textfield_password.focus = true
                    }
                }
            }

            SecondaryButton {
                id: button_ok
                title: qsTr("Ok")
                width: parent.width/3
                height: dp(40)
                anchors.top: textfield_password.visible? textfield_password.bottom : text_message.bottom
                anchors.topMargin: dp(20)
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    messagebox.visible = false
                }
            }



            ConfirmButton {
                id: button_yes
                width: parent.width/3
                height: dp(40)
                anchors.top: textfield_password.visible? textfield_password.bottom : text_message.bottom
                anchors.topMargin: dp(20)
                anchors.right: parent.right
                anchors.rightMargin: (parent.width / 2 - width)/2
                onClicked: {
                    messagebox.visible = false
                    callback(true, textfield_password.text)
                }
            }

            ColorOverlay {
                id: overlay
                anchors.fill: button_yes
                color: "black"
                source: button_yes
                opacity: !button_yes.enabled ? 0.5 : 0
            }

            SecondaryButton {
                id: button_no
                width: parent.width/3
                height: dp(40)
                anchors.top: textfield_password.visible? textfield_password.bottom : text_message.bottom
                anchors.topMargin: dp(20)
                anchors.left: parent.left
                anchors.leftMargin: (parent.width / 2 - width)/2

                onClicked: {
                    messagebox.visible = false
                    callback(false)
                }
            }
    }
}
