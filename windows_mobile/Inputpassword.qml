import QtQuick 2.4
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import InputPasswordBridge 1.0
import WalletBridge 1.0
import UtilBridge 1.0

Item {
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    InputPasswordBridge {
        id: inputPassword
    }

    WalletBridge {
        id: wallet
    }

    UtilBridge {
        id: util
    }

    Connections {
        target: wallet
        onSgnLoginResult: {
            textfield_password.enabled = !ok
            button_login.enabled = !ok
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: {
            textfield_password.focus = false
        }
    }

    Image {
        id: image_logo
        width: dp(60)
        height: dp(30)
        fillMode: Image.PreserveAspectFit
        source: "../img/TBLogo@2x.svg"
        anchors.top: parent.top
        anchors.topMargin: dp(80)
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Text {
        id: text_instances
        text: qsTr("Instances")
        color: "white"
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.bottom: instanceComboBox.top
        anchors.bottomMargin: dp(10)
        font.pixelSize: dp(14)
    }

    ComboBox {
        id: instanceComboBox

        delegate: ItemDelegate {
            width: instanceComboBox.width
            contentItem: Text {
                text: modelData
                color: "#7579ff"
                font: instanceComboBox.font
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
            highlighted: instanceComboBox.highlightedIndex === index
        }

        indicator: Canvas {
            id: canvas
            x: instanceComboBox.width - width - instanceComboBox.rightPadding
            y: instanceComboBox.topPadding + (instanceComboBox.availableHeight - height) / 2
            width: dp(14)
            height: dp(7)
            contextType: "2d"

            Connections {
                target: instanceComboBox
                function onPressedChanged() { canvas.requestPaint() }
            }

            onPaint: {
                context.reset()
                context.moveTo(0, 0)
                context.lineTo(width / 2, height)
                context.lineTo(width, 0)
                context.strokeStyle = "white"
                context.stroke()
            }
        }

        contentItem: Text {
            text: instanceComboBox.displayText
            font: instanceComboBox.font
            color: "white"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            implicitHeight: dp(50)
            radius: dp(5)
            color: "#8633E0"
        }

        popup: Popup {
            y: instanceComboBox.height - 1
            width: instanceComboBox.width
            implicitHeight: contentItem.implicitHeight
            padding: dp(1)

            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: instanceComboBox.popup.visible ? instanceComboBox.delegateModel : null
                currentIndex: instanceComboBox.highlightedIndex

                ScrollIndicator.vertical: ScrollIndicator { }
            }

            background: Rectangle {
                border.color: "white"
                radius: dp(3)
            }
        }

        model: ListModel {
            id: accountItems
            ListElement { text: "Default" }
        }
        anchors.bottom: text_login.top
        anchors.bottomMargin: dp(15)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        leftPadding: dp(10)
        rightPadding: dp(10)
        font.pixelSize: dp(18)
    }

    Text {
        id: text_login
        text: qsTr("Login")
        color: "white"
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.bottom: textfield_password.top
        anchors.bottomMargin: dp(10)
        font.pixelSize: dp(14)
    }

    TextField {
        id: textfield_password
        height: dp(50)
        padding: dp(5)
        leftPadding: dp(10)
        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: dp(18)
        placeholderText: qsTr("Type your password")
        echoMode: "Password"
        color: "white"
        text: ""
        anchors.verticalCenterOffset: dp(-10)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        horizontalAlignment: Text.AlignHCenter


        background: Rectangle {
            color: "#8633E0"
            radius: dp(5)
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                textfield_password.focus = true
            }
        }
    }

    Button {
        id: button_login
        height: dp(50)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.top: textfield_password.bottom
        anchors.topMargin: dp(50)
        background: Rectangle {
            id: rectangle
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                id: loginText
                text: qsTr("Open Wallet")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
            if (!textfield_password.text) {
                messagebox.open(qsTr("Password"), qsTr("Please input your wallet password"))
                return
            }

            const validation = util.validateMwc713Str(textfield_password.text)
            if (validation) {
                messagebox.open(qsTr("Password"), qsTr(validation))
                return
            }
            if (textfield_password.text[0] === "-") {
                messagebox.open(qsTr("Password"), qsTr("You can't start your password from '-' symbol."))
                return
            }

            // Submit the password and wait until state will push us.
            inputPassword.submitPassword(textfield_password.text)
        }
    }

    Image {
        id: image_help
        width: dp(30)
        height: dp(30)
        anchors.horizontalCenterOffset: dp(-20)
        anchors.bottomMargin: dp(100)
        fillMode: Image.PreserveAspectFit
        source: "../img/HelpBtn@2x.svg"
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Text {
        id: text_help
        text: qsTr("Help")
        anchors.leftMargin: dp(20)
        color: "white"
        anchors.left: image_help.right
        anchors.verticalCenter: image_help.verticalCenter
        font.pixelSize: dp(18)
    }

    MessageBox {
        id: messagebox
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
    }

}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:4;anchors_width:248;anchors_x:220}D{i:7;anchors_width:150;anchors_x:270}
}
##^##*/
