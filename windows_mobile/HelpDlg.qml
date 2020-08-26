import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import QtWebView 1.1

Item {
    id: helpDlg

    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    function open(html) {
        webview.loadHtml(html)
        webview.reload()
        delay(100, (function () {
            helpDlg.visible = true;
        }))
    }

    function delay(delayTime, cb) {
        timer.interval = delayTime;
        timer.repeat = false;
        timer.triggered.connect(cb);
        timer.start();
    }

    Timer {
        id: timer
    }

    Rectangle {
        id: rect_header
        height: dp(110)
        color: "#ffffff"
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left

        Text {
            id: text_title
            color: "#3600c9"
            text: qsTr("Help")
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: dp(-10)
            font.bold: true
            font.pixelSize: dp(21)
        }

        Image {
            id: image_close
            width: dp(38)
            height: dp(38)
            anchors.top: parent.top
            anchors.topMargin: dp(25)
            anchors.right: parent.right
            anchors.rightMargin: dp(25)
            source: "../img/MessageBox_Close@2x.svg"
            fillMode: Image.PreserveAspectFit

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    helpDlg.visible = false
                }
            }
        }
    }

    ScrollView {
        id: view_content
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        anchors.top: rect_header.bottom
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.bottomMargin: dp(180)

        Rectangle {
            anchors.fill: parent
            color: white

            WebView {
                id: webview
                anchors.fill: parent
                anchors.leftMargin: dp(20)
                anchors.rightMargin: dp(30)
            }
        }
    }

    Rectangle {
        id: rect_space
        anchors.top: view_content.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: dp(30)
    }

    Rectangle {
        id: rect_bottom
        anchors.top: rect_space.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        Text {
            id: label_bottom
            color: "#3600c9"
            text: qsTr("Ask Questions")
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(15)
        }

        Rectangle {
            id: rect_splitter
            height: dp(2)
            color: "#e2ccf7"
            anchors.top: label_bottom.bottom
            anchors.topMargin: dp(20)
            anchors.right: parent.right
            anchors.rightMargin: dp(50)
            anchors.left: parent.left
            anchors.leftMargin: dp(50)
        }

        Rectangle {
            id: rect_buttons
            height: dp(38)
            width: dp(270)
            color: "#00000000"
            anchors.top: rect_splitter.bottom
            anchors.topMargin: dp(20)
            anchors.horizontalCenter: parent.horizontalCenter

            Button {
                id: button_twitter
                width: dp(79)
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                background: Rectangle {
                    color: "#00000000"
                    radius: dp(5)
                    border.color: "#3600C9"
                    border.width: dp(2)
                    Text {
                        text: qsTr("Twitter")
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: dp(19)
                        color: "#3600C9"
                    }
                }

                onClicked: {
                    Qt.openUrlExternally("https://twitter.com/M_W_Coin")
                }
            }

            Button {
                id: button_telegram
                width: dp(92)
                anchors.left: button_twitter.right
                anchors.leftMargin: dp(8)
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                background: Rectangle {
                    color: "#00000000"
                    radius: dp(5)
                    border.color: "#3600C9"
                    border.width: dp(2)
                    Text {
                        text: qsTr("Telegram")
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: dp(19)
                        color: "#3600C9"
                    }
                }

                onClicked: {
                    Qt.openUrlExternally("https://t.me/mimblewimble_coin")
                }
            }

            Button {
                id: button_discord
                width: dp(83)
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                background: Rectangle {
                    color: "#00000000"
                    radius: dp(5)
                    border.color: "#3600C9"
                    border.width: dp(2)
                    Text {
                        text: qsTr("Discord")
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: dp(19)
                        color: "#3600C9"
                    }
                }

                onClicked: {
                    Qt.openUrlExternally("https://discordapp.com/invite/eUNwqf3")
                }
            }
        }
    }
}
