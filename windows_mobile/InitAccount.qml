import QtQuick 2.15
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import QtGraphicalEffects 1.15
import UtilBridge 1.0
import InitAccountBridge 1.0
import "./models"

Item {

    property int tap: 0
    property bool testnet: false


    UtilBridge {
        id: util
    }

    InitAccountBridge {
        id: initAccount
    }

    onVisibleChanged: {
        if (visible) {
            textfield_instancename.text = ""
            textfield_password.text = ""
            textfield_confirm.text = ""
        }
    }

    function strenghtPassword(pass) {
        if (pass) {
            var weak = new RegExp("(?=.{1,})")
            var medium = new RegExp("((?=.{6,})(?=.*[A-Z])(?=.*[a-z]))|((?=.{6,})(?=.*[0-9])(?=.*[a-z]))|((?=.{16,}))")
            var strong = new RegExp("(?=.{10,})(?=.*[!@#\$%\^&\*])(?=.*[0-9])(?=.*[A-Z])(?=.*[a-z])")
            if (strong.test(pass)) {
                strenght_indicator.text = "Strong"
                strenght_indicator.color = Theme.green
            } else if (medium.test(pass)) {
                strenght_indicator.text = "Medium"
                strenght_indicator.color = "orange"
            } else {
                strenght_indicator.text = "Weak"
                strenght_indicator.color = Theme.red
            }
            return true
        }
        return false
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            textfield_instancename.focus = false
            textfield_password.field_focus = false
            textfield_confirm.field_focus = false
        }
    }
    Text {
        id: title
        anchors.top: parent.top
        anchors.topMargin: dp(30)
        text: "Create Wallet"
        font.pixelSize: dp(22)
        font.bold: true
        color: "white"
        width: parent.width
        elide: Text.ElideMiddle
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WrapAnywhere
        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (network_indicator.text != "Mainnet")
                    timer.start()
                tap += 1
                if (tap >= 5) {
                    testnet = !testnet
                    tap = 0
                    timer.stop()
                }
            }
        }
    }

    Text {
        id: network_indicator
        text: testnet? "⛔ Testnet ⛔" : "Mainnet"
        anchors.top: title.bottom
        font.capitalization : Font.AllUppercase
        anchors.topMargin: dp(5)
        font.pixelSize: dp(14)
        color: testnet? "orange" : "white"
        font.italic: true
        width: parent.width
        elide: Text.ElideMiddle
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WrapAnywhere
    }

    Timer {
        id: timer
        interval: 5000; running: false; repeat: false
        onTriggered: {
            tap = 0
            if (tap >= 5)
                stop()
        }
    }
    Text {
        id: desc
        width: parent.width
        anchors.top: title.bottom
        padding: dp(25)
        text: "Define the number of word composing your wallet seed, the instance name along the password that will encrypt your wallet"
        font.pixelSize: dp(15)
        color: "#c2c2c2"
        elide: Text.ElideLeft
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap

    }

    TextFieldCursor {
        id: textfield_instancename
        height: dp(50)
        padding: dp(10)
        leftPadding: dp(20)
        font.pixelSize: dp(18)
        placeholderText: qsTr("Name your instance")
        color: "white"
        text: ""
        anchors.top: desc.bottom
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        horizontalAlignment: Text.AlignLeft

        MouseArea {
            anchors.fill: parent
            onClicked: {
                textfield_instancename.focus = true
                textfield_password.field_focus = false
                textfield_confirm.field_focus = false
            }
        }
    }

    PasswordField {
        id: textfield_password
        height: dp(50)
        anchors.top: textfield_instancename.bottom
        anchors.topMargin: dp(30)
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        textField.onTextChanged: {
            let pass = textfield_password.text
            if (strenghtPassword(pass)) {
                password_strenght.visible = true
                strenght_indicator.visible = true
            } else {
                password_strenght.visible = false
                strenght_indicator.visible = false
            }

            if (!textfield_password.text || !textfield_confirm.text)
                confirm_indicator.visible = false
        }
        mouse.onClicked: {
            textfield_password.field_focus = true
            textfield_confirm.field_focus = false
        }
    }

    Text {
        id: password_strenght
        anchors.top: textfield_password.bottom
        anchors.left: textfield_password.left
        anchors.leftMargin: dp(10)
        anchors.topMargin: dp(10)
        font.pixelSize: dp(12)
        visible: false
        text: "Password Strenght:"
        color: "white"

    }

    Text {
        id: strenght_indicator
        anchors.top: password_strenght.top
        anchors.left: password_strenght.right
        anchors.leftMargin: dp(10)
        font.pixelSize: dp(12)
        font.bold: true
        visible: false
        text: ""
        color: "white"

    }

    PasswordField {
        id: textfield_confirm
        height: dp(50)
        placeHolder: qsTr("Confirm Password")
        anchors.top: password_strenght.bottom
        anchors.topMargin: dp(10)
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        border.width: dp(0)
        border.color: "#00000000"
        textField.onTextChanged: {
            if (!textfield_password.text || !textfield_confirm.text) {
                confirm_indicator.visible = false
            } else if (textfield_confirm.text === textfield_password.text) {
                confirm_indicator.text = "Password match!"
                confirm_indicator.color = "#1afe49"
                confirm_indicator.visible = true
            } else {
                confirm_indicator.text = "Password does not match!"
                confirm_indicator.color = "#fd124f"
                confirm_indicator.visible = true
            }
        }
        mouse.onClicked: {
            textfield_password.field_focus = false
            textfield_confirm.field_focus = true
        }
    }

    Text {
        id: confirm_indicator
        anchors.top: textfield_confirm.bottom
        anchors.left: textfield_confirm.left
        anchors.leftMargin: dp(10)
        anchors.topMargin: dp(10)
        font.pixelSize: dp(12)
        visible: false
        text: ""
        color: "white"

    }

    ToggleSwitch {
        id: toggle_seed
        height: dp(40)
        width: parent.width/2
        text_1: "12 words"
        text_2: "24 words"
        anchors.top: textfield_confirm.bottom
        anchors.topMargin: dp(50)
        anchors.horizontalCenter: parent.horizontalCenter
    }

    ConfirmButton {
        id: button_next
        title: "Next"
        anchors.top: toggle_seed.bottom
        anchors.topMargin: dp(50)
        anchors.horizontalCenter: parent.horizontalCenter

        onClicked: {
            if (!textfield_instancename.text) {
                messagebox.open(qsTr("Instance Name"), qsTr("Please input your instance name"))
                return
            }

            if (!textfield_password.text) {
                messagebox.open(qsTr("Password"), qsTr("Please input your wallet password"))
                return
            }

            const validation = util.validateMwc713Str(textfield_password.text)
            if (validation) {
                messagebox.open(qsTr("Password"), qsTr(validation))
                return
            }

            if (textfield_password.text !== textfield_confirm.text) {
                messagebox.open(qsTr("Password"), qsTr("Password doesn't match confirm string. Please retype the password correctly"))
                return
            }

            util.releasePasswordAnalyser()

            initAccount.setPassword(textfield_password.text)
            initAccount.submitWalletCreateChoices(testnet? 2 : 1, textfield_instancename.text, toggle_seed.state == toggle_seed.text_1 ? 1 : 2) // first param: MWC_NETWORK, second param: New Instance Name, third: SeedLenght: 1 == 12 : 2 == 24
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:4;anchors_width:248;anchors_x:220}D{i:7;anchors_width:150;anchors_x:270}
}
##^##*/
