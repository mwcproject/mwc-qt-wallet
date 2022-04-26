import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Window 2.0
import UtilBridge 1.0
import QtGraphicalEffects 1.0

Button {
    property string title: "Open"

    id: button_login
    height: dp(45)
    width: parent.width/1.5
    anchors.horizontalCenter: parent.horizontalCenter
    background: Rectangle {
        id: rectangle
        radius: dp(25)
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0
                color: "#531d55"
            }
            GradientStop {
                position: 1
                color: "#202020"
            }
        }
        Text {
            id: loginText
            text: qsTr(title)
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(18)
            color: "white"
        }
    }
}


