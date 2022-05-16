import QtQuick 2.15
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
import WalletBridge 1.0
import SendBridge 1.0
import ConfigBridge 1.0
import UtilBridge 1.0

import "./models"

Item {
    id: root

    property bool showGenProofWarning: false
    property int selectedSendMethod: 1
    property var contactList
    property bool isSlatepack: false
    property string recipientWallet: ""


    WalletBridge {
        id: wallet
    }

    SendBridge {
        id: send
    }

    Connections {
        target: send
        onSgnShowSendResult: (success, message) => {
                                 if (success) {
                                     messagebox.open("Success", "Your MWC was successfully sent to recipient")
                                 } else {
                                     messagebox.open(qsTr("Send request failed"), qsTr(message))
                                 }
                             }

    }

    UtilBridge {
        id: util
    }

    ConfigBridge {
        id: config
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            field_address.field_focus = false
            field_amount.field_focus = false
        }
    }

    function generateProofCallback(ok) {
        if (ok) {
            showGenProofWarning = true
            config.setGenerateProof(true)
        } else {
            checkbox_tx_proof.checked = false
        }
    }

    function updateContactsList() {
        const pairs = config.getContactsAsPairs()
        contactsModel.clear()
        contactList = []
        for (let i = 0; i < pairs.length; i += 2) {
            contactList.push({
                name: pairs[i],
                address: pairs[i+1]
            })
            contactsModel.append(contactList[contactList.length - 1])
        }
    }

    function selectSendMethod(sendMethod) {
        field_address.field_focus = false
        field_amount.field_focus = false
        selectedSendMethod = sendMethod
        switch (sendMethod) {
            case 1: // online_id
                rec_address.color = Theme.field
                rec_file.color = "#00000000"
                rec_slate.color = "#00000000"

                break;
            case 2: // file_id
                rec_address.color = "#00000000"
                rec_file.color = Theme.field
                rec_slate.color = "#00000000"
                break;
            case 3: // slatepack_id
                rec_address.color = "#00000000"
                rec_file.color = "#00000000"
                rec_slate.color = Theme.field
                break;
        }
    }

    function one2nano(str) {
        let amountFloat = parseFloat(str).toFixed(9)
        amountFloat = Math.abs(Math.pow(10, 9) * amountFloat)
        return parseInt(amountFloat)
    }

    function roundN(value) {
       var tenToN = 10 ** 9;
       return Math.round(value * tenToN) / tenToN;
    }

    onVisibleChanged: {
        if (visible) {
            updateContactsList()
            selectSendMethod(config.getSendMethod())
            field_address.text = ""
            field_amount.text = ""
            checkbox_tx_proof.checked = config.getGenerateProof()
        }
    }
    Rectangle {
        anchors.fill: parent
        color: Theme.bg
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            field_amount.field_focus = false
            field_address.field_focus = false
        }
    }



    Rectangle {
        id: rect_sendType
        anchors.top: parent.top
        anchors.topMargin: parent.height/14
        anchors.horizontalCenter: parent.horizontalCenter
        radius: dp(25)
        color: Theme.card
        height: parent.height/6
        width: parent.width/1.15

        Rectangle {
            id: rec_address
            height: parent.height - dp(16)
            width: parent.width/3 - dp(16)
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: dp(8)
            color: Theme.field
            radius: dp(25)
            SendType {
                id: address
                height: parent.height/1.7
                width: parent.width/1.7
                img_height: address.height/2
                img_source: "../../img/online.svg"
                mainText: qsTr("Address")
                secondaryText: qsTr("Automatic")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                img_color: Theme.textPrimary
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    selectSendMethod(1)
                }
            }
        }

        Rectangle {
            id: rec_file
            height: parent.height - dp(16)
            width: parent.width/3 - dp(16)
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#00000000"
            radius: dp(25)
            SendType {
                id: file
                height: parent.height/1.7
                width: parent.width/1.7
                img_height: file.height/2
                img_source: "../../img/file.svg"
                mainText: qsTr("File")
                secondaryText: qsTr("Manual")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                img_color: Theme.textPrimary
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    selectSendMethod(2)
                }
            }
        }

        Rectangle {
            id: rec_slate
            height: parent.height - dp(16)
            width: parent.width/3 - dp(16)
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: dp(8)
            color: "#00000000"
            radius: dp(25)
            SendType {
                id: slate
                height: parent.height/1.7
                width: parent.width/1.7
                img_height: slate.height/2
                img_source: "../../img/slate.svg"
                mainText: qsTr("Slatepack")
                secondaryText: qsTr("Manual")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                img_color: Theme.textPrimary
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    selectSendMethod(3)
                }
            }
        }
    }

    Text {
        id: text_address
        text: selectedSendMethod !== 2? (selectedSendMethod === 1? qsTr("Address") :  qsTr("Address (Optional)")) : ""
        color: Theme.textPrimary
        font.pixelSize: dp(17)
        font.letterSpacing: dp(0.5)
        anchors.top: rect_sendType.bottom
        anchors.topMargin: dp(40)
        anchors.left: field_address.left
        anchors.leftMargin: dp(10)
        visible: selectedSendMethod !== 2? true: false
        //anchors.horizontalCenter: parent.horizontalCenter
    }



    AddressField {
        id: field_address
        height: dp(50)
        width: rect_sendType.width
        anchors.top: text_address.bottom
        anchors.topMargin: dp(10)
        anchors.horizontalCenter: parent.horizontalCenter
        focus: false
        visible: selectedSendMethod !== 2? true: false
        onTextChanged: {
            field_amount.field_focus = false
            let recipientWallet = field_address.text.trim()
            if (!recipientWallet) {
                field_address.mainColor = Theme.field
                return
            }
            let verifyAddress = util.verifyAddress(recipientWallet)
            if (verifyAddress === "unknown") {
                field_address.mainColor = Theme.inputError
            } else {
                field_address.mainColor = Theme.field
            }
        }
        mouseContact.onClicked: {
            if (contactsModel.count === 0) {
                notification.text = qsTr("No contacts saved")
                notification.open()
            }

            contactNav.open()
        }
    }

    ListModel {
        id: contactsModel
    }

    DrawerList {
        id: contactNav
        repeater.model: contactsModel
        repeater.delegate: contactsDelegate
    }

    Component {
        id: contactsDelegate
        Rectangle {
            height: rectangle.height + dp(5)
            color: Theme.field
            width: root.width

            Rectangle {
                id: rectangle
                height: text_address.height + text_name.height + dp(12)
                width: parent.width
                color: "#181818"
                //radius: dp(15)
                anchors.bottom: parent.bottom
                //anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter

                Rectangle {
                    id: initial_logo
                    height: parent.height*0.5
                    width: height
                    radius: dp(150)
                    color: contactsItem.colorProfil(address, true)
                    anchors.left: parent.left
                    anchors.leftMargin: dp(20)
                    anchors.verticalCenter: parent.verticalCenter
                    Text {
                        color: contactsItem.colorProfil(address, false)
                        text: name.charAt(0)
                        //font.bold: true
                        font.family: barlow.light
                        font.pixelSize: initial_logo.height*0.6
                        font.capitalization: Font.AllUppercase
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }


                Text {
                    id: text_name
                    color: Theme.textPrimary
                    text: name
                    font.pixelSize: dp(16)
                    font.bold: true
                    anchors.top: parent.top
                    anchors.topMargin: dp(10)
                    anchors.left: initial_logo.right
                    anchors.leftMargin: dp(20)


                }
                TextEdit {
                    id: text_address
                    readOnly: true
                    font.pixelSize: dp(13)
                    anchors.top: text_name.bottom
                    anchors.topMargin: dp(2)
                    anchors.left: initial_logo.right
                    anchors.leftMargin: dp(20)
                    anchors.right: parent.right
                    anchors.rightMargin: parent.width*0.15
                    wrapMode: Text.WrapAnywhere
                    color: Theme.textPrimary
                    text: address
                }

                Rectangle {
                    id: icon_layout
                    height: parent.height*0.8
                    anchors.verticalCenter: parent.verticalCenter
                    width: rectangle.width/7
                    anchors.right: parent.right
                    color: "#00000000"

                }
                MouseArea {
                    id: m
                    anchors.fill: parent
                    onClicked: {
                        field_address.text = address
                    }
                }
           }
        }
    }


    Text {
        id: text_amount
        text: qsTr("Amount")
        color: Theme.textPrimary
        font.pixelSize: dp(17)
        font.letterSpacing: dp(0.5)
        anchors.top: selectedSendMethod !== 2? field_address.bottom : rect_sendType.bottom
        anchors.topMargin: dp(40)
        anchors.left: field_address.left
        anchors.leftMargin: dp(10)
        //anchors.horizontalCenter: parent.horizontalCenter
    }

    AmountField {
        id: field_amount
        height: dp(90)
        width: rect_sendType.width
        anchors.top: text_amount.bottom
        anchors.topMargin: dp(10)
        anchors.left: field_address.left
        focus: false
        mainColor: Theme.field
        onTextChanged: {
            field_address.field_focus = false
            if (!amountSend) {
                field_amount.mainColor = Theme.field
                return
            }
            if (amountSend > spendableBalance) {
                field_amount.mainColor = Theme.inputError
                return
            } else {
                field_amount.mainColor = Theme.field
            }

        }
    }

    Text {
        id: text_available
        text: qsTr("Available: %1 MWC").arg(spendableBalance)
        color: Theme.textSecondary
        font.pixelSize: dp(12)
        font.letterSpacing: dp(0.5)
        anchors.top: field_amount.bottom
        anchors.topMargin: dp(10)
        anchors.right: field_amount.right
        anchors.rightMargin: dp(10)
        //anchors.horizontalCenter: parent.horizontalCenter
    }

    /*Button {
        id: button_max
        height: field_amount.height
        width: rect_sendType.width/5
        anchors.right: rect_sendType.right
        anchors.leftMargin: dp(10)
        anchors.verticalCenter: field_amount.verticalCenter
        background: Rectangle {
            id: rectangle
            radius: dp(25)
            gradient: Gradient {
                //orientation: Gradient.Horizontal
                GradientStop {
                    position: 0
                    color: "#531d55"
                }
                GradientStop {
                    position: 1
                    color: "#202020"
                }
            }
            Text {
                id: loginText
                text: qsTr("Max")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: Theme.textPrimary
            }
        }
        onClicked:  {
            field_address.field_focus = false
            field_amount.field_focus = false
            const amount = parseFloat(send.getSpendAllAmount(selectedAccount))
            field_amount.text = amount
        }
    }*/

    CheckBox {
        id: checkbox_tx_proof
        text: qsTr("Generate Transaction Proof")
        font.pixelSize: dp(15)
        anchors.left: field_amount.left
        anchors.top: field_amount.bottom
        anchors.topMargin: dp(40)

        indicator: Rectangle {
            implicitWidth: dp(20)
            implicitHeight: dp(20)
            x: checkbox_tx_proof.leftPadding
            y: parent.height / 2 - height / 2
            radius: dp(5)
            color: "#242424"

            ImageColor {
                img_height: dp(17)
                anchors.fill: parent
                img_source: checkbox_tx_proof.checked ?  "../../img/check.svg" : ""
                img_color: Theme.textPrimary
            }
        }

        contentItem: Text {
            text: checkbox_tx_proof.text
            font: checkbox_tx_proof.font
            color: Theme.textPrimary
            verticalAlignment: Text.AlignVCenter
            anchors.left: checkbox_tx_proof.indicator.right
            anchors.leftMargin: dp(15)
        }

        nextCheckState: function() {
                if (checkState === Qt.Checked)
                    return Qt.Unchecked
                else
                    return Qt.Checked
            }

        onCheckStateChanged: {
             if (checkbox_tx_proof.checked && !showGenProofWarning) {
                 messagebox.open("Warning", "Transaction proof generation requires receiver wallet version 1.0.23 or higher.\n\nDo you want to generate proofs for all your send transactions?", true, "Cancel", "Generate", "", "", "", generateProofCallback)
             } else {
                 config.setGenerateProof(checkbox_tx_proof.checked)
             }
        }
    }

    CheckBox {
        id: checkbox_message
        text: qsTr("Include a message for the receiver")
        font.pixelSize: dp(15)
        anchors.left: field_amount.left
        anchors.top: checkbox_tx_proof.bottom
        anchors.topMargin: dp(20)

        indicator: Rectangle {
            implicitWidth: dp(20)
            implicitHeight: dp(20)
            x: checkbox_message.leftPadding
            y: parent.height / 2 - height / 2
            radius: dp(5)
            color: "#242424"

            ImageColor {
                img_height: dp(17)
                anchors.fill: parent
                img_source: checkbox_message.checked ?  "../../img/check.svg" : ""
                img_color: Theme.textPrimary
            }
        }

        contentItem: Text {
            text: checkbox_message.text
            font: checkbox_message.font
            color: Theme.textPrimary
            verticalAlignment: Text.AlignVCenter
            anchors.left: checkbox_message.indicator.right
            anchors.leftMargin: dp(15)
        }

        nextCheckState: function() {
                if (checkState === Qt.Checked)
                    return Qt.Unchecked
                else
                    return Qt.Checked
            }

        onCheckStateChanged: {
            if (checkbox_message.checked) {
                textfield_message.visible = true
            } else {
                textfield_message.visible = false
            }
        }
    }

    TextFieldCursor {
        id: textfield_message
        height: dp(40)
        width: rect_sendType.width
        leftPadding: dp(20)
        font.pixelSize: dp(16)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: checkbox_message.bottom
        anchors.topMargin: dp(20)
        placeholderText: qsTr("Message")
        visible: false
        color: Theme.textSecondary
    }

    ConfirmButton {
        id: button_next
        title: "Send"
        anchors.top: textfield_message.visible === true? textfield_message.bottom : checkbox_message.bottom
        anchors.topMargin: dp(40)
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: {
            field_address.field_focus = false
            field_amount.field_focus = false
            config.setSendMethod(selectedSendMethod)
            const sendAmount = field_amount.amountSend
            const res = send.initialSendSelection(selectedSendMethod, selectedAccount, sendAmount);
            const amount = one2nano((sendAmount))

            if (selectedSendMethod == 1) {
                if (res !== 0) {
                    return
                }
                if ( !send.isNodeHealthy() ) {
                    messagebox.open(qsTr("Unable to send"), qsTr("Your MWC Node, that wallet connected to, is not ready.\nMWC Node needs to be connected to a few peers and finish block synchronization process"))
                    return
                }



                const sendTo = field_address.text

                const isValidAddress = util.verifyAddress(sendTo)
                if (isValidAddress === "unknow") {
                    messagebox.open(qsTr("Invalid Address"), qsTr("Please specify a valid address."))
                    return
                }

                let valRes = util.validateMwc713Str(sendTo)
                if (valRes !== "") {
                    messagebox.open(qsTr("Incorrect Input"), qsTr(valRes))
                    textfield_send_to.focus = true
                    return
                }

                const description = textfield_message.text.split('\n').join('')


                if (!send.sendMwcOnline(selectedAccount, amount, sendTo, "", description)) {
                    //rect_progress.visible = false
                    //button_send.enabled = true
                    return
                }


            } else {
                let isSlatepack = false
                if (selectedSendMethod === 3)
                    isSlatepack = true
                let recipientWallet
                if (isSlatepack) {
                    recipientWallet = field_address.text
                    if (recipientWallet !== "" && util.verifyAddress(recipientWallet) !== "tor") {
                        messagebox.open(qsTr("Unable to send"), qsTr("Please specify valid recipient wallet address"))
                        field_address.focus = true
                        return
                    }
                }

                if ( !send.isNodeHealthy() ) {
                    messagebox.open(qsTr("Unable to send"), qsTr("Your MWC Node, that wallet connected to, is not ready.\nMWC Node needs to be connected to a few peers and finish block synchronization process"))
                    return
                }

                const description = textfield_message.text.trim().replace('\n', ' ')

                const valRes = util.validateMwc713Str(description);
                if (valRes !== "") {
                    messagebox.open("Incorrect Input", valRes)
                    textfield_message.focus = true
                    return
                }

                if (send.sendMwcOffline(selectedAccount, Number(amount).toString(), description, isSlatepack, config.getSendLockOutput(), recipientWallet)) {
                    rect_progress.visible = true
                }
            }
            config.setFluff(true)
            //sendConfirmation.visible = false
            callback(true)
        }
   }
}
/*
let recipientWallet
let isSlatepack = false

*/
