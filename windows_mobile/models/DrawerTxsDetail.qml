import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Templates 2.15 as T
import QtQuick.Window 2.0
import QtQuick.Layouts 1.15


Drawer {
    id: nav

    property alias repeater: rep
    height: Math.min(parent.height/2 + dp(50), column.Layout.minimumHeight+ dp(50))
    width: parent.width
    edge: Qt.BottomEdge
    interactive: position == 1.0? true : false

    enter: Transition {
        SmoothedAnimation {
            velocity: -1
            duration: 750
            easing.type: Easing.OutCirc
            easing.amplitude: 2.0
            easing.period: 1.5
        }
    }
    exit: Transition {
        SmoothedAnimation {
            velocity: -1
            duration: 500
            easing.type: Easing.OutCirc
            easing.amplitude: 2.0
            easing.period: 1.5

        }

    }

    Rectangle {
        anchors.top: parent.top
        anchors.topMargin: dp(50)
        height: pHeight/2
        width: parent.width
        color: "#00000000"
        Flickable {
            id: scroll
            width: parent.width
            height: parent.height
            contentHeight: column.Layout.minimumHeight
            clip: true
            ScrollBar.vertical: ScrollBar {
                policy: Qt.ScrollBarAsNeeded
            }
            ColumnLayout {
                id: column
                spacing: 0
                width: nav.width
                Repeater {
                    id: rep

                }
            }
        }
    }

    background: Rectangle {
        radius: 50
        height: column.Layout.minimumHeight+ dp(50) + parent.height/5
        width: parent.width
        //y: -parent.height/5
        color: "#151515"
    }

    T.Overlay.modal: Rectangle {
        color: "#80000000"
        Behavior on opacity {
            NumberAnimation {
                duration: 150
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:4;anchors_width:248;anchors_x:220}D{i:7;anchors_width:150;anchors_x:270}
}
##^##*/
