import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import UtilBridge 1.0
import QtGraphicalEffects 1.0







Rectangle {
    id: control
    property bool field_focus: false
    property string mainColor: "#242424"
    property alias text: field_password.text
    property alias textField: field_password
    property alias mouse: mouse

    color: mainColor
    radius: dp(20)


    TextFieldCursor {
        id: field_password
        height: parent.height
        width: parent.width
        anchors.left: parent.left
        leftPadding: dp(20)
        topPadding: dp(4)
        inputMethodHints: Qt.ImhDigitsOnly
        anchors.verticalCenter: parent.verticalCenter
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
}




