import QtQuick 2.0
import QtQuick.Window 2.0

Item {
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160); }

    Image {
        id: image_send
        width: dp(60)
        height: dp(75)
        anchors.bottom: image_finalize.top
        anchors.bottomMargin: dp(80)
        anchors.horizontalCenterOffset: dp(-60)
        anchors.horizontalCenter: parent.horizontalCenter
        source: "../img/Send_big@2x.svg"
        fillMode: Image.PreserveAspectFit

        MouseArea {
            anchors.fill: parent
            onClicked: {
                console.log("Wallet => Send")
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
        anchors.horizontalCenterOffset: dp(60)
        anchors.horizontalCenter: parent.horizontalCenter
        source: "../img/Receive_big@2x.svg"
        fillMode: Image.PreserveAspectFit

        MouseArea {
            anchors.fill: parent
            onClicked: {
                console.log("Wallet => Receive")
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
        anchors.verticalCenterOffset: dp(-20)
        anchors.horizontalCenterOffset: dp(-60)
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
        anchors.horizontalCenterOffset: dp(60)
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

    Rectangle {
        id: splitter
        height: 2
        color: "#ffffff"
        anchors.right: parent.right
        anchors.rightMargin: dp(65)
        anchors.left: parent.left
        anchors.leftMargin: dp(65)
        anchors.top: text_finalize.bottom
        anchors.topMargin: dp(55)
    }

    Image {
        id: image_hodl
        width: dp(60)
        height: dp(75)
        anchors.top: splitter.bottom
        anchors.topMargin: dp(55)
        anchors.horizontalCenterOffset: dp(-60)
        anchors.horizontalCenter: parent.horizontalCenter
        source: "../img/HODL_big@2x.svg"
        fillMode: Image.PreserveAspectFit

        MouseArea {
            anchors.fill: parent
            onClicked: {
                console.log("Wallet => Hodl")
            }
        }
    }

    Text {
        id: text_hodl
        color: "white"
        text: qsTr("HODL")
        anchors.top: image_hodl.bottom
        anchors.topMargin: dp(9)
        anchors.horizontalCenter: image_hodl.horizontalCenter
        font.pixelSize: dp(17)
    }

    Image {
        id: image_airdrop
        width: dp(60)
        height: dp(75)
        anchors.verticalCenter: image_hodl.verticalCenter
        anchors.horizontalCenterOffset: dp(60)
        anchors.horizontalCenter: parent.horizontalCenter
        source: "../img/Airdrop_big@2x.svg"
        fillMode: Image.PreserveAspectFit

        MouseArea {
            anchors.fill: parent
            onClicked: {
                console.log("Wallet => Airdrop")
            }
        }
    }

    Text {
        id: text_airdrop
        color: "white"
        text: qsTr("Airdrop")
        anchors.top: image_airdrop.bottom
        anchors.topMargin: dp(9)
        anchors.horizontalCenter: image_airdrop.horizontalCenter
        font.pixelSize: dp(17)
    }
}
