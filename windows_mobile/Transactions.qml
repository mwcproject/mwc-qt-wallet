import QtQuick 2.0
import QtQuick.Window 2.12

Item {
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    ListModel {
        id: transactionModel
        ListElement {
            txType: "Sent"
            txTime: "Jun 25, 2020  /  12:57pm"
            txBalance: "-90 MWC"
            txId: "ID: 49570294750498750249875049875"
            txUrl: "https://tokok.co/coin/mwc/7295hghhgh4bdfb654"
        }
        ListElement {
            txType: "Received"
            txTime: "Jan 2, 2020  /  2:07am"
            txBalance: "+100,000 MWC"
            txId: "ID: 98750249875049495702947504875"
            txUrl: "File Transfer"
        }
        ListElement {
            txType: "Received"
            txTime: "Dec 25, 2019  /  2:07pm"
            txBalance: "+100,000 MWC"
            txId: "ID: 02947504987495750249875049875"
            txUrl: "https://whitebit.com/coin/mwc/7295hghhgh4bdfb654"
        }
        ListElement {
            txType: "Sent"
            txTime: "Jun 25, 2020  /  12:57pm"
            txBalance: "-90 MWC"
            txId: "ID: 49570294750498750249875049875"
            txUrl: "https://tokok.co/coin/mwc/7295hghhgh4bdfb654"
        }
        ListElement {
            txType: "Received"
            txTime: "Jan 2, 2020  /  2:07am"
            txBalance: "+100,000 MWC"
            txId: "ID: 98750249875049495702947504875"
            txUrl: "File Transfer"
        }
        ListElement {
            txType: "Received"
            txTime: "Dec 25, 2019  /  2:07pm"
            txBalance: "+100,000 MWC"
            txId: "ID: 02947504987495750249875049875"
            txUrl: "https://whitebit.com/coin/mwc/7295hghhgh4bdfb654"
        }
        ListElement {
            txType: "Sent"
            txTime: "Jun 25, 2020  /  12:57pm"
            txBalance: "-90 MWC"
            txId: "ID: 49570294750498750249875049875"
            txUrl: "https://tokok.co/coin/mwc/7295hghhgh4bdfb654"
        }
        ListElement {
            txType: "Received"
            txTime: "Jan 2, 2020  /  2:07am"
            txBalance: "+100,000 MWC"
            txId: "ID: 98750249875049495702947504875"
            txUrl: "File Transfer"
        }
        ListElement {
            txType: "Received"
            txTime: "Dec 25, 2019  /  2:07pm"
            txBalance: "+100,000 MWC"
            txId: "ID: 02947504987495750249875049875"
            txUrl: "https://whitebit.com/coin/mwc/7295hghhgh4bdfb654"
        }
    }

    ListView {
        anchors.fill: parent
        anchors.top: parent.top
        model: transactionModel
        delegate: transactionDelegate
        focus: true
    }

    Component {
        id: transactionDelegate
        Rectangle {
            height: dp(215)
            color: "#00000000"
            anchors.left: parent.left
            anchors.right: parent.right

            Rectangle {
                height: dp(200)
                color: "#33bf84ff"
                anchors.top: parent.top
                anchors.topMargin: dp(15)
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                anchors.left: parent.left
                anchors.leftMargin: dp(20)

                Image {
                    width: dp(17)
                    height: dp(17)
                    anchors.top: parent.top
                    anchors.topMargin: dp(35)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    fillMode: Image.PreserveAspectFit
                    source: txType === "Sent" ? "../img/Transactions_Sent@2x.svg" : "../img/Transactions_Received@2x.svg"
                }

                Text {
                    color: "#ffffff"
                    text: txType
                    font.bold: true
                    font.pixelSize: dp(15)
                    anchors.top: parent.top
                    anchors.topMargin: dp(37)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(71)
                }

                Text {
                    width: dp(200)
                    height: dp(15)
                    color: "#bf84ff"
                    text: txTime
                    horizontalAlignment: Text.AlignRight
                    font.pixelSize: dp(15)
                    anchors.top: parent.top
                    anchors.topMargin: dp(37)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(35)
                }

                Rectangle {
                    height: dp(1)
                    color: "#ffffff"
                    anchors.top: parent.top
                    anchors.topMargin: dp(70)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(35)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                }

                Text {
                    color: "#ffffff"
                    text: txBalance
                    font.bold: true
                    anchors.top: parent.top
                    anchors.topMargin: dp(90)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    font.pixelSize: dp(15)
                }

                Text {
                    color: "#ffffff"
                    text: txId
                    anchors.top: parent.top
                    anchors.topMargin: dp(120)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    font.pixelSize: dp(15)
                }

                Text {
                    color: "#ffffff"
                    text: txUrl
                    anchors.top: parent.top
                    anchors.topMargin: dp(150)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(35)
                    font.pixelSize: dp(15)
                }
            }
        }
    }
}
