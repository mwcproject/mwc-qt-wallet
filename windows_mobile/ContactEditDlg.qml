import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import UtilBridge 1.0

Item {
    property var callback
    property var contactList: []

    UtilBridge {
        id: util
    }

    height: dp(390)
    id: dialog
    visible: false
    anchors.left: parent.left
    anchors.leftMargin: dp(25)
    anchors.right: parent.right
    anchors.rightMargin: dp(25)

    function open(name, address, contacts, editMode, _callback) {
        text_title.text = editMode ? "Edit Contact" : "New Contact"
        textfield_name.text = name
        textfield_address.text = address
        contactList = contacts
        callback = _callback
        dialog.visible = true
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
                textfield_name.focus = false
                textfield_address.focus = false
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

        TextField {
            id: textfield_name
            height: dp(50)
            padding: dp(10)
            leftPadding: dp(20)
            font.pixelSize: dp(18)
            placeholderText: qsTr("Name")
            color: "#3600C9"
            text: ""
            anchors.top: text_title.bottom
            anchors.topMargin: dp(50)
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
                    textfield_name.focus = true
                }
            }
        }

        TextField {
            id: textfield_address
            height: dp(80)
            padding: dp(10)
            leftPadding: dp(20)
            font.pixelSize: dp(18)
            placeholderText: qsTr("Address")
            color: "#3600C9"
            text: ""
            anchors.top: textfield_name.bottom
            anchors.topMargin: dp(30)
            anchors.right: parent.right
            anchors.rightMargin: dp(40)
            anchors.left: parent.left
            anchors.leftMargin: dp(40)
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WrapAnywhere
            background: Rectangle {
                border.width: dp(1)
                border.color: "#8633E0"
                color: "white"
                radius: dp(5)
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    textfield_address.focus = true
                }
            }
        }

        Button {
            id: button_apply
            width: dp(135)
            height: dp(50)
            anchors.bottom: parent.bottom
            anchors.bottomMargin: dp(30)
            anchors.right: parent.right
            anchors.rightMargin: parent.width / 2 - dp(170)

            background: Rectangle {
                color: "#6F00D6"
                radius: dp(5)
                border.width: dp(2)
                border.color: "#6F00D6"
                Text {
                    text: qsTr("Apply")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: dp(18)
                    color: "white"
                }
            }

            onClicked: {
                const name = textfield_name.text.trim()
                let address = textfield_address.text.trim()

                while(address[address.length - 1] === "/")
                    address = address.slice(0, address.length - 1)

                if (name === "")
                {
                    messagebox.open(qsTr("Need Info"), qsTr("Please specify a name for your contact"))
                    return;
                }

                if (address === "")
                {
                    messagebox.open(qsTr("Need Info"), qsTr("Please specify an address for your contact"))
                    return;
                }

                if (util.verifyAddress(address) === "unknown")
                {
                    messagebox.open(qsTr("Incorrect Input"), qsTr( "Please specify correct address for your contact"))
                    return;
                }

                if(contactList.find(contact => contact.name === name)) {
                    messagebox.open(qsTr("Names collision"), qsTr("Contact with a name " + name + " already exist. Please specify unique name for your contact"))
                    return;
                }

                dialog.visible = false
                callback(true, name, address)
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
                dialog.visible = false
                callback(false)
            }
        }
    }
}
