import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.13
import Qt.labs.platform 1.1
import CoreWindowBridge 1.0

Window {
    id: window
    visible: true
    title: qsTr("MWC-Mobile-Wallet")
    
    property int currentState
    property string initParams
    property int ttl_blocks
    property int questionTextDlgResponse
    property int sendConformationDlgResponse

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

    function updateInitParams(newParams) {
        initParams = newParams
        if (currentState === 2 || currentState === 18) {
            const params = JSON.parse(initParams)
            if (params.currentStep)
                newInstanceItem.updateCurrentStep(params)
            else if(params.msgProgress)
                progressWndItem.init(params.callerId, params.msgProgress)
        }
    }

    function openQuestionTextDlg(title, message, noBtnText, yesBtnText, blockButton, _ttl_blocks, passwordHash = "") {
        messagebox.open(title, message, true, noBtnText, yesBtnText, passwordHash, blockButton, _ttl_blocks, questionTextDlgCallback)
    }

    function questionTextDlgCallback(ret) {
        if (ret) {
            questionTextDlgResponse = 1
        } else {
            questionTextDlgResponse = 0
        }
    }

    function openMessageTextDlg(title, message) {
        messagebox.open(title, message)
    }

    function sendConfirmationDlgCallback(ret) {
        if (ret) {
            sendConformationDlgResponse = 1
        } else {
            sendConformationDlgResponse = 0
        }
    }

    function openSendConfirmationDlg(title, message, passwordHash) {
        sendConfirmationItem.open(title, message, passwordHash, sendConfirmationDlgCallback)
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

    InitWallet {
        id: initWalletItem
        anchors.fill: parent
        visible: currentState === 1
    }

    NewInstance {
        id: newInstanceItem
        anchors.fill: parent
        visible: currentState === 2
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
            visible: currentState === 9 && initParams.length === 0
        }

        Finalize {
            id: finalizeItem
            anchors.fill: parent
            visible: currentState === 19 && initParams.length === 0
        }

        FileTransaction {
            id: fileTransactionItem
            anchors.fill: parent
            visible: (currentState === 9 || currentState === 19) && initParams.length !== 0
        }

        Transactions {
            id: transactionsItem
            anchors.fill: parent
            visible: currentState === 11
        }

        Outputs {
            id: outputsItem
            anchors.fill: parent
            visible: currentState === 12
        }

        Accounts {
            id: accountsItem
            anchors.fill: parent
            visible: currentState === 4
        }

        AccountTransfer {
            id: accountTransferItem
            anchors.fill: parent
            visible: currentState === 5
        }

        Notifications {
            id: notificationsItem
            anchors.fill: parent
            visible: currentState === 6
        }

        WalletConfig {
            id: walletConfigItem
            anchors.fill: parent
            visible: currentState === 14
        }

        NewSeed {
            id: newSeedItem
            anchors.fill: parent
            visible: currentState === 16
        }

        NodeInfo {
            id: nodeInfoItem
            anchors.fill: parent
            visible: currentState === 17
        }

        Listeners {
            id: listenersItem
            anchors.fill: parent
            visible: currentState === 10
        }

        ProgressWnd {
            id: progressWndItem
            anchors.fill: parent
            visible: false
        }

        Contacts {
            id: contactsItem
            anchors.fill: parent
            visible: currentState === 13
        }

        SendConfirmation {
            id: sendConfirmationItem
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    Navbar {
        id: navbarItem
        anchors.fill: parent
        visible: currentState > 3
    }

    MessageBox {
        id: messagebox
        anchors.verticalCenter: parent.verticalCenter
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:14;anchors_height:200}D{i:19;anchors_y:0}
D{i:21;anchors_width:200}
}
##^##*/
