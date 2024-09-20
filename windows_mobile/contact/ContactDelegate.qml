import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import ConfigBridge 1.0
import "../models"

ItemDelegate {
    id: delegate

    ConfigBridge {
        id: config
    }

    function colorProfil(address, isBackground){
        let md5Address = Qt.md5(address).substring(0,8)
        let r = parseInt(md5Address.substring(0,2), 16)
        let g = parseInt(md5Address.substring(2,4), 16)
        let b = parseInt(md5Address.substring(4,6), 16)
        let a = Math.min(Math.max(parseInt(md5Address.substring(6,8), 16),25,100))

        if (isBackground) {
            return Qt.rgba(r/255, g/255, b/255, 0.2)
        }

        return Qt.rgba(r/255, g/255, b/255, 1)
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
            let name = contactsModel.get(pressIndex).name
            let addr = contactsModel.get(pressIndex).address
            const res = config.deleteContact(name, addr)
            if (res !== "") {
                messagebox.open(qsTr("Error"), qsTr("Unable to remove the contact '"+ prevCBName +"'.\nError: " + res))
            }
            updateContactsList()
            isPressHold = false
        }
    }



    checkable: false

    onClicked: {
        if (currentContact !== index) {
            currentContact = index
        } else {
             currentContact = -1
        }
        console.log(currentContact)

    }

    background: RectangleSideRound{
        rec_width: parent.width
        rec_height: parent.height
        rec_color: "#181818"
        upRound: roundUp
        downRound: roundDown
        rec_radius: rect.height/2
    }


    clip: true

    contentItem: ColumnLayout {
        Rectangle{
            id: rect
            height: dp(40)
            width: parent.width
            color: "#00000000"
            Rectangle {
                id: initial_logo
                height: parent.height*0.75
                width: height
                radius: width/2
                color: colorProfil(address, true)
                anchors.left: parent.left
                anchors.leftMargin: dp(20)
                anchors.verticalCenter: parent.verticalCenter
                Text {
                    color: colorProfil(address, false)
                    text: name.charAt(0)
                    font.pixelSize: initial_logo.height*0.6
                    font.capitalization: Font.AllUppercase
                    font.family: barlow.light
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Label {
                id: text_name
                text: name
                font.bold: true
                anchors.left: initial_logo.right
                anchors.leftMargin: dp(15)
                elide: Text.ElideRight
                font.pixelSize: dp(16)
                anchors.verticalCenter: parent.verticalCenter
                color: "white"
            }
        }



        ColumnLayout {
            id: grid
            spacing: 0

            Layout.alignment: Qt.AlignHCenter

            Behavior on Layout.preferredHeight {
                          NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
                        }
            Label {
                id: label_address
                text: qsTr("Address")
                Layout.topMargin: dp(10)
                Layout.bottomMargin: dp(10)
                Layout.alignment: Qt.AlignHCenter
                color: "white"
            }

            Rectangle {
                id: rect_contact_addr
                Layout.alignment: Qt.AlignHCenter
                color: "#252525"
                radius: dp(10)
                height: dp(20) + text_contact_address.height
                width: rect.width*0.9

                TextEdit {
                    id: text_contact_address
                    readOnly: true
                    Layout.alignment: Qt.AlignHCenter
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.rightMargin: dp(10)
                    anchors.leftMargin: dp(10)
                    anchors.topMargin: dp(10)
                    wrapMode: Text.WrapAnywhere
                    font.pixelSize: dp(15)
                    color: "white"
                    text: address
                }
            }
            RowLayout {
                Layout.topMargin: dp(10)
                Layout.bottomMargin: dp(10)
                Layout.alignment : Qt.AlignVCenter | Qt.AlignHCenter
                spacing: dp(40)

                Rectangle {
                    height: dp(25)
                    width: height
                    color: "green"
                    radius: dp(50)

                    ImageColor {
                        id: image_send
                        img_source: "../../img/arrow.svg"
                        img_height: parent.height/2.5
                        img_color: "white"
                        img_rotation: 180
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                isPressHold = false
                            }
                        }

                   }
                }


                ImageColor {
                    id: image_copy
                    img_source: "../../img/copy.svg"
                    img_height: dp(25)
                    img_color: "white"
                    img_rotation: 0
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            const name =  contactsModel.get(index).name
                            clipboard.text = contactsModel.get(index).address
                            notification.text = qsTr("%1 Address copied").arg(name)
                            notification.open()

                            isPressHold = false
                        }
                    }

               }
                ImageColor {
                    id: image_edit
                    img_source: "../../img/edit.svg"
                    img_height: dp(25)
                    img_color: "white"
                    img_rotation: 0
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            const name = contactsModel.get(index).name
                            const addr = contactsModel.get(index).address
                            const newContactList = contactList.filter(contact => contactList.name !== name)
                            editDlg.open(name, addr, newContactList, true, onEditContact)
                        }
                    }

               }
                ImageColor {
                    id: image_delete
                    img_source: "../../img/remove.svg"
                    img_height: dp(25)
                    img_color: "#00000000"
                    img_rotation: 0
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            messagebox.open("Remove a contact", "Are you sure, you want to remove the contact " + contactsModel.get(index).name + "?", true, "No", "Yes", "", "", "", onDeleteContact)
                        }
                    }

               }
            }
        }
        Rectangle {
            visible: !roundDown
            height: dp(0.5)
            width: parent.width/1.3
            color: "gray"
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom
            Layout.rightMargin: (parent.width - width)/3
        }
    }

    states: [
        State {
            name: "close"
            when: currentContact !== index
            PropertyChanges {
                target: grid; Layout.preferredHeight: 0
            }

        },
        State {
            name: "expanded"
            when: currentContact === index
            PropertyChanges {
                target: grid; Layout.preferredHeight: Layout.minimumHeight
            }

        }
    ]
}

