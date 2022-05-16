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
    property alias textField: field_password
    property alias mouse: mouse


    function text_pass(show_pass) {
        if (field_password.text == "") {
            field_password.font.letterSpacing = dp(0)
            field_password.font.pixelSize = dp(18)
        } else if (!show_pass) {
            field_password.font.letterSpacing = dp(2)
            field_password.font.pixelSize = dp(9)
        } else {
            field_password.font.letterSpacing = dp(0)
            field_password.font.pixelSize = dp(18)
        }
    }

    color: mainColor
    radius: dp(20)
    border.width: dp(0)
    border.color: "#00000000"

    //onVisibleChanged:

    TextFieldCursor {
        id: field_password
        height: parent.height - dp(4)
        width: parent.width - dp(40)
        anchors.left: parent.left
        leftPadding: dp(20)

        //inputMethodHints: Qt.ImhSensitiveData
        //padding: dp(2)
        font.pixelSize: text_pass(show_pass)
        placeholderText: qsTr(placeHolder)
        echoMode: show_pass? "Normal" : "Password"
        color: "white"
        anchors.verticalCenter: parent.verticalCenter
        horizontalAlignment: Text.AlignLeft
        focus: field_focus
        onTextEdited: text_pass(show_pass)
        colorbg: "#00000000"

        MouseArea {
            id: mouse
            anchors.fill: parent
            onClicked: {
                field_focus = true
            }
        }
    }
    Image {
        id: image
        height: dp(30)
        anchors.left: field_password.right
        anchors.verticalCenter: parent.verticalCenter
        source: show_pass? "../../img/visibility.svg" : "../../img/invisible.svg"
        fillMode: Image.PreserveAspectFit
        MouseArea {
            id: visible
            anchors.fill: parent
            onClicked: {
                show_pass = !show_pass
                text_pass(show_pass)
            }
        }

   }
    ColorOverlay {
        id: overlay
           anchors.fill: image
           source: image
           color: "#5c5c5c"
       }
    states: State {
        name: "pressed"; when: visible.pressed
        PropertyChanges { target: overlay; opacity: 0.5 }
    }

    transitions: Transition {
        NumberAnimation { properties: "opacity"; duration: 250; easing.type: Easing.InOutQuad }
    }
}




