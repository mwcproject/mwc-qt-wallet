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
        text_title.text = editMode ? qsTr("Edit Contact") : qsTr("New Contact")
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
        color: Theme.card
        radius: dp(15)
        height: text_title.height + textfield_name.height + textfield_address.height + button_yes.height + dp(120)
        width: parent.width/1.3
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

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
            font.pixelSize: dp(18)
            color: Theme.textPrimary
        }

        TextFieldCursor {
            id: textfield_name
            height: dp(40)
            width: parent.width/1.3
            padding: dp(10)
            leftPadding: dp(20)
            font.pixelSize: dp(16)
            placeholderText: qsTr("Name")
            color: Theme.textPrimary
            text: ""
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: text_title.bottom
            anchors.topMargin: dp(30)
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
            height: dp(80)
            width: parent.width/1.3
            padding: dp(10)
            leftPadding: dp(20)
            font.pixelSize: dp(16)
            placeholderText: qsTr("Address")
            color: Theme.textPrimary
            text: ""
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: textfield_name.bottom
            anchors.topMargin: dp(20)
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WrapAnywhere
            onTextChanged: {
                if (text == "") {
                    colorbg = Theme.field
                    return
                }
                if (util.verifyAddress(text) === "unknown") {
                    colorbg = Theme.fieldError
                } else {
                    colorbg = Theme.field
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    textfield_address.focus = true
                }
            }
        }


        ConfirmButton {
            id: button_yes
            width: parent.width/3
            height: dp(40)
            anchors.top: textfield_address.bottom
            anchors.topMargin: dp(30)
            anchors.right: parent.right
            anchors.rightMargin: (parent.width / 2 - width)/2
            title: "Create"
            pixelSize: dp(16)
            color: button_yes.enabled ? "#6F00D6" : "white"
            onClicked: {
                const name = textfield_name.text.trim()
                let address = textfield_address.text.trim()

                while (address[address.length - 1] === "/")
                    address = address.slice(0, address.length - 1)

                if (name === "") {
                    messagebox.open(qsTr("Missing Name"), qsTr("Specify an name for your contact"))
                    return;
                }

                if (address === "") {
                    messagebox.open(qsTr("Missing Address"), qsTr("Specify an address for your contact"))
                    return;
                }

                if (util.verifyAddress(address) === "unknown") {
                    messagebox.open(qsTr("Incorrect Address"), qsTr("Specify a correct contact address"))
                    return;
                }

                if (contactList.find(contact => contact.name === name)) {
                    messagebox.open(qsTr("Contact collision"), qsTr("Contact with the name") + " " + name + " " + qsTr("already exist."))
                    return;
                }

                dialog.visible = false
                callback(true, name, address)
            }
        }

        SecondaryButton {
            id: button_cancel
            width: parent.width/3
            height: dp(40)
            title: "Cancel"
            anchors.top: textfield_address.bottom
            anchors.topMargin: dp(30)
            anchors.left: parent.left
            pixelSize: dp(16)
            anchors.leftMargin: (parent.width / 2 - width)/2

            onClicked: {
                dialog.visible = false
                callback(false)
            }
        }
    }
}
