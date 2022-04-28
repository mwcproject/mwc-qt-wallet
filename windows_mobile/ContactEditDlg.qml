import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import UtilBridge 1.0
import "./models"

Item {
    property var callback
    property var contactList: []

    UtilBridge {
        id: util
    }


    id: dialog
    visible: false

    function open(name, address, contacts, editMode, _callback) {
        text_title.text = editMode ?qsTr("id-edit-contact") :qsTr("id-new-contact")
        textfield_name.text = name
        textfield_address.text = address
        contactList = contacts
        callback = _callback
        dialog.visible = true
    }

    Rectangle {
        anchors.fill: parent
        color: "black"
        opacity: 0.5

        MouseArea {
            anchors.fill: parent
            onClicked: dialog.visible = false
        }
    }


    Rectangle {
        id: rectangle
        color: "#181818"
        radius: dp(25)
        height: text_title.height + textfield_name.height + textfield_address.height + button_apply.height + dp(150)
        anchors.left: parent.left
        anchors.leftMargin: dp(25)
        anchors.right: parent.right
        anchors.rightMargin: dp(25)
        anchors.verticalCenter: parent.verticalCenter

        MouseArea {
            anchors.fill: parent
            onClicked: {
                textfield_name.focus = false
                textfield_address.focus = false
            }
        }

        Text {
            id: text_title
            text: qsTr("id-title")
            font.bold: true
            anchors.top: parent.top
            anchors.topMargin: dp(20)
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(22)
            color: "white"
        }

        TextFieldCursor {
            id: textfield_name
            height: dp(55)
            padding: dp(10)
            leftPadding: dp(20)
            font.pixelSize: dp(18)
            placeholderText: qsTr("id-name")
            color: "white"
            text: ""
            anchors.top: text_title.bottom
            anchors.topMargin: dp(30)
            anchors.right: parent.right
            anchors.rightMargin: dp(40)
            anchors.left: parent.left
            anchors.leftMargin: dp(40)
            horizontalAlignment: Text.AlignLeft

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    textfield_name.focus = true
                }
            }
        }

        TextFieldCursor {
            id: textfield_address
            height: dp(55)
            padding: dp(10)
            leftPadding: dp(20)
            font.pixelSize: dp(18)
            placeholderText: qsTr("id-address")
            color: "white"
            text: ""
            anchors.top: textfield_name.bottom
            anchors.topMargin: dp(30)
            anchors.right: parent.right
            anchors.rightMargin: dp(40)
            anchors.left: parent.left
            anchors.leftMargin: dp(40)
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WrapAnywhere

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    textfield_address.focus = true
                }
            }
        }

        Rectangle {
            height: dp(30)
            width: parent.width/1.2
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#00000000"
            anchors.top: textfield_address.bottom
            anchors.topMargin: dp(30)
            ConfirmButton {
                id: button_apply
                height: dp(30)
                width: parent.width/2 - dp(20)
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                onClicked: {
                    const name = textfield_name.text.trim()
                    let address = textfield_address.text.trim()

                    while (address[address.length - 1] === "/")
                        address = address.slice(0, address.length - 1)

                    if (name === "") {
                        messagebox.open(qsTr("id-need-info"), qsTr("id-specify-name-contact"))
                        return;
                    }

                    if (address === "") {
                        messagebox.open(qsTr("id-need-info"), qsTr("id-specify-address-contact"))
                        return;
                    }

                    if (util.verifyAddress(address) === "unknown") {
                        messagebox.open(qsTr("id-incorrect-input"), qsTr("id-specify-correct-address-contact"))
                        return;
                    }

                    if (contactList.find(contact => contact.name === name)) {
                        messagebox.open(qsTr("id-name-collision"), qsTr("id-contact-with-name") + name + qsTr("id-exist-specify-unique-name"))
                        return;
                    }

                    dialog.visible = false
                    callback(true, name, address)
                }
            }

            SecondaryButton {
                id: button_cancel
                height: dp(30)
                width: parent.width/2 - dp(20)
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                onClicked: {
                    dialog.visible = false
                    callback(false)
                }
            }
        }
    }
}
