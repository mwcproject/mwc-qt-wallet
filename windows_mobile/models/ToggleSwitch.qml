import QtQuick 2.15

Item {
    id: root
    property string text_1: "left"
    property string text_2: "right"
    state: text_1

    Rectangle {
        id: rectMain
        height: testnetText.height*3
        width: mainnetText.width*3
        color: "#151515"
        radius: dp(50)
        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.state = root.state === text_1 ? text_2 : text_1
            }
        }

        Rectangle {
            id: rect
            height: parent.height*0.8
            width: parent.width*0.45
            radius: dp(50)
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.topMargin: dp(7)
            anchors.bottomMargin: dp(7)
            color: "#343b47"
        }
        Text {
            id: mainnetText
            text: text_1
            font.pixelSize: root.height*0.4
            anchors.left: parent.left
            anchors.leftMargin: (rectMain.width/2 - width) / 2 + dp(5)
            anchors.verticalCenter: parent.verticalCenter
            font.bold: true
            color: "white"
        }
        Text {
            id: testnetText
            text: text_2
            font.pixelSize: root.height*0.4
            anchors.right: parent.right
            anchors.rightMargin: (rectMain.width/2 - width) / 2  + dp(5)
            anchors.verticalCenter: parent.verticalCenter
            font.bold: true
            color: "white"
        }

    }

    states: [

        State {
            name: text_1
            AnchorChanges {
                target: rect
                anchors.left: parent.left
            }
            PropertyChanges {
                target: rect; anchors.leftMargin: dp(10)
            }
            PropertyChanges {
                target: mainnetText; color: "white"
            }
            PropertyChanges {
                target: testnetText; color: "gray"
            }
        },
        State {
            name: text_2
            AnchorChanges {
                target: rect; anchors.right: parent.right
            }
            PropertyChanges {
                target: rect; anchors.rightMargin: dp(10)
            }
            PropertyChanges {
                target: mainnetText; color: "gray"
            }
            PropertyChanges {
                target: testnetText; color: "white"
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
