import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import UtilBridge 1.0

Item {
    property var callback
    property string blockingPasswordHash
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
        label_password.visible = false
        textfield_password.visible = false
        textfield_password.text = ""
        if (isTwoButtons) {
            text_yesbutton.text = yesBtnText
            text_nobutton.text = noBtnText
            button_ok.visible = false
            button_yes.visible = true
            button_no.visible = true
            callback = _callback
            blockingPasswordHash = ""

            if (passwordHash !== "") {
                messagebox.height = text_message.height + dp(350)
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
            font.pixelSize: dp(20)
            color: "#3600C9"
        }

        TextField {
            id: textfield_password
            visible: false
            height: dp(50)
            padding: dp(10)
            leftPadding: dp(20)
            font.pixelSize: dp(18)
            placeholderText: qsTr("Password")
            echoMode: "Password"
            color: "#3600C9"
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
                color: button_yes.enabled ? "#6F00D6" : "white"
                radius: dp(5)
                border.width: dp(2)
                border.color: button_yes.enabled ? "#6F00D6" : "gray"
                Text {
                    id: text_yesbutton
                    text: qsTr("Yes")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: dp(18)
                    color: button_yes.enabled ? "white" : "gray"
                }
            }

            onClicked: {
                messagebox.visible = false
                callback(true, textfield_password.text)
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
