import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import UtilBridge 1.0



TextField {
    id: control
    background: Rectangle {
        id: backgroundTextField
        color: "#0d0f12"
        radius: dp(5)
        border.color: "#262933"
        border.width: dp(1)
    }
    MouseArea {
        anchors.fill: parent
        onClicked: {
            backgroundTextField.border.color = "#ff0019"
        }
    }
    cursorDelegate: Rectangle {
        id: cursor
        visible: false
        color: "salmon"
        width: dp(1.5)

        SequentialAnimation {
            loops: Animation.Infinite
            running: control.cursorVisible

            PropertyAction {
                target: cursor
                property: 'visible'
                value: true
            }

            PauseAnimation {
                duration: 600
            }

            PropertyAction {
                target: cursor
                property: 'visible'
                value: false
            }

            PauseAnimation {
                duration: 600
            }

            onStopped: {
                cursor.visible = false
            }
        }
    }
}

