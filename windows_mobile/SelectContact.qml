import QtQuick 2.12
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import ConfigBridge 1.0

Item {
    id: selectContactItem
    property var callback
    property var contactList: []
    property string prevCBName
    property string prevCBAddress

    function open(_callback) {
        callback = _callback
        selectContactItem.visible = true
    }

    ConfigBridge {
        id: config
    }

    function updateContactsList(searchStr) {
        const pairs = config.getContactsAsPairs()
        contactsModel.clear()
        for (let i = 0; i < pairs.length; i += 2) {
            if (searchStr === "" || pairs[i].includes(searchStr)) {
                contactList.push({
                     name: pairs[i],
                     address: pairs[i+1]
                })
                contactsModel.append(contactList[contactList.length - 1])
            }
        }
    }

    function onAddContact(ok, name, address) {
        if (ok) {
            const res = config.addContact(name, address)
            if (res !== "") {
                messagebox.open(qsTr("Error"), qsTr("Unable to add a new contact.\n" + res))
            }
            updateContactsList("")
        }
    }

    function onEditContact(ok, name, address) {
        if (ok) {
            const res = config.updateContact(prevCBName, prevCBAddress, name, address)
            if (res !== "") {
                messagebox.open(qsTr("Error"), qsTr("Unable to update the contact data. Error: " + res))
            }
            updateContactsList("")
        }
    }

    function onDeleteContact(ok) {
        if (ok) {
            const res = config.deleteContact(prevCBName, prevCBAddress)
            if (res !== "") {
                messagebox.open(qsTr("Error"), qsTr("Unable to remove the contact '"+ prevCBName +"'.\nError: " + res))
            }
            updateContactsList("")
        }
    }

    onVisibleChanged: {
        if (visible) {
            updateContactsList("")
        }
    }

    Rectangle {
        id: rect_background
        anchors.fill: parent
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

        MouseArea {
            anchors.fill: parent
            onClicked: {
                textfield_searchbar.focus = false
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
            anchors.bottomMargin: dp(90)
            anchors.top: rect_topbar.bottom
            anchors.topMargin: dp(15)
            model: contactsModel
            delegate: contactsDelegate
            focus: true
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    textfield_searchbar.focus = false
                }
            }
        }

        Component {
            id: contactsDelegate
            Rectangle {
                height: dp(140)
                color: "#00000000"
                anchors.left: parent.left
                anchors.right: parent.right

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        contactsList.currentIndex = index
                    }
                }

                Rectangle {
                    height: dp(130)
                    color: "#33bf84ff"
                    anchors.top: parent.top
                    anchors.topMargin: dp(15)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(20)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(20)

                    Rectangle {
                        width: dp(10)
                        height: parent.height
                        anchors.top: parent.top
                        anchors.left: parent.left
                        color: "#BCF317"
                        visible: index === contactsList.currentIndex
                    }

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
            id: rect_topbar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: dp(20)
            anchors.right: parent.right
            anchors.rightMargin: dp(20)
            height: dp(60)
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    textfield_searchbar.focus = false
                }
            }
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
            TextField {
                id: textfield_searchbar
                height: dp(50)
                padding: dp(10)
                leftPadding: dp(20)
                font.pixelSize: dp(18)
                placeholderText: qsTr("Search by name")
                color: "white"
                text: ""
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: image_add.left
                anchors.rightMargin: dp(20)
                horizontalAlignment: Text.AlignLeft
                background: Rectangle {
                    color: "#8633E0"
                    radius: dp(5)
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        textfield_searchbar.focus = true
                    }
                }

                onTextChanged: {
                    updateContactsList(textfield_searchbar.text)
                }
            }

            Image {
                id: image_add
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
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

        Button {
            id: button_select
            width: dp(150)
            height: dp(50)
            anchors.bottom: parent.bottom
            anchors.bottomMargin: dp(20)
            anchors.right: parent.right
            anchors.rightMargin: dp(100)

            background: Rectangle {
                color: "white"
                radius: dp(5)
                border.width: dp(2)
                border.color: "white"
                Text {
                    text: qsTr("Select")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: dp(18)
                    color: "#6F00D6"
                }
            }

            onClicked: {
                if (contactsList.currentIndex >= 0) {
                    selectContactItem.visible = false
                    callback(true, contactList[contactsList.currentIndex])

                } else {
                    messagebox.open(qsTr("Error"), qsTr("Please select a contact!"))
                }
            }
        }

        Button {
            id: button_cancel
            width: dp(150)
            height: dp(50)
            anchors.bottom: parent.bottom
            anchors.bottomMargin: dp(20)
            anchors.left: parent.left
            anchors.leftMargin: dp(100)

            background: Rectangle {
                color: "#00000000"
                radius: dp(5)
                border.width: dp(2)
                border.color: "white"
                Text {
                    text: qsTr("Cancel")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: dp(18)
                    color: "white"
                }
            }

            onClicked: {
                selectContactItem.visible = false
                callback(false)
            }
        }


        ContactEditDlg {
            id: editDlg
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
