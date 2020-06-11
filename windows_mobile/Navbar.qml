import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
//import NavbarBridge 1.0

Item {
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160); }

//    NavbarBridge {
//        id: bridge
//    }

//    Connections {
//        target: bridge
//        onWalletBalanceUpdated: {
//            balanceText.text = walletBalance
//        }
//    }

    Rectangle {
        id: menuRect
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: dp(70)
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
            id: title
            color: "white"
            text: qsTr("INSTANCE:  Default  //  ACCOUNT:  SatoshisDream")
            anchors.horizontalCenterOffset: dp(20)
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
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
//                                switch (index) {
//                                case 0:
//                                    bridge.clickWalletTab()
//                                    break
//                                case 1:
//                                    bridge.clickAccountTab()
//                                    break
//                                case 2:
//                                    bridge.clickSettingsTab()
//                                    break
//                                }
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
                        function onPressedChanged() { canvas.requestPaint(); }
                    }

                    onPaint: {
                        context.reset();
                        context.moveTo(0, 0);
                        context.lineTo(width / 2, height);
                        context.lineTo(width, 0);
                        context.strokeStyle = "white"
                        context.stroke();
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
                    console.log("Change instance")
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
}
