import QtQuick 2.0
import QtQuick.Controls 2.13

Item {
    width: 280
    height: 200
    id: messagebox
    visible: false

    function open(subject, description) {
        title.text = subject
        content.text = description
        messagebox.visible = true
    }

    Rectangle {
        id: rectangle
        color: "#ffffff"
        anchors.rightMargin: 1
        anchors.leftMargin: 1
        anchors.bottomMargin: 1
        anchors.topMargin: 1
        border.width: 1
        anchors.fill: parent

        Text {
            id: title
            text: qsTr("Title")
            font.bold: true
            anchors.top: parent.top
            anchors.topMargin: 25
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 20
            color: "#3600c9"
        }

        Text {
            id: content
            width: 200
            height: 80
            text: qsTr("Content")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 15
            color: "#3600c9"
        }

        Button {
            id: button_ok
            width: 150
            height: 30
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            anchors.horizontalCenter: parent.horizontalCenter

            background: Rectangle {
                color: "#9e00e7"
                radius: 10
                Text {
                    text: qsTr("OK")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pointSize: 15
                    color: "white"
                }
            }

            onClicked: {
                messagebox.visible = false
            }
        }
    }

}
