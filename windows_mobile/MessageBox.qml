import QtQuick 2.12
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
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
        label_password.visible = false
        textfield_password.visible = false
        textfield_password.text = ""

        // adjust rect_messagebox height
        rect_messagebox.height = text_message.height + dp(200)
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

            if (passwordHash !== "") {
                // adjust rect_messagebox height
                rect_messagebox.height = text_message.height + dp(330)
                text_message.anchors.verticalCenterOffset = dp(-50)

                label_password.visible = true
                textfield_password.visible = true
                if (blockButton === 0) {
                    button_no.enabled = false
                } else {
                    button_yes.enabled = false
                }
                blockingPasswordHash = passwordHash
            }
        } else {
            button_ok.visible = true
            button_yes.visible = false
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
            }

        }

    }

    Rectangle {
        id: rect_messagebox
        radius: dp(25)
        color: "#262933"
        anchors.left: parent.left
        anchors.leftMargin: dp(25)
        anchors.right: parent.right
        anchors.rightMargin: dp(25)
        anchors.verticalCenter: parent.verticalCenter

        MouseArea {
            anchors.fill: parent
            onClicked: {
                messagebox.visible = true
            }

        }

        /*Rectangle {
            id: rectangle
            color: "#ffffff"*/
            /*gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop {
                    position: 0
                    color: "#5b1d5d"
                }

                GradientStop {
                    position: 1
                    color: "#1d1d1f"
                }
            }*/
            /*radius: dp(25)
            anchors.rightMargin: dp(1)
            anchors.leftMargin: dp(1)
            anchors.bottomMargin: dp(1)
            anchors.topMargin: dp(1)
            //border.width: dp(1)
            anchors.fill: parent*/

            Text {
                id: text_title
                text: qsTr("Title")
                font.bold: true
                anchors.top: parent.top
                anchors.topMargin: dp(38)
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(22)
                color: "#ffffff"
            }

            Text {
                id: text_message
                anchors.left: parent.left
                anchors.leftMargin: dp(40)
                anchors.right: parent.right
                anchors.rightMargin: dp(40)
                text: qsTr("Content")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: dp(18)
                color: "#cccccc"
            }

            Text {
                id: label_password
                visible: false
                anchors.left: parent.left
                anchors.leftMargin: dp(40)
                anchors.right: parent.right
                anchors.rightMargin: dp(40)
                anchors.top: text_message.bottom
                anchors.topMargin: dp(30)
                text: qsTr("Input your password to continue:")
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: dp(18)
                color: "#cccccc"
            }

            TextField {
                id: textfield_password
                visible: false
                height: dp(50)
                padding: dp(10)
                leftPadding: dp(20)
                font.pixelSize: textfield_password.text ? dp(10) : dp(18)
                placeholderText: qsTr("Password")
                echoMode: "Password"
                color: "#cccccc"
                text: ""
                anchors.top: label_password.bottom
                anchors.topMargin: dp(10)
                anchors.right: parent.right
                anchors.rightMargin: dp(40)
                anchors.left: parent.left
                anchors.leftMargin: dp(40)
                horizontalAlignment: Text.AlignLeft
                background: Rectangle {
                    border.width: dp(1)
                    border.color: "#8633E0"
                    color: "white"
                    radius: dp(5)
                }

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
                width: dp(135)
                height: dp(50)
                anchors.bottom: parent.bottom
                anchors.bottomMargin: dp(20)
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    messagebox.visible = false
                }
            }

            ConfirmButton {
                id: button_yes
                width: dp(135)
                height: dp(50)
                anchors.bottom: parent.bottom
                anchors.bottomMargin: dp(30)
                anchors.right: parent.right
                anchors.rightMargin: parent.width / 2 - dp(170)
                color: button_yes.enabled ? "#6F00D6" : "white"
                onClicked: {
                    messagebox.visible = false
                    callback(true, textfield_password.text)
                }
            }

            SecondaryButton {
                id: button_no
                width: dp(135)
                height: dp(50)
                anchors.bottom: parent.bottom
                anchors.bottomMargin: dp(30)
                anchors.left: parent.left
                anchors.leftMargin: parent.width / 2 - dp(170)
                onClicked: {
                    messagebox.visible = false
                    callback(false)
                }
            }
    }
}
