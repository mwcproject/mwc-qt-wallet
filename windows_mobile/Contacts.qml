import QtQuick 2.12
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import ConfigBridge 1.0
import QtQuick.Layouts 1.15
import QtQml 2.15
import QtGraphicalEffects 1.15
import Clipboard 1.0
import "./models"
import "./contact"

Item {
    id: control
    property var contactList: []
    property string prevCBName
    property string prevCBAddress
    property int pressIndex

    property int currentContact: -1

    ConfigBridge {
        id: config
    }

    Clipboard {
        id: clipboard
    }

    //property Component contactDelegate: contactsDelegate

    function updateContactsList() {
        const pairs = config.getContactsAsPairs()
        contactsModel.clear()
        contactList = []


        for (let i = 0; i < pairs.length; i += 2) {
            contactList.push({
                name: pairs[i],
                address: pairs[i+1],
                roundUp: false,
                roundDown: false
            })
        }

        contactList.sort( function( a, b ) {
            a = a.name.toLowerCase();
            b = b.name.toLowerCase();

            return a < b ? -1 : a > b ? 1 : 0;
        });
        let firstChar = 0
        for (let z=0; z <= contactList.length -1 ; z++) {
            let contactFirstChar = contactList[z].name.charAt(0).toLowerCase()
            console.log(firstChar, contactFirstChar)
            if (firstChar !== contactFirstChar) {
                console.log(firstChar, contactList[z].name)
                contactList[z].roundUp = true
                firstChar = contactFirstChar

            }
            if (contactList.length -1 == z || (contactList.length -1 != z && contactFirstChar !== contactList[z+1].name.charAt(0).toLowerCase()) ) {
                contactList[z].roundDown = true
            }

        }

        contactList.forEach(contact => contactsModel.append(contact));

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






    onVisibleChanged: {
        if (visible) {
            updateContactsList()
        } else {
            currentContact = -1
        }
    }

    Rectangle {
        id: nav
        anchors.top: parent.top
        height: navbarTop.navHeight
        width: parent.width
        color: "#00000000" 
    }

    ListModel {
        id: contactsModel
    }

    Rectangle {
        id: container_list
        anchors.top: nav.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        color: Theme.bg
        opacity: 0.5


    }
    ContactView {
        id: contactView
        anchors.fill: container_list
        model: contactsModel

    }
    Rectangle {
        id: add_contact
        height: width
        width: parent.width/8
        radius: dp(150)
        color: "#181818"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height/8
        anchors.right: parent.right
        anchors.rightMargin: dp(25)

        ImageColor {
            img_source: "../../img/add.svg"
            img_height: parent.height * 0.5
            img_color: "white"
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
        }
        MouseArea {
            anchors.fill: parent
            onClicked: editDlg.open("", "", contactList, false, onAddContact)
        }

    }
    DropShadow {
            anchors.fill: add_contact
            horizontalOffset: 0
            verticalOffset: 0
            radius: 12
            samples: 25
            color: "#80000000"
            source: add_contact
        }

}
