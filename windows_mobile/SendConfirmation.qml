import QtQuick 2.15
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import ConfigBridge 1.0
import UtilBridge 1.0
import "./models"


Item {
    property var callback
    property string blockingPasswordHash
    anchors.fill: parent

    property string address
    property double amountSpend
    property double txFees

    //height: text_message.height + dp(420)
    id: sendConfirmation
    visible: false
    //anchors.fill:parent
    /*anchors.left: parent.left
    anchors.leftMargin: dp(25)
    anchors.right: parent.right
    anchors.rightMargin: dp(25)*/

    function open(title, info, passwordHash, _callback) {
        text_title.text = title
        console.log(info)
        const txInfo = JSON.parse(info)
        address = txInfo[1]
        amountSpend = txInfo[2]
        txFees = txInfo[3]
        console.log(address, " ", amountSpend, " ", txFees, " ", )
        blockingPasswordHash = passwordHash
        callback = _callback
        sendConfirmation.visible = true
    }

    UtilBridge {
        id: util
    }

    ConfigBridge {
        id: config
    }

    onVisibleChanged: {
        if (visible) {
            checkbox_fluff.checked = config.isFluffSet()
            button_confirm.enabled = false
            textfield_password.text = ""
        }
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop {
                position: 0
                color: Theme.gradientTop
            }

            GradientStop {
                position: 0.3
                color: Theme.gradientBottom
            }
        }
    }

    Rectangle {
        id: rectangle
        color: "#00000000"
        anchors.fill: parent

        MouseArea {
            anchors.fill: parent
            onClicked: {
                textfield_password.focus = false
            }
        }

        Text {
            id: text_title
            text: qsTr("Title")
            font.bold: true
            anchors.top: parent.top
            anchors.topMargin: dp(31)
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(22)
            color: "white"
        }

        Text {
            id: text_address
            anchors.left: parent.left
            anchors.leftMargin: dp(40)
            anchors.right: parent.right
            anchors.rightMargin: dp(40)
            anchors.top: text_title.bottom
            anchors.topMargin: dp(40)
            text: qsTr("Content")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            font.pixelSize: dp(18)
            color: "#3600C9"
        }

        Text {
            id: text_message
            anchors.left: parent.left
            anchors.leftMargin: dp(40)
            anchors.right: parent.right
            anchors.rightMargin: dp(40)
            anchors.top: text_title.bottom
            anchors.topMargin: dp(40)
            text: qsTr("Content")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            font.pixelSize: dp(18)
            color: "#3600C9"
        }

        CheckBox {
            id: checkbox_fluff
            text: qsTr("Fluff the transaction")
            font.pixelSize: dp(18)
            anchors.left: parent.left
            anchors.leftMargin: dp(40)
            anchors.right: parent.right
            anchors.rightMargin: dp(40)
            anchors.top: text_message.bottom
            anchors.topMargin: dp(40)

            indicator: Rectangle {
                implicitWidth: dp(20)
                border.width: dp(1)
                implicitHeight: dp(20)
                x: checkbox_fluff.leftPadding
                y: parent.height / 2 - height / 2

                Image {
                    width: dp(20)
                    height: dp(20)
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                    source: checkbox_fluff.checked ? "../img/purpleCheckOn@2x.svg" : "../img/purpleCheckOff@2x.svg"
                }
            }

            contentItem: Text {
                text: checkbox_fluff.text
                font: checkbox_fluff.font
                color: "#3600C9"
                verticalAlignment: Text.AlignVCenter
                anchors.left: checkbox_fluff.indicator.right
                anchors.leftMargin: dp(5)
            }

        }

        Text {
            id: checkbox_note
            anchors.left: parent.left
            anchors.leftMargin: dp(65)
            anchors.right: parent.right
            anchors.rightMargin: dp(40)
            anchors.top: checkbox_fluff.bottom
            anchors.topMargin: dp(5)
            text: qsTr("Note: Ignores Dandelion relay protocol and reduces anonymity of transactions")
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            font.pixelSize: dp(15)
            color: "#3600C9"
        }

        Text {
            id: label_password
            anchors.left: parent.left
            anchors.leftMargin: dp(40)
            anchors.right: parent.right
            anchors.rightMargin: dp(40)
            anchors.top: checkbox_note.bottom
            anchors.topMargin: dp(20)
            text: qsTr("Input your password to continue:")
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            font.pixelSize: dp(18)
            color: "#3600C9"
        }

        TextField {
            id: textfield_password
            height: dp(50)
            padding: dp(10)
            leftPadding: dp(20)
            font.pixelSize: textfield_password.text ? dp(10) : dp(18)
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
                button_confirm.enabled = ok
                if (ok)
                    button_confirm.focus = true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    textfield_password.focus = true
                }
            }
        }

        ConfirmButton {
            id: button_confirm
            title: "Confirm"
            width: dp(135)
            height: dp(50)
            anchors.bottom: parent.bottom
            anchors.bottomMargin: dp(30)
            anchors.right: parent.right
            anchors.rightMargin: parent.width / 2 - dp(170)
            onClicked: {
                config.setFluff(checkbox_fluff.checked)
                sendConfirmation.visible = false
                callback(true)
            }
        }

        SecondaryButton {
            id: button_decline
            title: "Cancel"
            width: dp(135)
            height: dp(50)
            anchors.bottom: parent.bottom
            anchors.bottomMargin: dp(30)
            anchors.left: parent.left
            anchors.leftMargin: parent.width / 2 - dp(170)
            onClicked: {
                sendConfirmation.visible = false
                callback(false)
            }
        }
    }

}
