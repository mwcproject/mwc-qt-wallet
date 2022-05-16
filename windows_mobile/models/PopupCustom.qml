import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import UtilBridge 1.0



TextField {
    property alias colorbg: backgroundTextField.color
    property alias radius: backgroundTextField.radius

    id: control
    background: Rectangle {
        id: backgroundTextField
        color: "#242424"
        radius: dp(15)
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

