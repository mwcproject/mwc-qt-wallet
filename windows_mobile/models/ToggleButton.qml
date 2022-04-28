import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import "../."

Item {
    id: root
    state: "disabled"

    Rectangle {
        id: skype
        height: parent.height
        width: parent.width
        color: root.state == "disabled" ? Theme.green : Theme.red
        radius: dp(50)

        Rectangle {
            id: rect
            height: parent.height*1.2
            width: height
            radius: dp(50)
            anchors.verticalCenter: parent.verticalCenter
            color: "gray"
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {

            }
        }
    }

    states: [

        State {
            name: "disabled"
            AnchorChanges {
                target: rect; anchors.left: parent.left
            }
            PropertyChanges {
                target: skype; color: Theme.red
            }
        },
        State {
            name: "enabled"
            AnchorChanges {
                target: rect; anchors.right: parent.right
            }
            PropertyChanges {
                target: skype; color: Theme.green
            }
        }]

    transitions: Transition {
        AnchorAnimation {
            duration: 250
        }
        PropertyAnimation {
            properties: "color";
            duration: 250
        }
    }
}
