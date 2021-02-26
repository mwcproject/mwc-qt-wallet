import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import QtWebView 1.1

Item {
    id: helpDlg

    function open(docName, html) {
        switch (docName) {
        case 'accounts.html':
            text_title.text = "Help: Accounts"
            webview_accounts.loadHtml(html)
            webview_accounts.update()
            webview_accounts.visible = true
            webview_contacts.visible = false
            webview_notifications.visible = false
            webview_finalize.visible = false
            webview_listener.visible = false
            webview_node_overview.visible = false
            webview_outputs.visible = false
            webview_receive.visible = false
            webview_mnemonic.visible = false
            webview_running_mode.visible = false
            webview_send.visible = false
            webview_swap.visible = false
            webview_transactions.visible = false
            webview_wallet_configuration.visible = false
            webview_default.visible = false
            break;
        case 'contacts.html':
            text_title.text = "Help: Contacts"
            webview_contacts.loadHtml(html)
            webview_contacts.update()
            webview_accounts.visible = false
            webview_contacts.visible = true
            webview_notifications.visible = false
            webview_finalize.visible = false
            webview_listener.visible = false
            webview_node_overview.visible = false
            webview_outputs.visible = false
            webview_receive.visible = false
            webview_mnemonic.visible = false
            webview_running_mode.visible = false
            webview_send.visible = false
            webview_swap.visible = false
            webview_transactions.visible = false
            webview_wallet_configuration.visible = false
            webview_default.visible = false
            break;
        case 'event_log.html':
            text_title.text = "Help: Notifications"
            webview_notifications.loadHtml(html)
            webview_notifications.update()
            webview_accounts.visible = false
            webview_contacts.visible = false
            webview_notifications.visible = true
            webview_finalize.visible = false
            webview_listener.visible = false
            webview_node_overview.visible = false
            webview_outputs.visible = false
            webview_receive.visible = false
            webview_mnemonic.visible = false
            webview_running_mode.visible = false
            webview_send.visible = false
            webview_swap.visible = false
            webview_transactions.visible = false
            webview_wallet_configuration.visible = false
            webview_default.visible = false
            break;
        case 'finalize.html':
            text_title.text = "Help: Finalize"
            webview_finalize.loadHtml(html)
            webview_finalize.update()
            webview_accounts.visible = false
            webview_contacts.visible = false
            webview_notifications.visible = false
            webview_finalize.visible = true
            webview_listener.visible = false
            webview_node_overview.visible = false
            webview_outputs.visible = false
            webview_receive.visible = false
            webview_mnemonic.visible = false
            webview_running_mode.visible = false
            webview_send.visible = false
            webview_swap.visible = false
            webview_transactions.visible = false
            webview_wallet_configuration.visible = false
            webview_default.visible = false
            break;
        case 'listener.html':
            text_title.text = "Help: Listener"
            webview_listener.loadHtml(html)
            webview_listener.update()
            webview_accounts.visible = false
            webview_contacts.visible = false
            webview_notifications.visible = false
            webview_finalize.visible = false
            webview_listener.visible = true
            webview_node_overview.visible = false
            webview_outputs.visible = false
            webview_receive.visible = false
            webview_mnemonic.visible = false
            webview_running_mode.visible = false
            webview_send.visible = false
            webview_swap.visible = false
            webview_transactions.visible = false
            webview_wallet_configuration.visible = false
            webview_default.visible = false
            break;
        case 'node_overview.html':
            text_title.text = "Help: Node Overview"
            webview_node_overview.loadHtml(html)
            webview_node_overview.update()
            webview_accounts.visible = false
            webview_contacts.visible = false
            webview_notifications.visible = false
            webview_finalize.visible = false
            webview_listener.visible = false
            webview_node_overview.visible = true
            webview_outputs.visible = false
            webview_receive.visible = false
            webview_mnemonic.visible = false
            webview_running_mode.visible = false
            webview_send.visible = false
            webview_swap.visible = false
            webview_transactions.visible = false
            webview_wallet_configuration.visible = false
            webview_default.visible = false
            break;
        case 'outputs.html':
            text_title.text = "Help: Outputs"
            webview_outputs.loadHtml(html)
            webview_outputs.update()
            webview_accounts.visible = false
            webview_contacts.visible = false
            webview_notifications.visible = false
            webview_finalize.visible = false
            webview_listener.visible = false
            webview_node_overview.visible = false
            webview_outputs.visible = true
            webview_receive.visible = false
            webview_mnemonic.visible = false
            webview_running_mode.visible = false
            webview_send.visible = false
            webview_swap.visible = false
            webview_transactions.visible = false
            webview_wallet_configuration.visible = false
            webview_default.visible = false
            break;
        case 'receive.html':
            text_title.text = "Help: Receive"
            webview_receive.loadHtml(html)
            webview_receive.update()
            webview_accounts.visible = false
            webview_contacts.visible = false
            webview_notifications.visible = false
            webview_finalize.visible = false
            webview_listener.visible = false
            webview_node_overview.visible = false
            webview_outputs.visible = false
            webview_receive.visible = true
            webview_mnemonic.visible = false
            webview_running_mode.visible = false
            webview_send.visible = false
            webview_swap.visible = false
            webview_transactions.visible = false
            webview_wallet_configuration.visible = false
            webview_default.visible = false
            break;
        case 'seed.html':
            text_title.text = "Help: Mnemonic"
            webview_mnemonic.loadHtml(html)
            webview_mnemonic.update()
            webview_accounts.visible = false
            webview_contacts.visible = false
            webview_notifications.visible = false
            webview_finalize.visible = false
            webview_listener.visible = false
            webview_node_overview.visible = false
            webview_outputs.visible = false
            webview_receive.visible = false
            webview_mnemonic.visible = true
            webview_running_mode.visible = false
            webview_send.visible = false
            webview_swap.visible = false
            webview_transactions.visible = false
            webview_wallet_configuration.visible = false
            webview_default.visible = false
            break;
        case 'select_mode.html':
            text_title.text = "Help: Select Running Mode"
            webview_running_mode.loadHtml(html)
            webview_running_mode.update()
            webview_accounts.visible = false
            webview_contacts.visible = false
            webview_notifications.visible = false
            webview_finalize.visible = false
            webview_listener.visible = false
            webview_node_overview.visible = false
            webview_outputs.visible = false
            webview_receive.visible = false
            webview_mnemonic.visible = false
            webview_running_mode.visible = true
            webview_send.visible = false
            webview_swap.visible = false
            webview_transactions.visible = false
            webview_wallet_configuration.visible = false
            webview_default.visible = false
            break;
        case 'send.html':
            text_title.text = "Help: Send"
            webview_send.loadHtml(html)
            webview_send.update()
            webview_accounts.visible = false
            webview_contacts.visible = false
            webview_notifications.visible = false
            webview_finalize.visible = false
            webview_listener.visible = false
            webview_node_overview.visible = false
            webview_outputs.visible = false
            webview_receive.visible = false
            webview_mnemonic.visible = false
            webview_running_mode.visible = false
            webview_send.visible = true
            webview_swap.visible = false
            webview_transactions.visible = false
            webview_wallet_configuration.visible = false
            webview_default.visible = false
            break;
        case 'swap.html':
            text_title.text = "Help: Swap"
            webview_swap.loadHtml(html)
            webview_swap.update()
            webview_accounts.visible = false
            webview_contacts.visible = false
            webview_notifications.visible = false
            webview_finalize.visible = false
            webview_listener.visible = false
            webview_node_overview.visible = false
            webview_outputs.visible = false
            webview_receive.visible = false
            webview_mnemonic.visible = false
            webview_running_mode.visible = false
            webview_send.visible = false
            webview_swap.visible = true
            webview_transactions.visible = false
            webview_wallet_configuration.visible = false
            webview_default.visible = false
            break;
        case 'transactions.html':
            text_title.text = "Help: Transactions"
            webview_transactions.loadHtml(html)
            webview_transactions.update()
            webview_accounts.visible = false
            webview_contacts.visible = false
            webview_notifications.visible = false
            webview_finalize.visible = false
            webview_listener.visible = false
            webview_node_overview.visible = false
            webview_outputs.visible = false
            webview_receive.visible = false
            webview_mnemonic.visible = false
            webview_running_mode.visible = false
            webview_send.visible = false
            webview_swap.visible = false
            webview_transactions.visible = true
            webview_wallet_configuration.visible = false
            webview_default.visible = false
            break;
        case 'wallet_configuration.html':
            text_title.text = "Help: Wallet Configuration"
            webview_wallet_configuration.loadHtml(html)
            webview_wallet_configuration.update()
            webview_accounts.visible = false
            webview_contacts.visible = false
            webview_notifications.visible = false
            webview_finalize.visible = false
            webview_listener.visible = false
            webview_node_overview.visible = false
            webview_outputs.visible = false
            webview_receive.visible = false
            webview_mnemonic.visible = false
            webview_running_mode.visible = false
            webview_send.visible = false
            webview_swap.visible = false
            webview_transactions.visible = false
            webview_wallet_configuration.visible = true
            webview_default.visible = false
            break;
        default:
            text_title.text = "Help"
            webview_default.loadHtml(html)
            webview_default.update()
            webview_accounts.visible = false
            webview_contacts.visible = false
            webview_notifications.visible = false
            webview_finalize.visible = false
            webview_listener.visible = false
            webview_node_overview.visible = false
            webview_outputs.visible = false
            webview_receive.visible = false
            webview_mnemonic.visible = false
            webview_running_mode.visible = false
            webview_send.visible = false
            webview_swap.visible = false
            webview_transactions.visible = false
            webview_wallet_configuration.visible = false
            webview_default.visible = true
            break;
        }

        delay(200, (function () {
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
            font.pixelSize: dp(24)
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
            color: "white"

            WebView {
                id: webview_accounts
                visible: false
                anchors.fill: parent
                anchors.leftMargin: dp(20)
                anchors.rightMargin: dp(30)
            }

            WebView {
                id: webview_contacts
                visible: false
                anchors.fill: parent
                anchors.leftMargin: dp(20)
                anchors.rightMargin: dp(30)
            }

            WebView {
                id: webview_notifications
                visible: false
                anchors.fill: parent
                anchors.leftMargin: dp(20)
                anchors.rightMargin: dp(30)
            }

            WebView {
                id: webview_finalize
                visible: false
                anchors.fill: parent
                anchors.leftMargin: dp(20)
                anchors.rightMargin: dp(30)
            }

            WebView {
                id: webview_listener
                visible: false
                anchors.fill: parent
                anchors.leftMargin: dp(20)
                anchors.rightMargin: dp(30)
            }

            WebView {
                id: webview_node_overview
                visible: false
                anchors.fill: parent
                anchors.leftMargin: dp(20)
                anchors.rightMargin: dp(30)
            }

            WebView {
                id: webview_outputs
                visible: false
                anchors.fill: parent
                anchors.leftMargin: dp(20)
                anchors.rightMargin: dp(30)
            }

            WebView {
                id: webview_receive
                visible: false
                anchors.fill: parent
                anchors.leftMargin: dp(20)
                anchors.rightMargin: dp(30)
            }

            WebView {
                id: webview_mnemonic
                visible: false
                anchors.fill: parent
                anchors.leftMargin: dp(20)
                anchors.rightMargin: dp(30)
            }

            WebView {
                id: webview_running_mode
                visible: false
                anchors.fill: parent
                anchors.leftMargin: dp(20)
                anchors.rightMargin: dp(30)
            }

            WebView {
                id: webview_send
                visible: false
                anchors.fill: parent
                anchors.leftMargin: dp(20)
                anchors.rightMargin: dp(30)
            }

            WebView {
                id: webview_swap
                visible: false
                anchors.fill: parent
                anchors.leftMargin: dp(20)
                anchors.rightMargin: dp(30)
            }

            WebView {
                id: webview_transactions
                visible: false
                anchors.fill: parent
                anchors.leftMargin: dp(20)
                anchors.rightMargin: dp(30)
            }

            WebView {
                id: webview_wallet_configuration
                visible: false
                anchors.fill: parent
                anchors.leftMargin: dp(20)
                anchors.rightMargin: dp(30)
            }

            WebView {
                id: webview_default
                visible: false
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
