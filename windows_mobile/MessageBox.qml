import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0

Item {
    property var callback
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    height: text_message.height + dp(250)
    id: messagebox
    visible: false
    anchors.left: parent.left
    anchors.leftMargin: dp(25)
    anchors.right: parent.right
    anchors.rightMargin: dp(25)

    function open(title, message, isTwoButtons, noBtnText, yesBtnText, passwordHash, blockButton, _ttl_blocks, _callback) {
        text_title.text = title
        text_message.text = message
        if (isTwoButtons) {
            text_yesbutton.text = yesBtnText
            text_nobutton.text = noBtnText
            button_ok.visible = false
            button_yes.visible = true
            button_no.visible = true
            callback = _callback
        } else {
            button_ok.visible = true
            button_yes.visible = false
            button_no.visible = false
        }
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
            anchors.bottom: text_message.top
            anchors.bottomMargin: dp(30)
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(20)
            color: "#3600c9"
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
            font.pixelSize: dp(21)
            color: "#3600C9"
        }

        Button {
            id: button_ok
            width: dp(135)
            height: dp(50)
            anchors.bottom: parent.bottom
            anchors.bottomMargin: dp(30)
            anchors.horizontalCenter: parent.horizontalCenter

            background: Rectangle {
                color: "#6F00D6"
                radius: dp(5)
                Text {
                    text: qsTr("OK")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: dp(18)
                    color: "white"
                }
            }

            onClicked: {
                messagebox.visible = false
            }
        }

        Button {
            id: button_yes
            width: dp(135)
            height: dp(50)
            anchors.bottom: parent.bottom
            anchors.bottomMargin: dp(30)
            anchors.right: parent.right
            anchors.rightMargin: parent.width / 2 - dp(150)

            background: Rectangle {
                color: "#6F00D6"
                radius: dp(5)
                Text {
                    id: text_yesbutton
                    text: qsTr("Yes")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: dp(18)
                    color: "white"
                }
            }

            onClicked: {
                messagebox.visible = false
                callback(true)
            }
        }

        Button {
            id: button_no
            width: dp(135)
            height: dp(50)
            anchors.bottom: parent.bottom
            anchors.bottomMargin: dp(30)
            anchors.left: parent.left
            anchors.leftMargin: parent.width / 2 - dp(150)

            background: Rectangle {
                color: "#ffffff"
                radius: dp(5)
                border.width: dp(2)
                border.color: "#3600C9"
                Text {
                    id: text_nobutton
                    text: qsTr("No")
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
