import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Window 2.0
import UtilBridge 1.0
import QtGraphicalEffects 1.0

Button {
    property string title: "Open"
    property alias overlay: overlay
    property alias color: rectangle.color
    property alias pixelSize: loginText.font.pixelSize

    id: button_login
    height: dp(45)
    width: parent.width/1.5
    //anchors.horizontalCenter: parent.horizontalCenter
    background: Rectangle {
        id: rectangle
        radius: dp(25)
        LinearGradient {
            start:Qt.point(0,0)
            end:Qt.point(rectangle.width, rectangle.height)
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop {
                    position: 1
                    color: "#3700c9"
                }
                GradientStop {
                    position: 0
                    color: "#c300ff"
                }


            }
            anchors.fill: rectangle
            source: rectangle
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
    ColorOverlay {
        id: overlay
        anchors.fill: rectangle
        source: rectangle
        opacity: 0
    }
    states: [
        State {
            name: "pressed"; when: pressed
            PropertyChanges { target: overlay; color: "black"; opacity: 0.5 }
        },
        State {
            name: "unpressed"; when: !pressed
            PropertyChanges { target: overlay; color: "black"; opacity: 0 }
        }]
    
    transitions: Transition {
        NumberAnimation { properties: "opacity"; duration: 100; easing.type: Easing.InOutQuad }
    }
}


