import QtQuick 2.15
import QtQuick.Window 2.0
import QtQuick.Layouts 1.3
import QtCharts 2.15
import StateMachineBridge 1.0
import WalletBridge 1.0
import QtGraphicalEffects 1.15
import WalletConfigBridge 1.0
import "./models"

Item {


    StateMachineBridge {
        id: stateMachine
    }

    WalletBridge {
        id: wallet
    }

    WalletConfigBridge {
        id: walletConfig
    }

    Connections {
        target: wallet

        onSgnMwcAddressWithIndex: {
            updateStatus()
        }

        onSgnTorAddress: {
            updateStatus()
        }

        onSgnUpdateListenerStatus: {
            updateStatus()
        }
    }

    onVisibleChanged: {
        if(visible) {
            transactionsItem.requestTransactions()
            transactionsItem.updateData()
            receiveItem.requestAddress()
        }
    }

    function updateStatus() {
        image_mwcmqs.source = wallet.getMqsListenerStatus() ? "../img/CircGreen@2x.svg" : "../img/CircRed@2x.svg"
        image_http.source = wallet.getHttpListeningStatus() === "true" ? "../img/CircGreen@2x.svg" : "../img/CircRed@2x.svg"
        image_tor.source = wallet.getTorListenerStatus() ? "../img/CircGreen@2x.svg" : "../img/CircRed@2x.svg"
    }



    // CONFIRMED ACCOUNT BALANCE
    Text {
        id: text_account
        text: qsTr("current account value")
        color: "#c4c4c4"
        font.pixelSize: dp(13)
        font.letterSpacing: dp(0.5)
        anchors.top: parent.top
        anchors.topMargin: dp(10)
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Text {
        id: text_spend_balance
        text: (hiddenAmount? hidden : spendableBalance)+ " MWC"
        color: "white"
        font.family: barlow.bold
        font.pixelSize: dp(25)
        //font.letterSpacing: dp(1)
        anchors.top: text_account.bottom
        anchors.topMargin: dp(3)
        anchors.horizontalCenter: parent.horizontalCenter
    }
    Text {
        id: text_secondary_currency
        text: (hiddenAmount? hidden : spendableBalance) + " USD"
        color: "#c4c4c4"
        font.pixelSize: dp(13)
        font.letterSpacing: dp(1)
        anchors.top: text_spend_balance.bottom
        anchors.topMargin: dp(2)
        anchors.horizontalCenter: parent.horizontalCenter
    }

    // AWAITING BALANCE
    Rectangle {
        id: rec_awaiting
        width: parent.width/1.2
        height: dp(40)
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0
                color: "#531d55"
            }
            GradientStop {
                position: 1
                color: "#202020"
            }
        }
        radius: dp(15)
        visible: awaitBalance !== 0? true : false
        anchors.top: text_secondary_currency.bottom
        anchors.topMargin: dp(15)
        anchors.horizontalCenter: parent.horizontalCenter

        Image {
            id: img_await
            height: rec_awaiting.height/2
            source:"../img/hourglass.svg"
            fillMode: Image.PreserveAspectFit
            //anchors.bottom: rec_awaiting.bottom
            anchors.left: rec_awaiting.left
            anchors.leftMargin: dp(10)
            anchors.verticalCenter: parent.verticalCenter
        }
        ColorOverlay {
               anchors.fill: img_await
               source: img_await
               color: "#ffffff"
           }
        Text {
            id: text_await_balance
            text: qsTr(awaitBalance + " MWC")
            color: "white"
            font.pixelSize: dp(15)
            font.italic: true
            font.weight: Font.Light
            anchors.left: img_await.right
            anchors.leftMargin: dp(7)
            anchors.verticalCenter: parent.verticalCenter
        }

    }

    // LOCKED BALANCE
    Rectangle {
        id: rec_locked
        width: parent.width/1.2
        height: dp(40)
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0
                color: "#1f1d55"
            }
            GradientStop {
                position: 1
                color: "#202020"
            }
        }
        radius: dp(15)
        visible: lockedBalance !== 0? true : false
        anchors.top: rec_locked.visible? rec_awaiting.bottom : text_secondary_currency.bottom
        anchors.topMargin: dp(15)
        anchors.horizontalCenter: parent.horizontalCenter

        Image {
            id: img_locked
            height: rec_locked.height/2
            source:"../img/lock.svg"
            fillMode: Image.PreserveAspectFit
            anchors.left: rec_locked.left
            anchors.leftMargin: dp(10)
            anchors.verticalCenter: parent.verticalCenter
        }
        ColorOverlay {
               anchors.fill: img_locked
               source: img_locked
               color: "#ffffff"
           }
        Text {
            text: qsTr(lockedBalance + " MWC")
            color: "white"
            font.pixelSize: dp(15)
            font.italic: true
            font.weight: Font.Light
            visible: lockedBalance !== 0? true : false
            anchors.left: img_locked.right
            anchors.leftMargin: dp(7)
            anchors.verticalCenter: parent.verticalCenter
        }
    }



    // MAIN NAV BUTTON (SEND-RECEIVE-FINALIZE)
    Rectangle {
        id: navbarButton
        height: parent.width/7
        width: parent.width/1.25
        anchors.top: rec_locked.visible? rec_locked.bottom : (rec_awaiting.visible? rec_awaiting.bottom : text_secondary_currency.bottom)
        anchors.topMargin: dp(20)
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#00000000"

        RowLayout {
            id: layoutNavBottom
            spacing: 0
            Rectangle {
                color: "#00000000"
                Layout.fillHeight: true
                Layout.minimumHeight: parent.parent.height
                Layout.minimumWidth: parent.parent.width/3
                Rectangle {
                    id: rec_send
                    height: parent.height
                    width:  height
                    color: "#353237"
                    radius: dp(50)
                    anchors.horizontalCenter: parent.horizontalCenter
                    ImageColor {
                        id: img_send
                        img_height: rec_send.height/1.7
                        img_source:"../../img/arrow.svg"
                        img_rotation: 0
                        img_color: "#ffffff"
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {

                            stateMachine.setActionWindow(8)
                        }
                    }
                }
                Text {
                    id: text_send
                    text: qsTr("Send")
                    color: "#ffffff"
                    font.pixelSize: dp(12)
                    font.letterSpacing: dp(0.5)
                    anchors.top: rec_send.bottom
                    anchors.topMargin: dp(5)
                    anchors.horizontalCenter: parent.horizontalCenter
                }


            }
            Rectangle {
                color: "#00000000"
                Layout.fillHeight: true
                Layout.minimumHeight: parent.parent.height
                Layout.minimumWidth: parent.parent.width/3
                Rectangle {
                    id: rec_receive
                    height: parent.height
                    width:  height
                    color: "#6b69ee"
                    radius: dp(50)
                    anchors.horizontalCenter: parent.horizontalCenter
                    ImageColor {
                        id: img_receive
                        img_height: rec_receive.height/1.7
                        img_source:"../../img/arrow.svg"
                        img_rotation: -90
                        img_color: "white"
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            stateMachine.setActionWindow(9)
                        }
                    }
                }
                Text {
                    id: text_receive
                    text: qsTr("Receive")
                    color: "#ffffff"
                    font.pixelSize: dp(12)
                    font.letterSpacing: dp(0.5)
                    anchors.top: rec_receive.bottom
                    anchors.topMargin: dp(5)
                    anchors.horizontalCenter: parent.horizontalCenter
                }


            }
            Rectangle {
                color: "#00000000"
                Layout.fillHeight: true
                Layout.minimumHeight: parent.parent.height
                Layout.minimumWidth: parent.parent.width/3
                Rectangle {
                    id: rec_finalize
                    height: parent.height
                    width:  height
                    color: "#353237"
                    radius: dp(50)
                    anchors.horizontalCenter: parent.horizontalCenter

                    ImageColor {
                        id: img_finalize
                        img_height: rec_finalize.height/1.7
                        img_source:"../../img/arrow.svg"
                        img_rotation: -90
                        img_color: "#ffffff"
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            stateMachine.setActionWindow(19)
                        }
                    }
                }

                Text {
                    id: text_finalize
                    text: qsTr("Finalize")
                    color: "#ffffff"
                    font.pixelSize: dp(12)
                    font.letterSpacing: dp(0.5)
                    anchors.top: rec_finalize.bottom
                    anchors.topMargin: dp(5)
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }
}


/*##^##
Designer {
    D{i:0;autoSize:true;height:640;width:0}
}
##^##*/
