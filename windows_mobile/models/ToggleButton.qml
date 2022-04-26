import QtQuick 2.15

Item {
    id: root
    anchors.fill: parent
    state: "left"

    Rectangle {
        id: skype
        height: 50
        width: 125
        color: root.state == "left" ? "green" : "red"
        radius: 50

        Rectangle {
            id: rect
            height: parent.height*1.2
            width: height
            radius: 50
            anchors.verticalCenter: parent.verticalCenter
            color: "gray"
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.state = root.state === "left" ? "right" : "left"
            }
        }
    }

    states: [

        State {
            name: "left"
            AnchorChanges {
                target: rect; anchors.left: parent.left
            }
            PropertyChanges {
                target: skype; color: "red"
            }
        },
        State {
            name: "right"
            AnchorChanges {
                target: rect; anchors.right: parent.right
            }
            PropertyChanges {
                target: skype; color: "green"
            }
        }]

    transitions: Transition {
        AnchorAnimation {
            duration: 500
        }
        PropertyAnimation {
            properties: "color";
            duration: 500
        }
    }
}
