import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import ConfigBridge 1.0

Item {
    function isInteger(value) {
      return /^\d+$/.test(value)
    }

    id: settingsItem
    visible: false

    ConfigBridge {
        id: config
    }

    onVisibleChanged: {
        if (visible) {
            textfield_confirmation.text = Number(config.getInputConfirmationNumber()).toString()
            textfield_outputs.text = Number(config.getChangeOutputs()).toString()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#00000000"

        MouseArea {
            anchors.fill: parent
        }
    }

    Rectangle {
        height: dp(400)
        anchors.left: parent.left
        anchors.leftMargin: dp(25)
        anchors.right: parent.right
        anchors.rightMargin: dp(25)
        anchors.verticalCenter: parent.verticalCenter

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
                    textfield_confirmation.focus = false
                    textfield_outputs.focus = false
                }
            }

            Text {
                id: text_title
                text: qsTr("Send MWC parameters")
                font.bold: true
                anchors.top: parent.top
                anchors.topMargin: dp(31)
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(22)
                color: "#3600c9"
            }

            Text {
                id: label_confirmation
                anchors.left: parent.left
                anchors.leftMargin: dp(40)
                anchors.right: parent.right
                anchors.rightMargin: dp(40)
                anchors.top: text_title.bottom
                anchors.topMargin: dp(30)
                text: qsTr("Number of confirmations")
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: dp(18)
                color: "#3600C9"
            }

            TextField {
                id: textfield_confirmation
                height: dp(50)
                padding: dp(10)
                leftPadding: dp(20)
                font.pixelSize: dp(18)
                color: "#3600C9"
                text: ""
                anchors.top: label_confirmation.bottom
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

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        textfield_confirmation.focus = true
                    }
                }
            }

            Text {
                id: label_outputs
                anchors.left: parent.left
                anchors.leftMargin: dp(40)
                anchors.right: parent.right
                anchors.rightMargin: dp(40)
                anchors.top: textfield_confirmation.bottom
                anchors.topMargin: dp(18)
                text: qsTr("Change outputs")
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: dp(20)
                color: "#3600C9"
            }

            TextField {
                id: textfield_outputs
                height: dp(50)
                padding: dp(10)
                leftPadding: dp(20)
                font.pixelSize: dp(18)
                color: "#3600C9"
                text: ""
                anchors.top: label_outputs.bottom
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

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        textfield_outputs.focus = true
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
                anchors.rightMargin: parent.width / 2 - dp(170)

                background: Rectangle {
                    color: "white"
                    radius: dp(5)
                    border.width: dp(2)
                    border.color: "#3600C9"
                    Text {
                        text: qsTr("OK")
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: dp(18)
                        color: "#3600C9"
                    }
                }

                onClicked: {
                    const inputConfirmationNumber = textfield_confirmation.text.trim()
                    if (!isInteger(inputConfirmationNumber) || parseInt(inputConfirmationNumber) <= 0) {
                        messagebox.open(qsTr("Need info"), qsTr("Please input correct value for minimum number of confirmations"))
                        return
                    }

                    const changeOutputs = textfield_outputs.text.trim()
                    if (!isInteger(changeOutputs) || parseInt(changeOutputs) <= 0) {
                        messagebox.open(qsTr("Need info"), qsTr("Please input correct value for change outputs"))
                        return
                    }

                    config.updateSendCoinsParams(parseInt(inputConfirmationNumber), parseInt(changeOutputs))
                    settingsItem.visible = false
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
                    settingsItem.visible = false
                }
            }
        }
    }
}
