import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
import SendBridge 1.0
import UtilBridge 1.0

Item {
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    property string account
    property double amount

    function init(initParams) {
        account = initParams.selectedAccount
        amount = initParams.amount
        text_from_account.text = qsTr("From account: " + account)
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
            if (success) {
//                control::MessageBox::messageText(this, "Success", "Your MWC was successfully sent to recipient");
                console.log("Success", "Your MWC was successfully sent to recipient")
                textfield_send_to.text = ""
                textarea_description.text = ""
                return;
            }
//            control::MessageBox::messageText( this, "Send request failed", message );
            console.log("Send request failed", message)
        }
    }

    onVisibleChanged: {
        if (visible) {
            textfield_api_secret.visible = false
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            textfield_send_to.focus = false
            textarea_description.focus = false
        }
    }

    Image {
        id: image_send_online
        width: dp(100)
        height: dp(100)
        anchors.bottom: textfield_send_to.top
        anchors.bottomMargin: dp(50)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        source: "../img/SendOnline@2x.svg"
        fillMode: Image.PreserveAspectFit
    }

    Text {
        id: text_send_to_address
        color: "#ffffff"
        text: qsTr("Send to Address")
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

    TextField {
        id: textfield_send_to
        height: dp(50)
        padding: dp(5)
        leftPadding: dp(10)
        font.pixelSize: dp(18)
        placeholderText: qsTr("Send to")
        color: "white"
        text: ""
        anchors.bottom: text_formats.top
        anchors.bottomMargin: dp(20)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.right: parent.right
        anchors.rightMargin: dp(90)
        horizontalAlignment: Text.AlignLeft
        background: Rectangle {
            color: "#8633E0"
            radius: dp(5)
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                textfield_send_to.focus = true
            }
        }
        onTextEdited: {
            text_formats.text = "FORMATS:  [mwcmqs://]<mqs_address>  keybase://<name>  http(s)://<host>:<port>"
        }
        onTextChanged: {
            const addrType = util.verifyAddress(textfield_send_to.text)
            if ( addrType === "https" || addrType ==="tor" )
                textfield_api_secret.visible = true
            else
                textfield_api_secret.visible = false
        }
    }

    Button {
        id: button_contacts
        width: dp(50)
        height: dp(50)
        anchors.verticalCenter: textfield_send_to.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("#")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
        }
    }

    Text {
        id: text_formats
        color: "#b3ffffff"
        text: "FORMATS:  [mwcmqs://]<mqs_address>  keybase://<name>  http(s)://<host>:<port>"
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        anchors.bottom: textfield_api_secret.visible ? textfield_api_secret.top : textarea_description.top
        anchors.bottomMargin: dp(30)
        font.pixelSize: dp(13)
    }

    TextField {
        id: textfield_api_secret
        visible: false
        height: dp(50)
        padding: dp(5)
        leftPadding: dp(10)
        font.pixelSize: dp(18)
        placeholderText: qsTr("Recipient HTTP Foreign API secret (optional)")
        color: "white"
        text: ""
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.right: parent.right
        anchors.rightMargin: dp(90)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: textfield_api_secret.height / 2
        horizontalAlignment: Text.AlignLeft
        background: Rectangle {
            color: "#8633E0"
            radius: dp(5)
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                textfield_send_to.focus = true
            }
        }
    }

    TextArea {
        id: textarea_description
        height: textfield_api_secret.visible ? dp(120) : dp(200)
        padding: dp(10)
        font.pixelSize: dp(20)
        placeholderText: qsTr("Description")
        color: "white"
        text: ""
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: textfield_api_secret.visible ? textarea_description.height / 2 + textfield_api_secret.height + dp(30) : textarea_description.height / 2
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
        anchors.topMargin: dp(30)
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

            const sendTo = textfield_send_to.text.trim()

            let valRes = util.validateMwc713Str(sendTo)
            if (valRes !== "") {
//                control::MessageBox::messageText(this, "Incorrect Input", valRes);
                console.log("Incorrect Input", valRes)
                textfield_send_to.focus = true
                return
            }

            if (sendTo.length === 0 ) {
//                control::MessageBox::messageText(this, "Incorrect Input",
//                                             "Please specify a valid address." );
                console.log("Incorrect Input", "Please specify a valid address.")
                textfield_send_to.focus = true
                return
            }

            const description = textarea_description.text.trim();

            valRes = util.validateMwc713Str(description);
            if (valRes !== "") {
//                control::MessageBox::messageText(this, "Incorrect Input", valRes);
                console.log("Incorrect Input", valRes)
                textarea_description.focus = true
                return;
            }

            const apiSecret = textfield_api_secret.text;
            valRes = util.validateMwc713Str(apiSecret);
            if (valRes !== "") {
//                control::MessageBox::messageText(this, "Incorrect Input", valRes);
                console.log("Incorrect Input", valRes)
                textfield_api_secret.focus = true
                return
            }

            if (send.sendMwcOnline(account, Number(amount).toString(), sendTo, apiSecret, description)) {
//                ui->progress->show();
                console.log("Sending is in progress")
            }
        }
    }
}
