import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Window 2.0

Popup {
    id: popup
    modal: false
    focus: false
    property alias text: txt.text

    closePolicy: Popup.NoAutoClose // To close click the notification or wait 3 seconds

    Timer {
        id: timer
        interval: 2000 // milliseconds
        running: true
        repeat: false
        onTriggered: {
            popup.close()
        }
    }

    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
    }

    background: Rectangle {
        color: "blue"
        opacity: 0.1
        radius: 5
    }

    contentItem: Item {
        implicitWidth: txt.implicitWidth
        implicitHeight: txt.implicitHeight

        anchors.fill: parent
        MouseArea {
            anchors.fill: parent
            onClicked: {
                popup.close()
            }
        }
        Text {
            id: txt
            color: "white"
            font.pixelSize: dp(20)
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
    onOpened: {
        timer.start()
    }
}
