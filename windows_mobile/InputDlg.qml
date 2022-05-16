import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import "./models"

Item {
    property var callback

    id: messagebox
    visible: false

    function open(title, message, placeholder, text, inputMaxLength, _callback) {
        text_title.text = title
        text_message.text = message
        textfield_input.placeholderText = placeholder
        textfield_input.text = text
        textfield_input.maximumLength = inputMaxLength
        callback = _callback
        messagebox.visible = true
    }

    Rectangle {
        anchors.fill: parent
        color: "#000000"
        opacity: 0.5

        MouseArea {
            anchors.fill: parent
            onClicked: {
                messagebox.visible = false
                textfield_input.focus = false
            }

        }

    }

    Rectangle {
        id: rect_messagebox
        radius: dp(15)
        height: button_yes.height + textfield_input.height + label_password.height + text_message.height + text_title.height + dp(100)
        color: "#262933"
        width: parent.width/1.3
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        MouseArea {
            anchors.fill: parent
            onClicked: {
                messagebox.visible = true
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
                color: Theme.textPrimary
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
                color: Theme.textPrimary
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
                color: Theme.textPrimary
            }

            TextFieldCursor {
                id: textfield_input
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

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        textfield_input.focus = true
                    }
                }
            }

            ConfirmButton {
                id: button_yes
                width: parent.width/3
                height: dp(40)
                title: "Confirm"
                anchors.top: textfield_input.bottom
                anchors.topMargin: dp(25)
                anchors.right: parent.right
                anchors.rightMargin: (parent.width / 2 - width)/2
                color: button_yes.enabled ? "#6F00D6" : "white"
                pixelSize: dp(16)
                onClicked: {
                    messagebox.visible = false
                    callback(true, textfield_input.text)
                }
            }

            SecondaryButton {
                id: button_no
                width: parent.width/3
                height: dp(40)
                pixelSize: dp(16)
                title: "Cancel"
                anchors.top: textfield_input.bottom
                anchors.topMargin: dp(25)
                anchors.left: parent.left
                anchors.leftMargin: (parent.width / 2 - width)/2
                onClicked: {
                    messagebox.visible = false
                    callback(false)
                }
            }
    }


}
