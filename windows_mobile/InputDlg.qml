import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0

Item {
    property var callback

    height: text_message.height + dp(300)
    id: messagebox
    visible: false
    anchors.left: parent.left
    anchors.leftMargin: dp(25)
    anchors.right: parent.right
    anchors.rightMargin: dp(25)

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
                textfield_input.focus = false
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
                    messagebox.visible = false
                }
            }
        }

        Text {
            id: text_title
            text: qsTr("Title")
            font.bold: true
            anchors.top: parent.top
            anchors.topMargin: dp(30)
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(24)
            color: "#3600c9"
        }

        Text {
            id: text_message
            anchors.left: parent.left
            anchors.leftMargin: dp(40)
            anchors.right: parent.right
            anchors.rightMargin: dp(40)
            anchors.top: text_title.bottom
            anchors.topMargin: dp(50)
            text: qsTr("Content")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            font.pixelSize: dp(18)
            color: "#3600C9"
        }

        TextField {
            id: textfield_input
            height: dp(50)
            padding: dp(10)
            leftPadding: dp(20)
            font.pixelSize: dp(18)
            placeholderText: qsTr("Password")
            color: "#3600C9"
            text: ""
            anchors.top: text_message.bottom
            anchors.topMargin: dp(25)
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

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    textfield_input.focus = true
                }
            }
        }

        Button {
            id: button_ok
            width: dp(135)
            height: dp(50)
            anchors.bottom: parent.bottom
            anchors.bottomMargin: dp(30)
            anchors.right: parent.right
            anchors.rightMargin: parent.width / 2 - dp(200)

            background: Rectangle {
                color: button_ok.enabled ? "#6F00D6" : "white"
                radius: dp(5)
                border.width: dp(2)
                border.color: button_ok.enabled ? "#6F00D6" : "gray"
                Text {
                    text: qsTr("OK")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: dp(18)
                    color: button_ok.enabled ? "white" : "gray"
                }
            }

            onClicked: {
                messagebox.visible = false
                callback(true, textfield_input.text)
            }
        }

        Button {
            id: button_cancel
            width: dp(135)
            height: dp(50)
            anchors.bottom: parent.bottom
            anchors.bottomMargin: dp(30)
            anchors.left: parent.left
            anchors.leftMargin: parent.width / 2 - dp(200)

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
                messagebox.visible = false
                callback(false)
            }
        }
    }

}
