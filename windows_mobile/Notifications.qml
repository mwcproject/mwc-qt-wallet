import QtQuick 2.0
import QtQuick.Window 2.12
import EventsBridge 1.0

Item {
    id: notificationsItem

    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    readonly property int msg_group_size: 5
    property var locale: Qt.locale()

    EventsBridge {
        id: events
    }

    function updateShowMessages() {
        const messages = events.getWalletNotificationMessages()
        notificationModel.clear()

        for (let i = messages.length / msg_group_size - 1; i>=0; i--) {
            const date = Date.fromLocaleString(locale, messages[i*msg_group_size + 1], "ddd MMMM dd yyyy hh:mm:ss")
            const level = messages[i * msg_group_size + 2] === "Crit" ? "critical error" : messages[i * msg_group_size + 2];
            const message = messages[i * msg_group_size + 4];

            notificationModel.append({
                time: date.toLocaleString(locale, "MMM dd, yyyy / hh:mm ap"),
                level,
                message
            })
        }
    }

    onVisibleChanged: {
        if(visible) {
            updateShowMessages()
        }
    }

    ListModel {
        id: notificationModel
    }

    ListView {
        anchors.fill: parent
        anchors.top: parent.top
        model: notificationModel
        delegate: notificationDelegate
        focus: true
    }

    Component {
        id: notificationDelegate
        Rectangle {
            height: rect_notification.height + dp(15)
            color: "#00000000"
            anchors.left: parent.left
            anchors.right: parent.right

            Rectangle {
                id: rect_notification
                height: dp(50) + text_message.height
                color: "#33bf84ff"
                anchors.top: parent.top
                anchors.topMargin: dp(15)
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                anchors.left: parent.left
                anchors.leftMargin: dp(20)

                Rectangle {
                    width: dp(10)
                    height: parent.height
                    anchors.top: parent.top
                    anchors.left: parent.left
                    color: "#BCF317"
                    visible: level === "critical error"
                }

                Text {
                    color: "#BF84FF"
                    text: time + "  / " + level
                    font.pixelSize: dp(15)
                    anchors.top: parent.top
                    anchors.topMargin: dp(15)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(40)
                }

                Text {
                    id: text_message
                    color: "#ffffff"
                    text: message
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    anchors.top: parent.top
                    anchors.topMargin: dp(37)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(40)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(30)
                    font.pixelSize: dp(16)
                    font.bold: true
                }
            }
        }
    }
}
