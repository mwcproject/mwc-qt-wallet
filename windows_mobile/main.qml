import QtQuick 2.15
import QtQuick.Window 2.12
import QtQuick.Controls 2.13
import Qt.labs.platform 1.1
import CoreWindowBridge 1.0
import StateMachineBridge 1.0
import QtQml 2.15
import "./models"




Window {
    id: window
    visible: true
    title: qsTr("MWC-Mobile-Wallet")


    property int currentState
    property string initParams
    property int ttl_blocks
    property int questionTextDlgResponse
    property int sendConfirmationDlgResponse
    property bool onLoadTxs
    property bool onLoadAccount
    property double price: 586590.26

    property bool darkTheme: true

    property bool hiddenAmount: false
    property string hidden: "🌒" //"💰💰💰💰"//"▒▒▒"


    property var txsList: []
    property var txsListCache: []

    property double totalAmount: 0

    property double spendableBalance: 0
    property double awaitBalance: 0
    property double lockedBalance: 0

    property string selectedAccount

    readonly property double dpi: 3.0 + (Screen.pixelDensity - 22.1) / 10
    function dp(x) { return x * dpi }

    CoreWindowBridge {
        id: coreWindow
    }

    StateMachineBridge {
        id: stateMachine
    }
    /*Text {
        font.pixelSize: Style.textSize
        color: Style.textColor
        text: "Hello World"
    }*/

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
            navbarTop.updateTitle(currentState)
            if (currentState !== 2 && currentState !== 18)
                progressWndItem.visible = false
        }
    }

    onClosing: {
        const windowDialog = (navbarTop.open || inputDlg.visible || editDlg.visible  || messagebox.visible || transactionDetail.visible)
        if (windowDialog) {
            if (inputDlg.visible)
                inputDlg.visible = false
            if (editDlg.visible)
                editDlg.visible = false
            if (messagebox.visible)
                messagebox.visible  = false
            if (transactionDetail.visible)
                transactionDetail.visible  = false
            if (navbarTop.open)
                navbarTop.open = false
            close.accepted = false
            return
        }
        if (stateMachine.returnBack()) {
            close.accepted = false
            return
        }
    }

    function updateInitParams(newParams) {
        initParams = newParams
        progressWndItem.visible = false
        if (currentState === 2 || currentState === 18) {
            const params = JSON.parse(initParams)
            if (params.currentStep)
                newInstanceItem.updateCurrentStep(params)
            else
                progressWndItem.init(params)
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
            sendConfirmationDlgResponse = 1
        } else {
            sendConfirmationDlgResponse = 0
        }
    }

    function openSendConfirmationDlg(title, info, passwordHash) {
        sendConfirmationItem.open(title, info, passwordHash, sendConfirmationDlgCallback)
    }

    function openWalletStoppingMessageDlg(taskTimeout) {
        walletStoppingMessageDlg.open(taskTimeout)
    }

    function closeWalletStoppingMessageDlg() {
        walletStoppingMessageDlg.visible = false
    }

    Dark {
        id: dark
    }

    Light {
        id: light
    }

    MyFont {
        id: barlow
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop {
                position: 0
                color: darkTheme? dark.bgGradientTop : light.bgGradientTop
            }

            GradientStop {
                position: 0.2
                color: darkTheme? dark.bgGradientBottom : light.bgGradientBottom
            }
        }
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
        anchors.top: parent.top
        anchors.topMargin: parent.height/14
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height/14
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 0

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
            id: walletSettingsItem
            anchors.fill: parent
            visible: currentState === 23
        }

        FileTransactionReceive {
            id: fileTransactionReceiveItem
            anchors.fill: parent
            visible: currentState === 9 && initParams.length !== 0 && initParams.search("backStateId") < 0
        }

        FileTransactionFinalize {
            id: fileTransactionFinalizeItem
            anchors.fill: parent
            visible: currentState === 19 && initParams.length !== 0 && initParams.search("fileNameOrSlatepack") >= 0
        }

        ResultedSlatepack {
            id: resultedSlatepackItem
            anchors.fill: parent
            visible: (currentState === 8 || currentState === 9) && initParams.length !== 0 && initParams.search("backStateId") >= 0 && initParams.search("svg") < 0
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

        /*
        SelectContact {
            id: selectContactItem
        }*/

        /*Accounts {
            id: accountsItem
            anchors.fill: parent
            visible: currentState === 4
        }*/

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
    }

    Send {
        id: sendItem
        anchors.fill: parent
        visible: currentState === 8 && initParams.length === 0
    }

    SendOnline {
        id: sendOnlineItem
        anchors.fill: parent
        visible: currentState === 8 && initParams.length !== 0 && initParams.search("isSendOnline") >= 0 && JSON.parse(initParams).isSendOnline
        onVisibleChanged: {
            if (visible) {
                sendOnlineItem.init(JSON.parse(initParams))

            }
        }
    }

    SendOffline {
        id: sendOfflineItem
        anchors.fill: parent
        visible: currentState === 8 && initParams.length !== 0 && initParams.search("isSendOnline") >= 0 && !JSON.parse(initParams).isSendOnline && initParams.search("backStateId") < 0
        onVisibleChanged: {
            if (visible) {
                sendOfflineItem.init(JSON.parse(initParams))
            }
        }
    }

    SendConfirmation {
        id: sendConfirmationItem
        anchors.verticalCenter: parent.verticalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height/14
    }

    Receive {
        id: receiveItem
        anchors.fill: parent
        visible: (currentState === 9 && initParams.length === 0)
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height/14
    }

    QrCode {
        id: qrCode
        anchors.fill: parent
        visible: currentState === 9 && initParams.length !== 0 && (initParams.search("svg") >= 0)
        onVisibleChanged: {
            if (visible) {
                qrCode.init(JSON.parse(initParams))
            }
        }
    }

    Finalize {
        id: finalizeItem
        anchors.fill: parent
        visible: currentState === 19 && initParams.length === 0
    }

    TransactionDetail {
        id: transactionDetail
        anchors.fill: parent
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height/14
    }



    NavbarBottom {
        id: navbarBottom
        anchors.fill: parent
        visible: currentState > 3
    }

    NavbarTop {
        id: navbarTop
        anchors.fill: parent
        visible: currentState > 3 && (currentState != 8 && currentState != 9 && currentState != 9 && !transactionDetail.visible)
        //anchors.bottom: parent.bottom
        //anchors.bottomMargin: parent.height/14
    }


    /*InputSlatepack {
        id: inputSlatepack
        anchors.fill: parent
    }*/

    SendSettings {
        id: settingsItem
        anchors.fill: parent
    }

    InputDlg {
        id: inputDlg
        anchors.fill: parent
    }

    ContactEditDlg {
        id: editDlg
        anchors.fill: parent
    }



    OutputDetail {
        id: outputDetailItem
        anchors.fill: parent
    }

    WalletStoppingMessageDlg {
        id: walletStoppingMessageDlg
        anchors.fill: parent
    }

    ShowProofDlg {
        id: showProofDlg
        anchors.fill: parent
    }

    MessageBox {
        id: messagebox
        anchors.fill: parent
    }

    HelpDlg {
        id: helpDlg
        anchors.fill: parent
    }

    Rectangle {
        x: (parent.width - notification.width)/2
        anchors.bottom: parent.bottom
        anchors.bottomMargin: (parent.height/14)*2
        Notification {
            id: notification
        }
    }

}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:14;anchors_height:200}D{i:19;anchors_y:0}
D{i:21;anchors_width:200}
}
##^##*/
