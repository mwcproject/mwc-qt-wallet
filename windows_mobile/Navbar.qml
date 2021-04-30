import QtQuick 2.12
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
import WalletBridge 1.0
import StateMachineBridge 1.0
import ConfigBridge 1.0

Item {
    readonly property int status_ignore: 0
    readonly property int status_red: 1
    readonly property int status_yellow: 2
    readonly property int status_green: 3

    property string docName

    function updateTitle(state) {
        switch (state) {
        case 4:
            text_title.text = qsTr("Wallet  >   Accounts")
            break
        case 5:
            text_title.text = qsTr("Wallet  >   Accounts  >   AccountTransfer")
            break
        case 6:
            text_title.text = qsTr("Notifications")
            break
        case 8:
            text_title.text = qsTr("Wallet  >   Send")
            break
        case 9:
            text_title.text = qsTr("Wallet  >   Receive")
            break
        case 10:
            text_title.text = qsTr("Wallet Settings   >   Listeners")
            break
        case 11:
            text_title.text = qsTr("Wallet  >   Transactions")
            break
        case 12:
            text_title.text = qsTr("Wallet  >   Outputs")
            break
        case 13:
            text_title.text = qsTr("Account Options   >   Contacts")
            break
        case 14:
            text_title.text = qsTr("Wallet Settings   >   Config")
            break
        case 16:
            text_title.text = qsTr("Account Options   >   Passphrase")
            break
        case 17:
            text_title.text = qsTr("Wallet Settings   >   MWC Node Status (Cloud)")
            break
        case 19:
            text_title.text = qsTr("Wallet  >   Finalize")
            break
        case 21:
            text_title.text = qsTr("Wallet")
            break
        case 22:
            text_title.text = qsTr("Account Options")
            break
        case 23:
            text_title.text = qsTr("Wallet Settings")
            break
        }
    }

    function changeInstanceCallback(ret) {
        if (ret) {
            stateMachine.logout()
        }
    }

    WalletBridge {
        id: wallet
    }

    StateMachineBridge {
        id: stateMachine
    }

    ConfigBridge {
        id: config
    }

    Connections {
        target: wallet

        onSgnWalletBalanceUpdated: {
            text_balance.text = wallet.getTotalMwcAmount() + " MWC"
        }

        onSgnUpdateNodeStatus: (online, errMsg, nodeHeight, peerHeight, totalDifficulty, connections) => {
           if ( !online ) {
               setStatusButtonState(true, status_red, "")
           }
           else if (connections === 0 || nodeHeight === 0 || ( peerHeight > 0 && peerHeight - nodeHeight > 5 )) {
               setStatusButtonState(true, status_yellow, "")
           }
           else {
               setStatusButtonState(true, status_green, "")
           }
        }

        onSgnConfigUpdate: {
            updateNetworkName()
            updateInstanceAccountText()
        }

        onSgnLoginResult: {
            updateNetworkName()
        }

        onSgnUpdateListenerStatus: {
            updateListenerBtn()
        }

        onSgnHttpListeningStatus: {
            updateListenerBtn()
        }
    }

    onVisibleChanged: {
        if (visible) {
            updateListenerBtn()
            updateNetworkName()
            updateInstanceAccountText()
//            updateAccountList()
        }
    }

    function updateNetworkName() {
        setStatusButtonState(true, status_ignore, config.getNetwork())
    }

    function updateListenerBtn() {
        const mqsStatus = wallet.getMqsListenerStatus()
        const torStatus = wallet.getTorListenerStatus()
        const httpListenerStatus = wallet.getHttpListeningStatus()

        let listening = mqsStatus | torStatus
        let listenerNames = ""
        if (mqsStatus)
            listenerNames +=  "MWC MQS"

        if (torStatus) {
            if (listenerNames !== "")
                listenerNames += ", "
            listenerNames += "TOR"
        }

        if (httpListenerStatus === "true") {
            listening = true
            if (listenerNames !== "")
                listenerNames += ", "
            listenerNames += "Http"
            if (config.hasTls())
                listenerNames += "s"
        }

        setStatusButtonState(false, listening ? status_green : status_red, listening ? listenerNames : "Listeners")
    }

    function setStatusButtonState(isNetwork, status, text) {
        if (isNetwork) {
            switch (status) {
                case status_green:
                    image_network.source = "../img/CircGreen@2x.svg"
                    break;
                case status_red:
                    image_network.source = "../img/CircRed@2x.svg"
                    break;
                case status_yellow:
                    image_network.source = "../img/CircYellow@2x.svg"
                    break;
                default: // Ingnore suppose to be here
                    break;
            }

            if (text !== "")
                text_network.text = text
        } else {
            switch (status) {
                case status_green:
                    image_listener.source = "../img/CircGreen@2x.svg"
                    break;
                case status_red:
                    image_listener.source = "../img/CircRed@2x.svg"
                    break;
                case status_yellow:
                    image_listener.source = "../img/CircYellow@2x.svg"
                    break;
                default: // Ingnore suppose to be here
                    break;
            }

            if (text !== "")
                text_listener.text = text
        }
    }

    function updateInstanceAccountText() {
        text_instance_account.text = "INSTANCE:  " + config.getCurrentWalletInstance()[2] + "  //  ACCOUNT:  " + wallet.getCurrentAccountName()
    }

//    function updateAccountList() {
//        const accountInfo = wallet.getWalletBalance(true, true, false)
//        const selectedAccount = wallet.getCurrentAccountName()
//        let selectedAccIdx = 0

//        accountItems.clear()

//        let idx = 0
//        for (let i = 1; i < accountInfo.length; i += 2) {
//            if (accountInfo[i-1] === selectedAccount)
//                selectedAccIdx = idx

//            accountItems.append({ info: accountInfo[i-1] + accountInfo[i].substring(27), account: accountInfo[i-1]})
//            idx++
//        }
//        accountComboBox.currentIndex = selectedAccIdx
//    }

    Rectangle {
        id: navbarTop
        height: dp(100)
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
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

        Rectangle {
            id: menuRect
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: dp(50)
            color: "#00000000"

            Rectangle {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                width: dp(60)
                color: "#00000000"

                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: dp(28)
                    width: dp(6)
                    height: dp(6)
                    radius: dp(3)
                }

                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: dp(39)
                    width: dp(6)
                    height: dp(6)
                    radius: dp(3)
                }

                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: dp(50)
                    width: dp(6)
                    height: dp(6)
                    radius: dp(3)
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        nav.toggle()
                    }
                }
            }

            Text {
                id: text_title
                color: "#ffffff"
                text: "Wallet"
                anchors.horizontalCenter: parent.horizontalCenter
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: dp(16)
            }
        }

        Rectangle {
            id: rect_splitter
            width: dp(250)
            height: dp(1)
            color: "#ffffff"
            anchors.top: menuRect.bottom
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Rectangle {
            id: rect_balance
            width: dp(72) + text_balance.width
            height: dp(50)
            color: "#00000000"
            anchors.top: menuRect.bottom
            anchors.horizontalCenter: parent.horizontalCenter

            Image {
                id: image_logo1
                width: dp(58)
                height: dp(29)
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
                source: "../img/TBLogo@2x.svg"
            }

            Text {
                id: text_balance
                text: ""
                anchors.left: image_logo1.right
                anchors.leftMargin: dp(14)
                font.pixelSize: dp(20)
                font.bold: true
                color: "white"
                anchors.verticalCenter: image_logo1.verticalCenter
            }
        }
    }

    Rectangle
    {
        id: navbarBottom
        height: dp(90)
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
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

        Image {
            id: image_notifications
            width: dp(28)
            height: dp(28)
            anchors.bottom: text_instance_account.top
            anchors.bottomMargin: dp(13)
            anchors.left: parent.left
            anchors.leftMargin: dp(28)
            fillMode: Image.PreserveAspectFit
            source: "../img/NavNotificationNormal@2x.svg"

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    stateMachine.setActionWindow(6) // Notifications Page
                }
            }
        }

        Image {
            id: image_help
            width: dp(28)
            height: dp(28)
            anchors.left: image_notifications.right
            anchors.leftMargin: dp(17)
            anchors.verticalCenter: image_notifications.verticalCenter
            fillMode: Image.PreserveAspectFit
            source: "../img/HelpBtn@2x.svg"

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    docName = stateMachine.getCurrentHelpDocName()
                    var xhr = new XMLHttpRequest
                    xhr.open('GET', "qrc:/help_mobile/" + docName)
                    xhr.onreadystatechange = function() {
                        if (xhr.readyState === XMLHttpRequest.DONE) {
                            var response = xhr.responseText
                            helpDlg.open(docName, response)
                        }
                    }
                    xhr.send()
                }
            }
        }

        Rectangle {
            id: rect_listener
            width: text_listener.width + dp(40)
            height: dp(25)
            color: "#00000000"
            radius: dp(12.5)
            border.width: dp(1)
            border.color: "#ffffff"
            anchors.right: parent.right
            anchors.rightMargin: dp(28)
            anchors.verticalCenter: image_help.verticalCenter

            Image {
                id: image_listener
                width: dp(12)
                height: dp(12)
                anchors.left: parent.left
                anchors.leftMargin: dp(10)
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
                source: "../img/CircGreen@2x.svg"
            }

            Text {
                id: text_listener
                color: "#ffffff"
                text: qsTr("MWC MQS")
                anchors.left: image_listener.right
                anchors.leftMargin: dp(8)
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: dp(11)
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    stateMachine.setActionWindow(10)    // Listening Page
                }
            }
        }

        Rectangle {
            id: rect_network
            width: text_network.width + dp(40)
            height: dp(25)
            color: "#00000000"
            radius: dp(12.5)
            anchors.rightMargin: dp(12)
            anchors.verticalCenter: image_help.verticalCenter
            border.width: dp(1)
            border.color: "#ffffff"
            anchors.right: rect_listener.left

            Image {
                id: image_network
                width: dp(12)
                height: dp(12)
                anchors.left: parent.left
                anchors.leftMargin: dp(10)
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
                source: "../img/CircGreen@2x.svg"
            }

            Text {
                id: text_network
                text: qsTr("Floonet")
                anchors.left: image_network.right
                anchors.leftMargin: dp(8)
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: dp(11)
                color: "white"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    stateMachine.setActionWindow(17)    // NodeInfo Page
                }
            }
        }

        Text {
            id: text_instance_account
            color: "white"
            text: qsTr("INSTANCE:  Default  //  ACCOUNT:  SatoshisDream")
            anchors.bottom: parent.bottom
            anchors.bottomMargin: dp(14)
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: dp(13)
        }
    }

    NavigationDrawer {
        id: nav
        Rectangle
        {
            anchors.fill: parent
            color: "#3600C9"
        }
        Rectangle {
            anchors.fill: parent
            color: "#00000000"

            Image {
                id: image_logo
                width: dp(58)
                height: dp(29)
                anchors.top: parent.top
                anchors.topMargin: dp(70)
                anchors.horizontalCenter: parent.horizontalCenter
                source: "../img/TBLogo@2x.svg"
                fillMode: Image.PreserveAspectFit
            }

            ListView {
                anchors.topMargin: dp(240)
                anchors.fill: parent

                delegate: Item {
                    height: dp(60)
                    anchors.left: parent.left
                    anchors.right: parent.right

                    Rectangle {
                        anchors.fill: parent
                        color: "#00000000"

                        Image {
                            width: dp(32)
                            height: dp(32)
                            anchors.left: parent.left
                            anchors.leftMargin: dp(60)
                            anchors.verticalCenter: parent.verticalCenter
                            source: imagePath
                            fillMode: Image.PreserveAspectFit
                        }

                        Text {
                            text: pageName
                            anchors.left: parent.left
                            anchors.leftMargin: dp(120)
                            anchors.verticalCenter: parent.verticalCenter
                            font.pixelSize: dp(18)
                            color: "white"
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                switch (index) {
                                case 0:
                                    stateMachine.setActionWindow(21)    // Wallet Page
                                    break
                                case 1:
                                    stateMachine.setActionWindow(22)    // Account Options Page
                                    break
                                case 2:
                                    stateMachine.setActionWindow(23)    // Settings Page
                                    break
                                }
                                nav.toggle()
                            }
                        }
                    }
                }
                model: navModel
            }

//            Text {
//                id: text_accounts
//                text: qsTr("Accounts")
//                color: "white"
//                anchors.left: parent.left
//                anchors.leftMargin: dp(35)
//                anchors.bottom: accountComboBox.top
//                anchors.bottomMargin: dp(10)
//                font.pixelSize: dp(12)
//            }

//            ComboBox {
//                id: accountComboBox

//                onCurrentIndexChanged: {
//                    if (accountComboBox.currentIndex >= 0) {
//                        const selectedAccount = accountItems.get(accountComboBox.currentIndex).account
//                        wallet.switchAccount(selectedAccount)
//                        updateInstanceAccountText()
//                    }
//                }

//                delegate: ItemDelegate {
//                    width: accountComboBox.width
//                    contentItem: Text {
//                        text: info
//                        color: "white"
//                        font: accountComboBox.font
//                        elide: Text.ElideRight
//                        verticalAlignment: Text.AlignVCenter
//                    }
//                    background: Rectangle {
//                        color: accountComboBox.highlightedIndex === index ? "#955BDD" : "#8633E0"
//                    }
//                    topPadding: dp(10)
//                    bottomPadding: dp(10)
//                    leftPadding: dp(20)
//                    rightPadding: dp(20)
//                }

//                indicator: Canvas {
//                    id: canvas
//                    x: accountComboBox.width - width - accountComboBox.rightPadding
//                    y: accountComboBox.topPadding + (accountComboBox.availableHeight - height) / 2
//                    width: dp(14)
//                    height: dp(7)
//                    contextType: "2d"

//                    Connections {
//                        target: accountComboBox
//                        function onPressedChanged() { canvas.requestPaint() }
//                    }

//                    onPaint: {
//                        context.reset()
//                        if (accountComboBox.popup.visible) {
//                            context.moveTo(0, height)
//                            context.lineTo(width / 2, 0)
//                            context.lineTo(width, height)
//                        } else {
//                            context.moveTo(0, 0)
//                            context.lineTo(width / 2, height)
//                            context.lineTo(width, 0)
//                        }
//                        context.strokeStyle = "white"
//                        context.lineWidth = 2
//                        context.stroke()
//                    }
//                }

//                contentItem: Text {
//                    text: accountComboBox.currentIndex >= 0 && accountItems.get(accountComboBox.currentIndex).info
//                    font: accountComboBox.font
//                    color: "white"
//                    verticalAlignment: Text.AlignVCenter
//                    horizontalAlignment: Text.AlignHCenter
//                    elide: Text.ElideRight
//                }

//                background: Rectangle {
//                    implicitHeight: dp(50)
//                    radius: dp(5)
//                    color: "#8633E0"
//                }

//                popup: Popup {
//                    y: accountComboBox.height + dp(3)
//                    width: accountComboBox.width
//                    implicitHeight: contentItem.implicitHeight + dp(20)
//                    topPadding: dp(10)
//                    bottomPadding: dp(10)
//                    leftPadding: dp(0)
//                    rightPadding: dp(0)

//                    contentItem: ListView {
//                        clip: true
//                        implicitHeight: contentHeight
//                        model: accountComboBox.popup.visible ? accountComboBox.delegateModel : null
//                        currentIndex: accountComboBox.highlightedIndex

//                        ScrollIndicator.vertical: ScrollIndicator { }
//                    }

//                    background: Rectangle {
//                        color: "#8633E0"
//                        radius: dp(5)
//                    }

//                    onVisibleChanged: {
//                        if (!accountComboBox.popup.visible) {
//                            canvas.requestPaint()
//                        }
//                    }
//                }

//                model: ListModel {
//                    id: accountItems
//                }
//                anchors.bottom: button_changeinstance.top
//                anchors.bottomMargin: dp(55)
//                anchors.right: parent.right
//                anchors.rightMargin: dp(35)
//                anchors.left: parent.left
//                anchors.leftMargin: dp(35)
//                leftPadding: dp(20)
//                rightPadding: dp(20)
//                font.pixelSize: dp(18)
//            }

            Button {
                id: button_changeinstance
                height: dp(50)
                anchors.right: parent.right
                anchors.rightMargin: dp(35)
                anchors.left: parent.left
                anchors.leftMargin: dp(35)
                anchors.top: parent.bottom
                anchors.topMargin: dp(-100)
                background: Rectangle {
                    id: rectangle
                    color: "#00000000"
                    radius: dp(4)
                    border.color: "white"
                    border.width: dp(2)
                    Text {
                        id: loginText
                        text: qsTr("Logout / Change Wallet")
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: dp(18)
                        color: "white"
                    }
                }

                onClicked: {
                    nav.toggle()
                    messagebox.open(qsTr("LOGOUT / CHANGE WALLET"), qsTr("Are you sure you want to logout?"), true, "No", "Yes", "", "", "", changeInstanceCallback)
                }
            }
        }
    }

    ListModel {
        id: navModel
        ListElement {
            pageName: "Wallet"
            imagePath: "../img/NavWallet@2x.svg"
        }
        ListElement {
            pageName: "Account Options"
            imagePath: "../img/NavAccount@2x.svg"
        }
        ListElement {
            pageName: "Settings"
            imagePath: "../img/NavSettings@2x.svg"
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:640;width:320}D{i:43;anchors_x:38}D{i:44;anchors_x:116}
}
##^##*/
