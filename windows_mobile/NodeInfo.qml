import QtQuick 2.0
import QtQuick.Window 2.0
import NodeInfoBridge 1.0
import UtilBridge 1.0

Item {
    property string lastShownErrorMessage
    property int nodeHeightDiffLimit: 5
    property int heightBlock

    NodeInfoBridge {
        id: nodeInfo
    }

    UtilBridge {
        id: util
    }



    onVisibleChanged: {
        if (visible) {
            if (parent.height > dp(470)) {
                anchors.topMargin = (parent.height - dp(470)) / 2
            }
        }
    }

    Rectangle {
        id: rect_status
        width: dp(160)
        height: dp(210)
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: (parent.width / 2 - dp(160)) / 2
        color: "#00000000"
        border.color: "white"
        border.width: dp(2)
        radius: dp(5)

        Image {
            id: image_status
            width: dp(80)
            height: dp(80)
            anchors.top: parent.top
            anchors.topMargin: dp(20)
            anchors.horizontalCenter: parent.horizontalCenter
            source: "../img/Ready@2x.svg"
            fillMode: Image.PreserveAspectFit
        }

        Text {
            id: text_status
            color: "white"
            text: qsTr("Online")
            anchors.top: image_status.bottom
            anchors.topMargin: dp(20)
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(20)
            font.bold: true
        }

        Rectangle {
            width: dp(100)
            height: dp(2)
            color: "#e2ccf7"
            anchors.top: text_status.bottom
            anchors.topMargin: dp(15)
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            id: label_status
            color: "white"
            text: qsTr("Status")
            anchors.top: text_status.bottom
            anchors.topMargin: dp(30)
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(17)
        }
    }

    Rectangle {
        id: rect_connections
        width: dp(160)
        height: dp(210)
        anchors.verticalCenter: rect_status.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: (parent.width / 2 - dp(160)) / 2
        color: "#00000000"
        border.color: "white"
        border.width: dp(2)
        radius: dp(5)

        Image {
            id: image_connections
            width: dp(80)
            height: dp(80)
            anchors.top: parent.top
            anchors.topMargin: dp(20)
            anchors.horizontalCenter: parent.horizontalCenter
            source: "../img/Connections@2x.svg"
            fillMode: Image.PreserveAspectFit
        }

        Text {
            id: text_connections
            color: "white"
            text: qsTr("8")
            anchors.top: image_connections.bottom
            anchors.topMargin: dp(20)
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(20)
            font.bold: true
        }

        Rectangle {
            width: dp(100)
            height: dp(2)
            color: "#e2ccf7"
            anchors.top: text_connections.bottom
            anchors.topMargin: dp(15)
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            id: label_connections
            color: "white"
            text: qsTr("Connections")
            anchors.top: text_connections.bottom
            anchors.topMargin: dp(30)
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(17)
        }
    }

    Rectangle {
        id: rect_blocks
        width: dp(160)
        height: dp(210)
        anchors.top: rect_status.bottom
        anchors.topMargin: dp(50)
        anchors.horizontalCenter: rect_status.horizontalCenter
        color: "#00000000"
        border.color: "white"
        border.width: dp(2)
        radius: dp(5)

        Image {
            id: image_blocks
            width: dp(80)
            height: dp(80)
            anchors.top: parent.top
            anchors.topMargin: dp(20)
            anchors.horizontalCenter: parent.horizontalCenter
            source: "../img/Blockchain@2x.svg"
            fillMode: Image.PreserveAspectFit
        }

        Text {
            id: text_blocks
            color: "white"
            text: qsTr("631,898")
            anchors.top: image_blocks.bottom
            anchors.topMargin: dp(20)
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(20)
            font.bold: true
        }

        Rectangle {
            width: dp(100)
            height: dp(2)
            color: "#e2ccf7"
            anchors.top: text_blocks.bottom
            anchors.topMargin: dp(15)
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            id: label_blocks
            color: "white"
            text: qsTr("Blocks")
            anchors.top: text_blocks.bottom
            anchors.topMargin: dp(30)
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(17)
        }
    }

    Rectangle {
        id: rect_difficulty
        width: dp(160)
        height: dp(210)
        anchors.verticalCenter: rect_blocks.verticalCenter
        anchors.horizontalCenter: rect_connections.horizontalCenter
        color: "#00000000"
        border.color: "white"
        border.width: dp(2)
        radius: dp(5)

        Image {
            id: image_difficulty
            width: dp(80)
            height: dp(80)
            anchors.top: parent.top
            anchors.topMargin: dp(20)
            anchors.horizontalCenter: parent.horizontalCenter
            source: "../img/Difficulty@2x.svg"
            fillMode: Image.PreserveAspectFit
        }

        Text {
            id: text_difficulty
            color: "white"
            text: qsTr("76,223 M")
            anchors.top: image_difficulty.bottom
            anchors.topMargin: dp(20)
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(20)
            font.bold: true
        }

        Rectangle {
            width: dp(100)
            height: dp(2)
            color: "#e2ccf7"
            anchors.top: text_difficulty.bottom
            anchors.topMargin: dp(15)
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            id: label_difficulty
            color: "white"
            text: qsTr("Difficulty")
            anchors.top: text_difficulty.bottom
            anchors.topMargin: dp(30)
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(17)
        }
    }
}
