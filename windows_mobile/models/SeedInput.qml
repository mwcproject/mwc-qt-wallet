import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import UtilBridge 1.0
import QtGraphicalEffects 1.0

Rectangle {
    id: control
    property bool field_focus: false
    property bool show_pass: false
    property string placeHolder: "Password"
    property string mainColor: "#242424"
    property alias text: field_password.text
    property string text_index
    property alias textField: field_password
    //property alias mouse: mouse

    color: mainColor
    border.width: dp(0)
    border.color: "black"

    TextFieldCursor {
        id: field_password
        height: parent.height
        width: parent.width
        anchors.left: parent.left
        leftPadding: dp(50)
        topPadding: dp(4)
        inputMethodHints: Qt.ImhSensitiveData | Qt.ImhLowercaseOnly
        font.pixelSize: dp(14)
        placeholderText: qsTr(placeHolder)
        echoMode: "Normal"
        color: "white"
        horizontalAlignment: Text.AlignLeft
        focus: field_focus
        colorbg: "#00000000"
        Rectangle {
            id: circle_index
            height: parent.height - dp(15)
            width: circle_index.height
            color: "#cccccc"
            radius: dp(50)
            anchors.left: parent.left
            anchors.leftMargin: dp(10)
            anchors.verticalCenter: parent.verticalCenter
            Text {
                id: index_word
                color: "#000000"
                text: text_index
                font.bold: true
                font.pixelSize: parent.height*0.55
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}



