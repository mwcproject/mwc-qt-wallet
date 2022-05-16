import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import UtilBridge 1.0
import QtGraphicalEffects 1.0







Rectangle {
    id: control
    property bool field_focus: false
    property bool show_pass: false
    property string placeHolder: "Search by uuid, amount, address"
    property string mainColor: "#242424"
    property alias text: field_password.text
    property alias textField: field_password

    property alias mouse: mouse

    color: mainColor
    radius: dp(10)


    TextFieldCursor {
        id: field_password
        height: parent.height
        width: parent.width - img_qr.width
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        leftPadding: dp(20)
        rightPadding: dp(15)
        topPadding: dp(4)
        //inputMethodHints: Qt.ImhSensitiveData
        placeholderText: qsTr(placeHolder)
        font.pixelSize: dp(17)
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
    ImageColor {
        id: img_qr
        img_height: parent.height/1.7
        img_source: "../../img/qrcode.svg"
        img_color: "gray"
        anchors.left: control.left
        anchors.leftMargin: dp(10)
        anchors.verticalCenter: parent.verticalCenter
    }

}




