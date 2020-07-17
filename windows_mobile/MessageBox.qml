import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0

Item {
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    width: dp(400)
    height: dp(400)
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
        anchors.rightMargin: dp(1)
        anchors.leftMargin: dp(1)
        anchors.bottomMargin: dp(1)
        anchors.topMargin: dp(1)
        border.width: dp(1)
        anchors.fill: parent

        Text {
            id: title
            text: qsTr("Title")
            font.bold: true
            anchors.top: parent.top
            anchors.topMargin: dp(30)
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(20)
            color: "#3600c9"
        }

        Text {
            id: content
            width: dp(320)
            height: dp(280)
            text: qsTr("Content")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(21)
            color: "#3600C9"
        }

        Button {
            id: button_ok
            width: dp(135)
            height: dp(50)
            anchors.bottom: parent.bottom
            anchors.bottomMargin: dp(25)
            anchors.horizontalCenter: parent.horizontalCenter

            background: Rectangle {
                color: "#6F00D6"
                radius: dp(5)
                Text {
                    text: qsTr("OK")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: dp(18)
                    color: "white"
                }
            }

            onClicked: {
                messagebox.visible = false
            }
        }
    }

}
