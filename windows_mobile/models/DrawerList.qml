import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Templates 2.15 as T
import QtQuick.Window 2.0
import QtQuick.Layouts 1.15


Drawer {
    id: nav

    property alias repeater: rep
    property alias title: text_title
    property alias image: img
    property alias secondaryTitle: text_secondary
    property bool onTouch: false

    property var pHeight: parent.height
    height: Math.min(parent.height/1.7, column.Layout.minimumHeight+ rect_title.height)
    width: parent.width
    edge: Qt.BottomEdge
    interactive: position == 1.0 && !onTouch? true : false

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
        id: rec
        anchors.top: parent.top
        height: pHeight/1.7
        width: parent.width
        color: "#00000000"
        Rectangle {
            id: rect_title
            anchors.top: parent.top
            anchors.bottom: text_secondary.bottom
            height: parent.height/3
            width: parent.width
            color: "#00000000"
            Image{
                id: img
                height: parent.height/2
                source: "../../img/currency.svg"
                fillMode: Image.PreserveAspectFit
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: dp(10)
            }
            Text {
                id: text_title
                text: qsTr("Select a currency")
                font.family: barlow.bold
                anchors.top: img.bottom
                anchors.topMargin: dp(10)
                anchors.left: parent.left
                anchors.leftMargin: dp(10)
                anchors.right: parent.right
                anchors.rightMargin: dp(10)
                font.pixelSize: dp(16)
                wrapMode: Text.WordWrap
                horizontalAlignment:Text.AlignHCenter
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
                color: "white"
            }
            Text {
                id: text_secondary
                text: qsTr("Each MWC amount in the wallet will show the total worth in the currency selected as well")
                font.family: barlow.bold
                anchors.top: text_title.bottom
                anchors.topMargin: dp(10)
                anchors.left: parent.left
                anchors.leftMargin: dp(10)
                anchors.right: parent.right
                anchors.rightMargin: dp(10)
                font.pixelSize: dp(14)
                wrapMode: Text.WordWrap
                horizontalAlignment:Text.AlignHCenter
                color: "white"
            }

        }
        Flickable {
            id: scroll
            width: parent.width
            height: parent.height - rect_title.height - dp(10)
            anchors.top: rect_title.bottom
            anchors.topMargin: dp(10)
            //anchors.topMargin: re
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
            onFlickStarted: {
                nav.onTouch = true
            }
            onFlickEnded: {
                nav.onTouch = false
            }
        }
    }

    background: Rectangle {
        radius: 50
        height: column.Layout.minimumHeight + rect_title.height //+ parent.height/5
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
