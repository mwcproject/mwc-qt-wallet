import QtQuick 2.12
import QtQuick.Window 2.0
import ProgressWndBridge 1.0

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
        rect_progress.width = rect_bar.width * (pos - progress_min) / (progress_max - progress_min)
        if (msgProgress !== "") {
            text_progress.text = msgProgress
            text_progress.visible = true
        } else {
            text_progress.visible = false
        }
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0
                color: "#9E00E7"
            }

            GradientStop {
                position: 1
                color: "#3600C9"
            }
        }

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
            anchors.bottom: rect_progress.top
            anchors.bottomMargin: dp(25)
            horizontalAlignment: Text.AlignHCenter
        }

        Rectangle {
            id: rect_bar
            anchors.left: parent.left
            anchors.leftMargin: dp(50)
            anchors.right: parent.right
            anchors.rightMargin: dp(50)
            anchors.verticalCenter: parent.verticalCenter
            height: dp(10)
            color: "#4cffffff"
        }

        Rectangle {
            id: rect_progress
            anchors.left: parent.left
            anchors.leftMargin: dp(50)
            anchors.verticalCenter: parent.verticalCenter
            height: dp(10)
            width: 0
            color: "#ffffff"
        }

        Text {
            id: text_progress
            text: qsTr("Restoring From Phrase - 50% complete")
            color: "white"
            font.pixelSize: dp(18)
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: rect_progress.bottom
            anchors.topMargin: dp(25)
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
