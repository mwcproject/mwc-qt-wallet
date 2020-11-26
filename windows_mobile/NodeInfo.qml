import QtQuick 2.0
import QtQuick.Window 2.0
import NodeInfoBridge 1.0
import UtilBridge 1.0

Item {
    property string lastShownErrorMessage
    property int nodeHeightDiffLimit: 5

    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    NodeInfoBridge {
        id: nodeInfo
    }

    UtilBridge {
        id: util
    }

    Connections {
        target: nodeInfo
        onSgnSetNodeStatus: (localNodeStatus, online, errMsg, nodeHeight, peerHeight, totalDifficulty2show, connections) => {
            if (!online) {
                text_status.text = "Offline"
                text_status.color = "#CCFF33"
                text_connections.text = "-"
                text_blocks.text = "-"
                text_difficulty.text = "-"
                if (lastShownErrorMessage !== errMsg) {
                    messagebox.open(qsTr("MWC Node connection error"), qsTr("Unable to retrieve MWC Node status.\n" + errMsg))
                    lastShownErrorMessage = errMsg
                }
            } else {
                if (nodeHeight + nodeHeightDiffLimit < peerHeight) {
                    text_status.text = "Syncing"
                    text_status.color = "#CCFF33"
                }
                else {
                    text_status.text = "Online"
                    text_status.color = "white"
                }

                if (connections <= 0) {
                    text_connections.text = "None"
                    text_connections.color = "#CCFF33"
                }
                else {
                    text_connections.text = Number(connections).toString()
                    text_connections.color = "white"
                }

                text_blocks.text = util.longLong2Str(nodeHeight)
                text_difficulty.text = totalDifficulty2show
            }
        }
    }

    Rectangle {
        id: rect_status
        width: dp(180)
        height: dp(230)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: dp(-150)
        anchors.horizontalCenterOffset: dp(-120)
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#00000000"
        border.color: "white"
        border.width: dp(2)
        radius: dp(5)

        Image {
            id: image_status
            width: dp(100)
            height: dp(100)
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
        width: dp(180)
        height: dp(230)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: dp(-150)
        anchors.horizontalCenterOffset: dp(120)
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#00000000"
        border.color: "white"
        border.width: dp(2)
        radius: dp(5)

        Image {
            id: image_connections
            width: dp(100)
            height: dp(100)
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
        width: dp(180)
        height: dp(230)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: dp(150)
        anchors.horizontalCenterOffset: dp(-120)
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#00000000"
        border.color: "white"
        border.width: dp(2)
        radius: dp(5)

        Image {
            id: image_blocks
            width: dp(100)
            height: dp(100)
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
        width: dp(180)
        height: dp(230)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: dp(150)
        anchors.horizontalCenterOffset: dp(120)
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#00000000"
        border.color: "white"
        border.width: dp(2)
        radius: dp(5)

        Image {
            id: image_difficulty
            width: dp(100)
            height: dp(100)
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

    MessageBox {
        id: messagebox
        anchors.verticalCenter: parent.verticalCenter
    }
}
