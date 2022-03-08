import QtQuick 2.4
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import UtilBridge 1.0
import InitAccountBridge 1.0

Item {
    property bool isMainNet: true
    //property bool isPassHidden: true

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

    MouseArea {
        anchors.fill: parent
        onClicked: {
            textfield_instancename.focus = false
            textfield_password.focus = false
            textfield_confirm.focus = false
        }
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
        anchors.bottom: textfield_password.top
        anchors.bottomMargin: dp(25)
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        horizontalAlignment: Text.AlignLeft

        MouseArea {
            anchors.fill: parent
            onClicked: {
                textfield_instancename.focus = true
            }
        }
    }

    TextFieldCursor {
        id: textfield_password
        height: dp(50)
        padding: dp(10)
        leftPadding: dp(20)
        font.pixelSize: textfield_password.text ? dp(10) : dp(18)
        placeholderText: qsTr("Password")
        echoMode: "Password"
        color: "white"
        text: ""
        anchors.bottom: rect_pwdcomment.visible ? rect_pwdcomment.top : textfield_confirm.top
        anchors.bottomMargin: rect_pwdcomment.visible ? dp(5) : dp(15)
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        horizontalAlignment: Text.AlignLeft
        onTextChanged: {
            const validation = util.validateMwc713Str(textfield_password.text)
            if (validation) {
                text_pwdcomment.text = validation
            }
            util.passwordQualitySet(textfield_password.text)
            text_pwdcomment.text = util.passwordQualityComment()
            if (util.passwordQualityIsAcceptable()) {
                rect_pwdcomment.color = "#00000000"
                text_pwdcomment.color = "white"
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                textfield_password.focus = true
            }
        }
    }

    Rectangle {
        id: rect_pwdcomment
        height: text_pwdcomment.height + dp(24)
        color: "#bf002d"
        radius: dp(4)
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        anchors.bottom: textfield_confirm.top
        anchors.bottomMargin: dp(15)
        visible: text_pwdcomment.text

        Text {
            id: text_pwdcomment
            color: "#ffffff"
            text: ""
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            font.pixelSize: dp(14)
        }
    }

    TextFieldCursor {
        id: textfield_confirm
        height: dp(50)
        padding: dp(10)
        leftPadding: dp(20)
        font.pixelSize: textfield_confirm.text ? dp(10) : dp(18)
        placeholderText: qsTr("Confirm password")
        echoMode: TextInput.Normal
        color: "white"
        text: ""
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: rect_pwdcomment.height ? rect_pwdcomment.height : 0
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        horizontalAlignment: Text.AlignLeft
        MouseArea {
            anchors.fill: parent
            onClicked: {
                textfield_confirm.focus = true
            }
        }
    }

    Rectangle {
        id: rect_network
        anchors.top: textfield_confirm.bottom
        anchors.topMargin: dp(50)
        anchors.left: parent.left
        anchors.right: parent.right
        color: "#00000000"

        Text {
            id: label_mainnet
            text: qsTr("Mainnet")
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: image_mainnet.left
            anchors.rightMargin: dp(15)
            horizontalAlignment: Text.AlignRight
            font.pixelSize: dp(17)
            color: "white"
        }

        Image {
            id: image_mainnet
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.horizontalCenterOffset: dp(-30)
            anchors.verticalCenter: parent.verticalCenter
            width: dp(20)
            height: dp(20)
            source: isMainNet ? "../img/StatusOk@2x.svg" : "../img/StatusEmpty@2x.svg"
        }

        MouseArea {
            anchors.left: label_mainnet.left
            anchors.top: image_mainnet.top
            anchors.right: image_mainnet.right
            anchors.bottom: image_mainnet.bottom

            onClicked: {
                isMainNet = true
            }
        }

        Text {
            id: label_testnet
            text: qsTr("Testnet")
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: image_mainnet.right
            anchors.leftMargin: dp(40)
            horizontalAlignment: Text.AlignRight
            font.pixelSize: dp(17)
            color: "white"
        }

        Image {
            id: image_testnet
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: label_testnet.right
            anchors.leftMargin: dp(15)
            width: dp(20)
            height: dp(20)
            source: isMainNet ? "../img/StatusEmpty@2x.svg" : "../img/StatusOk@2x.svg"
        }

        MouseArea {
            anchors.left: label_testnet.left
            anchors.top: image_testnet.top
            anchors.right: image_testnet.right
            anchors.bottom: image_testnet.bottom

            onClicked: {
                isMainNet = false
            }
        }
    }

    Button {
        id: button_next
        height: dp(50)
        width: dp(150)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: rect_network.bottom
        anchors.topMargin: dp(50)
        background: Rectangle {
            id: rectangle
            color: "#00000000"
            radius: dp(5)
            border.color: "white"
            border.width: dp(2)
            Text {
                id: loginText
                text: qsTr("Next")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

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

            util.passwordQualitySet(textfield_password.text)

            if (!util.passwordQualityIsAcceptable()) {
                return
            }

            if (textfield_password.text !== textfield_confirm.text) {
                messagebox.open(qsTr("Password"), qsTr("Password doesn't match confirm string. Please retype the password correctly"))
                return
            }

            util.releasePasswordAnalyser()

            initAccount.setPassword(textfield_password.text)
            initAccount.submitWalletCreateChoices(isMainNet ? 1 : 2, textfield_instancename.text) // first param: MWC_NETWORK, second param: New Instance Name
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:4;anchors_width:248;anchors_x:220}D{i:7;anchors_width:150;anchors_x:270}
}
##^##*/
