import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import ConfigBridge 1.0

Item {
    id: outputDetail

    property string blockExplorerUrl
    property string outputCommitment

    ConfigBridge {
        id: config
    }

    function open(output, outputNote) {
        blockExplorerUrl = config.getBlockExplorerUrl(config.getNetwork());
        text_status.text = output.outputStatus
        image_status.source = getOutputTypeIcon(output.outputStatus, output.coinbase)
        text_mwc.text = output.valueNano
        text_transaction_num.text = output.txIdx
        text_commitment.text = output.outputCommitment
        text_height.text = output.blockHeight
        text_locked.text = output.lockedUntil || "0"
        text_confirms.text = output.numOfConfirms
        text_coinbase.text = output.coinbase
        outputCommitment = output.outputCommitment
        outputDetail.visible = true
    }

    function getOutputTypeIcon(outputStatus, coinbase) {
        switch (outputStatus) {
            case "Unconfirmed":
                return "../img/Transactions_Unconfirmed_Blue@2x.svg"
            case "Unspent":
                if (coinbase) {
                    return "../img/Transactions_CoinBase_Blue@2x.svg"
                }
                return "../img/Outputs_Unspent_Blue@2x.svg"
            case "Locked":
                return "../img/iconLock_Blue@2x.svg"
            case "Spent":
                return "../img/Outputs_Spent_Blue@2x.svg"
        }
    }

    onVisibleChanged: {
        if (visible) {
            view_info.contentItem.contentY = 0
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "white"

        Rectangle {
            id: rect_header
            height: dp(110)
            color: "#ffffff"
            anchors.top: parent.top
            anchors.topMargin: 0
            anchors.right: parent.right
            anchors.rightMargin: 0
            anchors.left: parent.left
            anchors.leftMargin: 0

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    textfield_note.focus = false
                }
            }

            Image {
                id: image_status
                width: dp(20)
                height: dp(20)
                anchors.verticalCenter: text_status.verticalCenter
                anchors.right: text_status.left
                anchors.rightMargin: dp(5)
                source: "../img/Outputs_Unspent@2x.svg"
                fillMode: Image.PreserveAspectFit
            }

            Text {
                id: text_status
                color: "#3600c9"
                text: qsTr("Unspent")
                anchors.top: parent.top
                anchors.topMargin: dp(25)
                anchors.horizontalCenter: parent.horizontalCenter
                font.bold: true
                font.pixelSize: dp(20)
            }

            Text {
                id: text_mwc
                color: "#3600c9"
                text: qsTr("123 MWC")
                anchors.top: text_status.bottom
                anchors.topMargin: dp(10)
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(20)
            }

            Image {
                id: image_close
                width: dp(38)
                height: dp(38)
                anchors.top: parent.top
                anchors.topMargin: dp(25)
                anchors.right: parent.right
                anchors.rightMargin: dp(25)
                source: "../img/MessageBox_Close@2x.svg"
                fillMode: Image.PreserveAspectFit

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        outputDetail.visible = false
                    }
                }
            }
        }

        ScrollView {
            id: view_info
            clip: true
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            contentHeight: dp(540)
            anchors.top: rect_header.bottom
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.left: parent.left

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    textfield_note.focus = false
                }
            }

            Text {
                id: label_transaction_num
                color: "#3600c9"
                text: qsTr("Transaction #")
                font.bold: true
                anchors.left: parent.left
                anchors.leftMargin: dp(20)
                anchors.top: parent.top
                anchors.topMargin: 0
                font.pixelSize: dp(15)
            }

            Text {
                id: text_transaction_num
                color: "#3600c9"
                text: qsTr("65")
                anchors.left: parent.left
                anchors.leftMargin: dp(20)
                anchors.top: label_transaction_num.bottom
                anchors.topMargin: dp(3)
                font.pixelSize: dp(15)
            }

            Text {
                id: label_commitment
                color: "#3600c9"
                text: qsTr("Commitment")
                anchors.left: parent.left
                anchors.topMargin: dp(20)
                font.bold: true
                anchors.top: text_transaction_num.bottom
                font.pixelSize: dp(15)
                anchors.leftMargin: dp(20)
            }

            Text {
                id: text_commitment
                color: "#3600c9"
                text: qsTr("49570294750498750249875049875")
                elide: Text.ElideRight
                anchors.right: parent.right
                anchors.rightMargin: dp(110)
                anchors.left: parent.left
                anchors.topMargin: dp(3)
                anchors.top: label_commitment.bottom
                anchors.leftMargin: dp(20)
                font.pixelSize: dp(15)
            }

            Button {
                id: button_commitment_view
                height: dp(35)
                width: dp(80)
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                anchors.bottom: text_commitment.bottom
                background: Rectangle {
                    color: "#00000000"
                    radius: dp(4)
                    border.color: "#3600C9"
                    border.width: dp(2)
                    Text {
                        text: qsTr("View")
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: dp(14)
                        color: "#3600C9"
                    }
                }

                onClicked: {
                    Qt.openUrlExternally("https://" + blockExplorerUrl + "/#k" + text_commitment.text)
                }
            }

            Text {
                id: label_height
                color: "#3600c9"
                text: qsTr("Height")
                anchors.left: parent.left
                anchors.topMargin: dp(20)
                font.bold: true
                anchors.top: text_commitment.bottom
                anchors.leftMargin: dp(20)
                font.pixelSize: dp(15)
            }

            Text {
                id: text_height
                color: "#3600c9"
                text: qsTr("474524")
                anchors.left: parent.left
                anchors.topMargin: dp(3)
                anchors.top: label_height.bottom
                anchors.leftMargin: dp(20)
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                font.pixelSize: dp(15)
            }

            Text {
                id: label_locked
                color: "#3600c9"
                text: qsTr("Locked Until")
                anchors.left: parent.left
                anchors.topMargin: dp(20)
                font.bold: true
                anchors.top: text_height.bottom
                anchors.leftMargin: dp(20)
                font.pixelSize: dp(15)
            }

            Text {
                id: text_locked
                color: "#3600c9"
                text: qsTr("0")
                anchors.left: parent.left
                anchors.topMargin: dp(3)
                anchors.top: label_locked.bottom
                anchors.leftMargin: dp(20)
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                font.pixelSize: dp(15)
            }

            Text {
                id: label_confirms
                color: "#3600c9"
                text: qsTr("Confirms")
                anchors.left: parent.left
                anchors.topMargin: dp(20)
                font.bold: true
                anchors.top: text_locked.bottom
                anchors.leftMargin: dp(20)
                font.pixelSize: dp(15)
            }

            Text {
                id: text_confirms
                color: "#3600c9"
                text: qsTr("0")
                anchors.left: parent.left
                anchors.topMargin: dp(3)
                anchors.top: label_confirms.bottom
                anchors.leftMargin: dp(20)
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                font.pixelSize: dp(15)
            }

            Text {
                id: label_coinbase
                color: "#3600c9"
                text: qsTr("Coinbase")
                anchors.left: parent.left
                anchors.topMargin: dp(20)
                font.bold: true
                anchors.top: text_confirms.bottom
                anchors.leftMargin: dp(20)
                font.pixelSize: dp(15)
            }

            Text {
                id: text_coinbase
                color: "#3600c9"
                text: qsTr("0")
                anchors.left: parent.left
                anchors.topMargin: dp(3)
                anchors.top: label_coinbase.bottom
                anchors.leftMargin: dp(20)
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                font.pixelSize: dp(15)
            }

            Text {
                id: label_note
                color: "#3600c9"
                text: qsTr("Output Note")
                anchors.left: parent.left
                anchors.topMargin: dp(20)
                font.bold: true
                anchors.top: text_coinbase.bottom
                anchors.leftMargin: dp(20)
                font.pixelSize: dp(15)
            }

            Text {
                id: label_note_addition
                color: "#3600c9"
                text: qsTr("Notes are private and saved locally, only visible to you.")
                anchors.left: parent.left
                anchors.topMargin: dp(3)
                anchors.top: label_note.bottom
                font.pixelSize: dp(15)
                anchors.leftMargin: dp(20)
            }

            TextField {
                id: textfield_note
                height: dp(44)
                padding: dp(10)
                leftPadding: dp(20)
                font.pixelSize: dp(15)
                placeholderText: qsTr("note")
                color: "#3600C9"
                text: ""
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                anchors.left: parent.left
                anchors.leftMargin: dp(20)
                anchors.top: label_note_addition.bottom
                anchors.topMargin: dp(10)
                horizontalAlignment: Text.AlignLeft


                background: Rectangle {
                    color: "white"
                    border.color: "#E2CCF7"
                    border.width: dp(2)
                    radius: dp(4)
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        textfield_note.forceActiveFocus()
                    }
                }
            }

            Button {
                id: button_ok
                height: dp(40)
                width: dp(135)
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: textfield_note.bottom
                anchors.topMargin: dp(30)
                background: Rectangle {
                    color: "#6F00D6"
                    radius: dp(4)
                    Text {
                        text: qsTr("OK")
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: dp(15)
                        color: "white"
                    }
                }

                onClicked: {
                    if (textfield_note.text === "") {
                        config.deleteOutputNote(outputCommitment)
                    } else {
                        config.updateOutputNote(outputCommitment, textfield_note.text)
                    }
                    outputDetail.visible = false
                }
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:700;width:400}D{i:5;anchors_y:63}D{i:2;anchors_width:200;anchors_x:56;anchors_y:71}
D{i:6;anchors_height:200;anchors_width:200;anchors_x:100;anchors_y:290}
}
##^##*/
