import QtQuick 2.12
import QtQuick.Window 2.0
import ProgressWndBridge 1.0
import "./models"

Item {
    id: element
    property int progress_min: 0
    property int progress_max: 100

    function init(params) {
        progressWnd.callerId = params.callerId
        setHeader(params.header)
        updateProgress(0, params.msgProgress)
        setMsgPlus(params.msgPlus)
        element.visible = true
    }

    ProgressWndBridge {
        id: progressWnd
    }

    Connections {
        target: progressWnd

        onSgnInitProgress: (min, max) => {
            progress_min = min
            progress_max = max
        }

        onSgnSetHeader: (header) => {
            setHeader(header)
        }

        onSgnSetMsgPlus: (msgPlus) => {
            setMsgPlus(msgPlus)
        }

        onSgnUpdateProgress: (pos, msgProgress) => {
            updateProgress(pos, msgProgress)
        }
    }

    function setHeader(header) {
        if (header !== "") {
            text_header.text = header
            text_header.visible = true
        } else {
            text_header.visible = false
        }
    }

    function setMsgPlus(msgPlus) {
        if (msgPlus !== "") {
            text_msgplus.text = msgPlus
            text_msgplus.visible = true
        } else {
            text_msgplus.visible = false
        }
    }

    function updateProgress(pos, msgProgress) {
        progress_circle.arcEnd = 360 * (pos - progress_min) / (progress_max - progress_min)
        text_perc.text = String(Math.ceil((progress_circle.arcEnd / 3.6))) + "%"
        if (msgProgress !== "") {
            text_progress.text = msgProgress
            text_progress.visible = true
        } else {
            text_progress.visible = false
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.bg

        Text {
            id: text_header
            text: qsTr("Recovering from the passphrase")
            color: "white"
            font.pixelSize: dp(22)
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: dp(50)
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            id: text_msgplus
            text: qsTr("Preparing for recovery...")
            color: "white"
            font.pixelSize: dp(18)
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: progress_circle.top
            anchors.bottomMargin: dp(25)
            horizontalAlignment: Text.AlignHCenter
        }

        ProgressCircle {
            id: progress_circle
                size: parent.width/3
                colorCircle: "#FF3333"
                colorBackground: "gray"
                showBackground: true
                arcBegin: 0
                arcEnd: 0
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                lineWidth: dp(10)
                Text {
                    id: text_perc
                    text: ""
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: dp(30)
                    font.family: barlow.bold
                    color: "white"
                }
            }

        Text {
            id: text_progress
            text: qsTr("Restoring From Phrase - 50% complete")
            color: "white"
            font.pixelSize: dp(18)
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: progress_circle.bottom
            anchors.topMargin: dp(25)
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
