import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
import SendBridge 1.0
import UtilBridge 1.0

Item {
    property string account
    property double amount

    function init(initParams) {
        account = initParams.selectedAccount
        amount = parseInt(initParams.amount)
        text_from_account.text = qsTr("From account: " + account)
        text_amount_to_send.text = qsTr("Amount to send: " + ( amount < 0 ? "All" : util.nano2one(Number(amount).toString())) + " MWC" )
    }

    function onSelectContact(ok, contact) {
        if (ok) {
            textfield_send_to.text = contact.address
            text_formats.visible = false
            text_contact_name.text = "Contact: " + contact.name
            text_contact_name.visible = true
        }
    }

    UtilBridge {
        id: util
    }

    SendBridge {
        id: send
    }

    Connections {
        target: send
        onSgnShowSendResult: (success, message) => {
            rect_progress.visible = false
            if (success) {
                messagebox.open(qsTr("Success"), qsTr("Your MWC was successfully sent to recipient"))
                textfield_send_to.text = ""
                textarea_description.text = ""
            } else {
                messagebox.open(qsTr("Send request failed"), qsTr(message))
            }
        }
    }

    onVisibleChanged: {
        if (visible) {
            if (parent.height > dp(500)) {
                image_send_online.topMargin = (parent.height - dp(500)) / 2
            }
            rect_progress.visible = false
            textfield_api_secret.visible = false
            text_contact_name.text = ""
            text_contact_name.visible = false
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
        anchors.top: parent.top
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
        leftPadding: dp(20)
        font.pixelSize: dp(18)
        placeholderText: qsTr("Send to")
        color: "white"
        text: ""
        anchors.top: image_send_online.bottom
        anchors.topMargin: dp(30)
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
            text_contact_name.text = ""
            text_contact_name.visible = false
            text_formats.visible = true
        }
        onTextChanged: {
//            const addrType = util.verifyAddress(textfield_send_to.text)
//            if ( addrType === "https" || addrType ==="tor" )
//                textfield_api_secret.visible = true
//            else
//                textfield_api_secret.visible = false
        }
    }

    Image {
        width: dp(45)
        height: dp(45)
        anchors.verticalCenter: textfield_send_to.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        fillMode: Image.PreserveAspectFit
        source: "../img/Contact@2x.svg"

        MouseArea {
            anchors.fill: parent
            onClicked: {
                selectContactItem.open(true, true, true, onSelectContact)
            }
        }
    }

    Text {
        id: text_formats
        color: "#b3ffffff"
        text: "FORMATS:  [mwcmqs://]<mqs_address>  http(s)://<host>:<port>"
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        anchors.top: textfield_send_to.bottom
        anchors.topMargin: dp(20)
        font.pixelSize: dp(13)
    }

    Text {
        id: text_contact_name
        color: "white"
        text: ""
        anchors.left: parent.left
        anchors.leftMargin: dp(50)
        anchors.top: textfield_send_to.bottom
        anchors.topMargin: dp(10)
        font.pixelSize: dp(13)
        visible: false
    }

    TextField {
        id: textfield_api_secret
        visible: false
        height: dp(50)
        padding: dp(5)
        leftPadding: dp(20)
        font.pixelSize: dp(18)
        placeholderText: qsTr("Recipient HTTP Foreign API secret (optional)")
        color: "white"
        text: ""
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        anchors.top: textfield_send_to.bottom
        anchors.topMargin: dp(55)
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
        height: textfield_api_secret.visible ? dp(80) : dp(160)
        padding: dp(10)
        font.pixelSize: dp(18)
        placeholderText: qsTr("Description (optional)")
        color: "white"
        text: ""
        anchors.top: textfield_send_to.bottom
        anchors.topMargin: textfield_api_secret.visible ? dp(125) : dp(55)
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
            settingsItem.visible = true
        }
    }

    Button {
        id: button_send
        width: parent.width / 2 - dp(45)
        height: dp(50)
        anchors.verticalCenter: button_settings.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: dp(20)
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
                messagebox.open(qsTr("Unable to send"), qsTr("Your MWC Node, that wallet connected to, is not ready.\nMWC Node needs to be connected to a few peers and finish block synchronization process"))
                return
            }

            const sendTo = textfield_send_to.text.trim()

            let valRes = util.validateMwc713Str(sendTo)
            if (valRes !== "") {
                messagebox.open(qsTr("Incorrect Input"), qsTr(valRes))
                textfield_send_to.focus = true
                return
            }

            if (sendTo.length === 0 ) {
                messagebox.open(qsTr("Incorrect Input"), qsTr("Please specify a valid address."))
                textfield_send_to.focus = true
                return
            }

            const description = textarea_description.text.trim();

            valRes = util.validateMwc713Str(description);
            if (valRes !== "") {
                messagebox.open(qsTr("Incorrect Input"), qsTr(valRes))
                textarea_description.focus = true
                return;
            }

            const apiSecret = textfield_api_secret.text;
            valRes = util.validateMwc713Str(apiSecret);
            if (valRes !== "") {
                messagebox.open(qsTr("Incorrect Input"), qsTr(valRes))
                textfield_api_secret.focus = true
                return
            }

            if (send.sendMwcOnline(account, Number(amount).toString(), sendTo, apiSecret, description)) {
                rect_progress.visible = true
            }
        }
    }

    Rectangle {
        id: rect_progress
        width: dp(60)
        height: dp(30)
        anchors.top: button_send.bottom
        anchors.topMargin: dp(30)
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#00000000"
        visible: false
        AnimatedImage {
            id: animation
            source: "../img/loading.gif"
        }
    }

    SendSettings {
        id: settingsItem
        anchors.verticalCenter: parent.verticalCenter
        visible: false
    }

    SelectContact {
        id: selectContactItem
        anchors.fill: parent
        visible: false
    }
}
