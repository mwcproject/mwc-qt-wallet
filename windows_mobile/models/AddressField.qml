import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import UtilBridge 1.0
import QtGraphicalEffects 1.0
import "../."







Rectangle {
    id: control
    property bool field_focus: false
    property bool show_pass: false
    property string placeHolder: "Password"
    property string mainColor: "#242424"
    property alias text: field_password.text
    property alias textField: field_password
    property alias mouseContact: mouseContact

    property alias mouse: mouse

    color: mainColor
    radius: dp(20)

    TextFieldCursor {
        id: field_password
        height: parent.height
        width: parent.width - dp(30) - img_contact.width //- img_qr.width
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        leftPadding: dp(20)
        topPadding: dp(4)
        inputMethodHints: Qt.ImhSensitiveData
        font.pixelSize: dp(18)
        echoMode: "Normal"
        color: "white"
        horizontalAlignment: Text.AlignLeft
        focus: field_focus
        colorbg: "#00000000"
        MouseArea {
            id: mouse
            anchors.fill: parent
            onClicked: {
                field_focus = true
            }
        }
    }
    /*ImageColor {
        id: img_qr
        img_height: parent.height/1.7
        img_source: "../../img/qrcode.svg"
        img_color: "#ffffff"
        anchors.right: img_contact.left
        anchors.rightMargin: dp(10)
        anchors.verticalCenter: parent.verticalCenter
        MouseArea {
            anchors.fill: parent
            onClicked: {
                show_pass = !show_pass
                text_pass(show_pass)
            }
        }
    }*/
    ImageColor {
        id: img_contact
        img_height: parent.height/2
        img_source: "../../img/contact.svg"
        img_color: "#ffffff"
        anchors.right: parent.right
        anchors.rightMargin: dp(10)
        anchors.verticalCenter: parent.verticalCenter
        MouseArea {
            id: mouseContact
            anchors.fill: parent
        }


    }

}




