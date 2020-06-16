import QtQuick 2.0
import QtQuick.Window 2.0
import StateMachineBridge 1.0

Item {
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    StateMachineBridge {
        id: stateMachine
    }

    Image {
        id: image_send
        width: dp(60)
        height: dp(75)
        anchors.bottom: image_finalize.top
        anchors.bottomMargin: dp(80)
        anchors.horizontalCenterOffset: dp(-90)
        anchors.horizontalCenter: parent.horizontalCenter
        source: "../img/Send_big@2x.svg"
        fillMode: Image.PreserveAspectFit

        MouseArea {
            anchors.fill: parent
            onClicked: {
                stateMachine.setActionWindow(8)
            }
        }
    }

    Text {
        id: text_send
        color: "white"
        text: qsTr("Send")
        anchors.top: image_send.bottom
        anchors.topMargin: dp(9)
        anchors.horizontalCenter: image_send.horizontalCenter
        font.pixelSize: dp(17)
    }

    Image {
        id: image_receive
        width: dp(60)
        height: dp(75)
        anchors.verticalCenter: image_send.verticalCenter
        anchors.horizontalCenterOffset: dp(90)
        anchors.horizontalCenter: parent.horizontalCenter
        source: "../img/Receive_big@2x.svg"
        fillMode: Image.PreserveAspectFit

        MouseArea {
            anchors.fill: parent
            onClicked: {
                stateMachine.setActionWindow(9)
            }
        }
    }

    Text {
        id: text_receive
        color: "white"
        text: qsTr("Receive")
        anchors.top: image_receive.bottom
        anchors.topMargin: dp(9)
        anchors.horizontalCenter: image_receive.horizontalCenter
        font.pixelSize: dp(17)
    }

    Image {
        id: image_finalize
        width: dp(60)
        height: dp(75)
        anchors.horizontalCenterOffset: dp(-90)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        source: "../img/FinalizeL@2x.svg"
        fillMode: Image.PreserveAspectFit

        MouseArea {
            anchors.fill: parent
            onClicked: {
                console.log("Wallet => Finalize")
            }
        }
    }

    Text {
        id: text_finalize
        color: "white"
        text: qsTr("Finalize")
        anchors.top: image_finalize.bottom
        anchors.topMargin: dp(9)
        anchors.horizontalCenter: image_finalize.horizontalCenter
        font.pixelSize: dp(17)
    }

    Image {
        id: image_transactions
        width: dp(60)
        height: dp(75)
        anchors.horizontalCenterOffset: dp(90)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: image_finalize.verticalCenter
        source: "../img/Transactions_big@2x.svg"
        fillMode: Image.PreserveAspectFit

        MouseArea {
            anchors.fill: parent
            onClicked: {
                console.log("Wallet => Transactions")
            }
        }
    }

    Text {
        id: text_transactions
        color: "white"
        text: qsTr("Transactions")
        anchors.top: image_transactions.bottom
        anchors.topMargin: dp(9)
        anchors.horizontalCenter: image_transactions.horizontalCenter
        font.pixelSize: dp(17)
    }

    Image {
        id: image_outputs
        width: dp(60)
        height: dp(75)
        anchors.top: text_finalize.bottom
        anchors.topMargin: dp(55)
        anchors.horizontalCenterOffset: dp(-90)
        anchors.horizontalCenter: parent.horizontalCenter
        source: "../img/Output_big@2x.svg"
        fillMode: Image.PreserveAspectFit

        MouseArea {
            anchors.fill: parent
            onClicked: {
                console.log("Wallet => Output")
            }
        }
    }

    Text {
        id: text_outputs
        color: "white"
        text: qsTr("Outputs")
        anchors.top: image_outputs.bottom
        anchors.topMargin: dp(9)
        anchors.horizontalCenter: image_outputs.horizontalCenter
        font.pixelSize: dp(17)
    }

    Image {
        id: image_accounts
        width: dp(60)
        height: dp(75)
        anchors.verticalCenter: image_outputs.verticalCenter
        anchors.horizontalCenterOffset: dp(90)
        anchors.horizontalCenter: parent.horizontalCenter
        source: "../img/Account_big@2x.svg"
        fillMode: Image.PreserveAspectFit

        MouseArea {
            anchors.fill: parent
            onClicked: {
                console.log("Wallet => Accounts")
            }
        }
    }

    Text {
        id: text_accounts
        color: "white"
        text: qsTr("Accounts")
        anchors.top: image_accounts.bottom
        anchors.topMargin: dp(9)
        anchors.horizontalCenter: image_accounts.horizontalCenter
        font.pixelSize: dp(17)
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:640;width:0}
}
##^##*/
