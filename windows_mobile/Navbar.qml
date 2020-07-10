import QtQuick 2.12
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
import WalletBridge 1.0
import StateMachineBridge 1.0

Item {
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    function updateTitle(state) {
        switch (state) {
        case 8:
            text_title.text = qsTr("Wallet  >   Send")
            break
        case 9:
            text_title.text = qsTr("Wallet  >   Receive")
            break
        case 11:
            text_title.text = qsTr("Wallet  >   Transactions")
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

    WalletBridge {
        id: wallet
    }

    StateMachineBridge {
        id: stateMachine
    }

    Connections {
        target: wallet
        onSgnWalletBalanceUpdated: {
            text_balance.text = wallet.getTotalMwcAmount() + " MWC"
        }
    }

    Rectangle {
        id: navbarTop
        height: dp(140)
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
            height: dp(60)
            color: "#00000000"

            Rectangle {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                width: dp(70)
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
                    anchors.leftMargin: dp(49)
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
                font.pixelSize: dp(17)
            }
        }

        Rectangle {
            id: rect_splitter
            width: dp(250)
            height: dp(1)
            color: "#ffffff"
            anchors.top: menuRect.bottom
            anchors.topMargin: 0
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Rectangle {
            id: rect_balance
            height: dp(60)
            color: "#00000000"
            anchors.top: menuRect.bottom
            anchors.topMargin: 0
            anchors.right: parent.right
            anchors.rightMargin: 0
            anchors.left: parent.left
            anchors.leftMargin: 0

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

            Image {
                id: image_logo1
                width: dp(58)
                height: dp(29)
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenterOffset: dp(-30)
                anchors.horizontalCenter: parent.horizontalCenter
                fillMode: Image.PreserveAspectFit
                source: "../img/TBLogo@2x.svg"
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
        }

        Rectangle {
            id: rect_listener
            width: dp(95)
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
        }

        Rectangle {
            id: rect_network
            width: dp(84)
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

            Text {
                id: text_accounts
                text: qsTr("Accounts")
                color: "white"
                anchors.left: parent.left
                anchors.leftMargin: dp(35)
                anchors.bottom: accountComboBox.top
                anchors.bottomMargin: dp(10)
                font.pixelSize: dp(12)
            }

            ComboBox {
                id: accountComboBox

                delegate: ItemDelegate {
                    width: accountComboBox.width
                    contentItem: Text {
                        text: modelData
                        color: "#7579ff"
                        font: accountComboBox.font
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                    }
                    highlighted: accountComboBox.highlightedIndex === index
                }

                indicator: Canvas {
                    id: canvas
                    x: accountComboBox.width - width - accountComboBox.rightPadding
                    y: accountComboBox.topPadding + (accountComboBox.availableHeight - height) / 2
                    width: dp(14)
                    height: dp(7)
                    contextType: "2d"

                    Connections {
                        target: accountComboBox
                        function onPressedChanged() { canvas.requestPaint() }
                    }

                    onPaint: {
                        context.reset()
                        context.moveTo(0, 0)
                        context.lineTo(width / 2, height)
                        context.lineTo(width, 0)
                        context.strokeStyle = "white"
                        context.stroke()
                    }
                }

                contentItem: Text {
                    text: accountComboBox.displayText
                    font: accountComboBox.font
                    color: "white"
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                }

                background: Rectangle {
                    implicitHeight: dp(50)
                    radius: dp(5)
                    color: "#8633E0"
                }

                popup: Popup {
                    y: accountComboBox.height - 1
                    width: accountComboBox.width
                    implicitHeight: contentItem.implicitHeight
                    padding: dp(1)

                    contentItem: ListView {
                        clip: true
                        implicitHeight: contentHeight
                        model: accountComboBox.popup.visible ? accountComboBox.delegateModel : null
                        currentIndex: accountComboBox.highlightedIndex

                        ScrollIndicator.vertical: ScrollIndicator { }
                    }

                    background: Rectangle {
                        border.color: "white"
                        radius: dp(3)
                    }
                }

                model: ListModel {
                    id: accountItems
                    ListElement { text: "Default" }
                }
                anchors.bottom: button_changeinstance.top
                anchors.bottomMargin: dp(55)
                anchors.right: parent.right
                anchors.rightMargin: dp(35)
                anchors.left: parent.left
                anchors.leftMargin: dp(35)
                leftPadding: dp(10)
                rightPadding: dp(10)
                font.pixelSize: dp(18)
            }

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
                        text: qsTr("Change Instance")
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: dp(18)
                        color: "white"
                    }
                }

                onClicked: {
                    nav.toggle()
//                    messagebox.open(qsTr("Change Instance"), qsTr("Changing an instance will log you out of this current wallet instance. Are you sure you want to log out?"))
                }
            }

        }
    }

    ListModel {
        id: navModel
        ListElement {
            pageName: "Wallet"
            imagePath: "../img/NavAccount@2x.svg"
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

//    MessageBox {
//        id: messagebox
//        anchors.verticalCenter: parent.verticalCenter
//        anchors.horizontalCenter: parent.horizontalCenter
//    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:640;width:320}D{i:43;anchors_x:38}D{i:44;anchors_x:116}
}
##^##*/
