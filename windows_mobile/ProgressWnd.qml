import QtQuick 2.0
import QtQuick.Window 2.0
import ProgressWndBridge 1.0

Item {
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    property int progress_min: 0
    property int progress_max: 100

    function init(_callerId, msgProgress) {
        progressWnd.callerId = _callerId
        updateProgress(0, msgProgress)
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

        onSgnUpdateProgress: (pos, msgProgress) => {
            updateProgress(pos, msgProgress)
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
        id: rect_bar
        anchors.left: parent.left
        anchors.leftMargin: dp(50)
        anchors.right: parent.right
        anchors.rightMargin: dp(50)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: dp(-50)
        height: dp(4)
        color: "#4cffffff"
    }

    Rectangle {
        id: rect_progress
        anchors.left: parent.left
        anchors.leftMargin: dp(50)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: dp(-50)
        height: dp(4)
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
