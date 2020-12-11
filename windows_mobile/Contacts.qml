import QtQuick 2.12
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import ConfigBridge 1.0

Item {
    property var contactList: []
    property string prevCBName
    property string prevCBAddress
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    ConfigBridge {
        id: config
    }

    function updateContactsList() {
        const pairs = config.getContactsAsPairs()
        contactsModel.clear()
        for (let i = 0; i < pairs.length; i += 2) {
            contactList.push({
                 name: pairs[i],
                 address: pairs[i+1]
            })
            contactsModel.append(contactList[contactList.length - 1])
        }
    }

    function onAddContact(ok, name, address) {
        if (ok) {
            const res = config.addContact(name, address)
            if (res !== "") {
                messagebox.open(qsTr("Error"), qsTr("Unable to add a new contact.\n" + res))
            }
            updateContactsList()
        }
    }

    function onEditContact(ok, name, address) {
        if (ok) {
            const res = config.updateContact(prevCBName, prevCBAddress, name, address)
            if (res !== "") {
                messagebox.open(qsTr("Error"), qsTr("Unable to update the contact data. Error: " + res))
            }
            updateContactsList()
        }
    }

    function onDeleteContact(ok) {
        if (ok) {
            const res = config.deleteContact(prevCBName, prevCBAddress)
            if (res !== "") {
                messagebox.open(qsTr("Error"), qsTr("Unable to remove the contact '"+ prevCBName +"'.\nError: " + res))
            }
            updateContactsList()
        }
    }

    onVisibleChanged: {
        if (visible) {
            updateContactsList()
        }
    }

    ListModel {
        id: contactsModel
    }

    ListView {
        id: contactsList
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: rect_buttons.bottom
        anchors.topMargin: dp(15)
        model: contactsModel
        delegate: contactsDelegate
        focus: true
    }

    Component {
        id: contactsDelegate
        Rectangle {
            height: dp(140)
            color: "#00000000"
            anchors.left: parent.left
            anchors.right: parent.right

            Rectangle {
                height: dp(130)
                color: "#33bf84ff"
                anchors.top: parent.top
                anchors.topMargin: dp(15)
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                anchors.left: parent.left
                anchors.leftMargin: dp(20)

                Text {
                    color: "#ffffff"
                    text: name
                    font.bold: true
                    font.pixelSize: dp(15)
                    anchors.top: parent.top
                    anchors.topMargin: dp(20)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                }


                Rectangle {
                    height: dp(1)
                    color: "#ffffff"
                    anchors.top: parent.top
                    anchors.topMargin: dp(55)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(35)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                }

                Text {
                    color: "#ffffff"
                    text: address
                    anchors.top: parent.top
                    anchors.topMargin: dp(70)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(150)
                    wrapMode: Text.WrapAnywhere
                    font.pixelSize: dp(15)
                }

                Image {
                    id: image_edit
                    anchors.right: parent.right
                    anchors.rightMargin: dp(85)
                    anchors.top: parent.top
                    anchors.topMargin: dp(70)
                    width: dp(40)
                    height: dp(40)
                    fillMode: Image.PreserveAspectFit
                    source: "../img/Edit@2x.svg"

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            const newContactList = contactList.filter(contact => contact.name !== name)
                            prevCBName = name
                            prevCBAddress = address
                            editDlg.open(name, address, newContactList, true, onEditContact)
                        }
                    }
                }

                Image {
                    id: image_remove
                    anchors.right: parent.right
                    anchors.rightMargin: dp(35)
                    anchors.top: parent.top
                    anchors.topMargin: dp(70)
                    width: dp(40)
                    height: dp(40)
                    fillMode: Image.PreserveAspectFit
                    source: "../img/Delete@2x.svg"

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            prevCBName = name
                            prevCBAddress = address
                            messagebox.open("Remove a contact", "Are you sure, you want to remove the contact " + name + "?", true, "No", "Yes", "", "", "", onDeleteContact)
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: rect_buttons
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: dp(55)
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0
                color: "#9E00E7"
            }

            GradientStop {
                position: 1
                color: "#3600C9"
            }
        }

        Rectangle {
            width: dp(250)
            height: dp(45)
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#00000000"

            Image {
                id: image_add
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                width: dp(45)
                height: dp(45)
                fillMode: Image.PreserveAspectFit
                source: "../img/Add@2x.svg"

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        editDlg.open("", "", contactList, false, onAddContact)
                    }
                }
            }
        }
    }

    ContactEditDlg {
        id: editDlg
        anchors.verticalCenter: parent.verticalCenter
    }
}
