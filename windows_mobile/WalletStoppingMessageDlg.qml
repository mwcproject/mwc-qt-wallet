import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import UtilBridge 1.0

Item {
    id: messagebox
    visible: false

    function open(taskTimeout) {
        let waitingTime = ""
        if (taskTimeout < 120*1000 ) {
            waitingTime = Number(taskTimeout/1000).toString() + " seconds"
        }
        else {
            waitingTime = Number(taskTimeout/1000/60).toString() + " minutes"
        }
        text_message.text = "mwc713 stopping might take up to <b>" + waitingTime + "</b> because it need to finish current running task.\n\nPlease wait until mwc713 will finish the task and exit."
        // adjust rect_messagebox height
        rect_messagebox.height = text_message.height + dp(130)
        messagebox.visible = true
    }

    Rectangle {
        anchors.fill: parent
        color: "#00000000"

        MouseArea {
            anchors.fill: parent
        }
    }

    Rectangle {
        id: rect_messagebox
        height: text_message.height + dp(130)
        anchors.left: parent.left
        anchors.leftMargin: dp(25)
        anchors.right: parent.right
        anchors.rightMargin: dp(25)
        anchors.verticalCenter: parent.verticalCenter

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
                id: text_title
                text: qsTr("Title")
                font.bold: true
                anchors.top: parent.top
                anchors.topMargin: dp(38)
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(22)
                color: "#3600c9"
            }

            Text {
                id: text_message
                anchors.left: parent.left
                anchors.leftMargin: dp(40)
                anchors.right: parent.right
                anchors.rightMargin: dp(40)
                text: qsTr("Content")
                textFormat: Text.StyledText
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: dp(20)
                font.pixelSize: dp(18)
                color: "#3600C9"
            }
        }
    }
}
