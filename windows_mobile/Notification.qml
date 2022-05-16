import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

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
    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 750}
    }

    background: Rectangle {
        color: "#252525"
        radius: dp(25)
    }

    contentItem: Item {
        implicitWidth: con.implicitWidth
        implicitHeight: con.implicitHeight
        anchors.horizontalCenter: parent.horizontalCenter
        MouseArea {
            anchors.fill: parent
            onClicked: {
                timer.stop()
                popup.close()

            }
        }
        Rectangle {
            id: con
            implicitWidth : txt.width + dp(20)
            implicitHeight : txt.height + dp(20)
            color: "#00000000"
            Text {
                id: txt
                color: "white"
                font.pixelSize: dp(15)
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

    }
    onOpened: {
        timer.start()
    }
}
