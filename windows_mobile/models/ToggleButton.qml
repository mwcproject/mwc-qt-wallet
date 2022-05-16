import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import "../."



Item {
    id: root
    property alias mouse: mouseArea
    state: "disabled"

    Rectangle {
        id: skype
        height: parent.height
        width: parent.width
        color: root.state == "disabled" ? "green": "red"
        radius: dp(50)

        Rectangle {
            id: rect
            height: parent.height*0.5
            width: height
            radius: dp(50)
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: height/3
            anchors.rightMargin: height/3
            color: "white"
            states: [
                State {
                    when: mouseArea.pressed;
                    PropertyChanges {
                        target: rect
                        height: root.height*0.8
                    }
                }]
            transitions: Transition {
                PropertyAnimation {
                    properties: "height";
                    duration: 250
                }
            }
        }
        MouseArea {
            id: mouseArea
            anchors.fill: parent
        }
    }

    states: [

        State {
            name: "disabled"
            AnchorChanges {
                target: rect; anchors.left: parent.left
            }
            PropertyChanges {
                target: skype; color: "#222222"; anchors.leftMargin: height/3
            }
            PropertyChanges {
                target: rect; color: "#353535"
            }
        },
        State {
            name: "enabled"
            AnchorChanges {
                target: rect; anchors.right: parent.right
            }
            PropertyChanges {
                target: skype; color: "#0c87f2"; anchors.rightMargin: height/3

            }

        }]

    transitions: Transition {
        AnchorAnimation {
            duration: 150
        }
        PropertyAnimation {
            properties: "color";
            duration: 150
        }
    }
}
