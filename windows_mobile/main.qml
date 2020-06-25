import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.13
import CoreWindowBridge 1.0

Window {
    id: window
    visible: true
    title: qsTr("MWC-Mobile-Wallet")
    
    property int currentState
    property string initParams

    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    CoreWindowBridge {
        id: coreWindow
    }

/*
    NONE = 0,
    START_WALLET = 1,           // Start backed mwc713. Check what it want and then delegate control to the next state
    STATE_INIT = 2,             // first run. Creating the password for a wallet
    INPUT_PASSWORD = 3,         // Input password from the wallet
    ACCOUNTS = 4,               // Wallet accounts.              Note!!!  Keep accounts first Action page.
    ACCOUNT_TRANSFER= 5,        // Transfer funds from account to account
    EVENTS = 6,                 // Wallet events (logs)
    HODL = 7,                   // Hodl program.
    SEND = 8,                   // Send coins Page
    RECEIVE_COINS = 9,          // Receive coins
    LISTENING = 10,             // Listening API setting/status
    TRANSACTIONS = 11,          // Transactions dialog
    OUTPUTS = 12,               // Outputs for this wallet
    CONTACTS = 13,              // Contact page. COntacts supported by wallet713
    WALLET_CONFIG = 14,         // Wallet config
    AIRDRDOP_MAIN = 15,         // Starting airdrop page
    SHOW_SEED = 16,             // Show Seed
    NODE_INFO = 17,             // Show node info
    RESYNC = 18,                // Re-sync account with a node
    FINALIZE = 19,              // Finalize transaction. Windowless state
    WALLET_RUNNING_MODE = 20,   // Running mode as a node, wallet or cold wallet

    // Mobile Specific Pages
    WALLET_HOME = 21,           // Wallet home page
    ACCOUNT_OPTIONS = 22,       // Account options page
    WALLET_SETTINGS = 23,       // Settings page

    MIGRATION = 24              // Data migration between wallet versions.
*/

    Connections {
        target: coreWindow
        onSgnUpdateActionStates: {
            currentState = actionState
            navbarItem.updateTitle(currentState)
        }
    }

    Rectangle
    {
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
        anchors.fill: parent
    }

    Inputpassword {
        id: inputPasswordItem
        anchors.fill: parent
        visible: currentState === 3
    }

    Rectangle {
        color: "#00000000"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: dp(90)
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.top: parent.top
        anchors.topMargin: dp(140)

        Wallet {
            id: walletItem
            anchors.fill: parent
            visible: currentState === 21
        }

        AccountOptions {
            id: accountOptionsItem
            anchors.fill: parent
            visible: currentState === 22
        }

        Settings {
            id: settingsItem
            anchors.fill: parent
            visible: currentState === 23
        }

        Send {
            id: sendItem
            anchors.fill: parent
            visible: currentState === 8 && initParams.length === 0
        }

        SendOnline {
            id: sendOnlineItem
            anchors.fill: parent
            visible: currentState === 8 && initParams.length !== 0 && JSON.parse(initParams).isSendOnline
            onVisibleChanged: {
                if (visible) {
                    sendOnlineItem.init(JSON.parse(initParams))
                }
            }
        }

        SendOffline {
            id: sendOfflineItem
            anchors.fill: parent
            visible: currentState === 8 && initParams.length !== 0 && !JSON.parse(initParams).isSendOnline
            onVisibleChanged: {
                if (visible) {
                    sendOfflineItem.init(JSON.parse(initParams))
                }
            }
        }

        Receive {
            id: receiveItem
            anchors.fill: parent
            visible: currentState === 9
        }

        Finalize {
            id: finalizeItem
            anchors.fill: parent
            visible: currentState === 19
        }

        Transactions {
            id: transactionsItem
            anchors.fill: parent
            visible: currentState === 11
        }
    }

    Navbar {
        id: navbarItem
        anchors.fill: parent
        visible: currentState > 3
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:14;anchors_height:200}D{i:19;anchors_y:0}
D{i:21;anchors_width:200}
}
##^##*/
