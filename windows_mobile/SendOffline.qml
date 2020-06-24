import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
import SendBridge 1.0
import UtilBridge 1.0

Item {
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    property string selectedAccount
    property double amount

    function init(initParams) {
        selectedAccount = initParams.selectedAccount
        amount = initParams.amount
        text_from_account.text = qsTr("From account: " + selectedAccount)
        text_amount_to_send.text = qsTr("Amount to send: " + ( amount < 0 ? "All" : util.nano2one(Number(amount).toString())) + " MWC" )
    }

    UtilBridge {
        id: util
    }

    SendBridge {
        id: send
    }

    Connections {
        target: send
        onSgnShowSendResult: {
//            ui->progress->hide();
//            control::MessageBox::messageText(this, success ? "Success" : "Failure", message );
            console.log(success ? "Success" : "Failure", message)
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            textarea_description.focus = false
        }
    }

    Image {
        id: image_send_online
        width: dp(100)
        height: dp(100)
        anchors.bottom: textarea_description.top
        anchors.bottomMargin: dp(50)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        source: "../img/File@2x.svg"
        fillMode: Image.PreserveAspectFit
    }

    Text {
        id: text_send_to_address
        color: "#ffffff"
        text: qsTr("Send to File")
        anchors.left: image_send_online.right
        anchors.leftMargin: dp(30)
        anchors.top: image_send_online.top
        anchors.topMargin: dp(10)
        font.bold: true
        font.pixelSize: dp(22)
    }

    Text {
        id: text_from_account
        color: "#ffffff"
        text: qsTr("From account:")
        anchors.bottom: text_amount_to_send.top
        anchors.bottomMargin: dp(5)
        anchors.left: image_send_online.right
        anchors.leftMargin: dp(30)
        font.pixelSize: dp(15)
    }

    Text {
        id: text_amount_to_send
        color: "#ffffff"
        text: qsTr("Amount to send:")
        anchors.left: image_send_online.right
        anchors.leftMargin: dp(30)
        anchors.bottom: image_send_online.bottom
        anchors.bottomMargin: dp(5)
        font.pixelSize: dp(15)
    }

    TextArea {
        id: textarea_description
        height: dp(200)
        padding: dp(10)
        font.pixelSize: dp(20)
        placeholderText: qsTr("Description")
        color: "white"
        text: ""
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        horizontalAlignment: Text.AlignLeft
        background: Rectangle {
            color: "#8633E0"
            radius: dp(5)
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                textarea_description.focus = true
            }
        }
    }

    Button {
        id: button_settings
        width: parent.width / 2 - dp(45)
        height: dp(50)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.top: textarea_description.bottom
        anchors.topMargin: dp(50)
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Settings")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
        }
    }

    Button {
        id: button_send
        width: parent.width / 2 - dp(45)
        height: dp(50)
        anchors.verticalCenter: button_settings.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Send")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
            if ( !send.isNodeHealthy() ) {
//                control::MessageBox::messageText(this, "Unable to send", "Your MWC Node, that wallet connected to, is not ready.\n"
//                                                                             "MWC Node needs to be connected to a few peers and finish block synchronization process");
                console.log("Unable to send", "Your MWC Node, that wallet connected to, is not ready.\nMWC Node needs to be connected to a few peers and finish block synchronization process")
                return
            }

            const description = textarea_description.text.trim();

            const valRes = util.validateMwc713Str(description);
            if (valRes !== "") {
//                control::MessageBox::messageText(this, "Incorrect Input", valRes);
                console.log("Incorrect Input", valRes)
                textarea_description.focus = true
                return;
            }

            if (send.sendMwcOffline(selectedAccount, Number(amount).toString(), description)) {
//                ui->progress->show();
                console.log("Sending is in progress")
            }
        }
    }
}
