import QtQuick 2.4
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import InputPasswordBridge 1.0
import WalletBridge 1.0
import UtilBridge 1.0
import StateMachineBridge 1.0

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

    StateMachineBridge {
        id: stateMachine
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
        anchors.leftMargin: dp(45)
        anchors.bottom: instanceComboBox.top
        anchors.bottomMargin: dp(10)
        font.pixelSize: dp(14)
    }

    ComboBox {
        id: instanceComboBox

        delegate: ItemDelegate {
            width: instanceComboBox.width
            contentItem: Text {
                text: value
                color: instanceComboBox.highlightedIndex === index ? "#8633E0" : "white"
                font: instanceComboBox.font
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
            highlighted: instanceComboBox.highlightedIndex === index
            topPadding: dp(10)
            bottomPadding: dp(10)
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
                if (instanceComboBox.popup.visible) {
                    context.moveTo(0, height)
                    context.lineTo(width / 2, 0)
                    context.lineTo(width, height)
                } else {
                    context.moveTo(0, 0)
                    context.lineTo(width / 2, height)
                    context.lineTo(width, 0)
                }
                context.strokeStyle = "white"
                context.lineWidth = 2
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
            y: instanceComboBox.height + dp(3)
            width: instanceComboBox.width
            implicitHeight: contentItem.implicitHeight + dp(40)
            padding: dp(20)

            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: instanceComboBox.popup.visible ? instanceComboBox.delegateModel : null
                currentIndex: instanceComboBox.highlightedIndex

                ScrollIndicator.vertical: ScrollIndicator { }
            }

            background: Rectangle {
                color: "#8633E0"
                radius: dp(5)
            }
        }

        model: ListModel {
            id: accountItems
            ListElement { value: "Default" }
            ListElement { value: "Name of instance 2" }
            ListElement { value: "Another instance name" }
            ListElement { value: "another instance" }
            ListElement { value: "Long instance name long name" }
        }
        anchors.bottom: text_login.top
        anchors.bottomMargin: dp(15)
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        leftPadding: dp(20)
        rightPadding: dp(20)
        font.pixelSize: dp(18)
    }

    Text {
        id: text_login
        text: qsTr("Login")
        color: "white"
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        anchors.bottom: textfield_password.top
        anchors.bottomMargin: dp(10)
        font.pixelSize: dp(14)
    }

    TextField {
        id: textfield_password
        height: dp(50)
        padding: dp(10)
        leftPadding: dp(20)
        font.pixelSize: dp(18)
        placeholderText: qsTr("Type your password")
        echoMode: "Password"
        color: "white"
        text: ""
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: dp(-10)
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        horizontalAlignment: Text.AlignLeft
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
        height: dp(70)
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        anchors.top: textfield_password.bottom
        anchors.topMargin: dp(40)
        background: Rectangle {
            id: rectangle
            color: "#00000000"
            radius: dp(5)
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

            // Submit the password and wait until state will push us.
            inputPassword.submitPassword(textfield_password.text)
        }
    }

    Image {
        id: image_restore
        width: dp(30)
        height: dp(30)
        anchors.horizontalCenterOffset: dp(-40)
        anchors.bottomMargin: dp(30)
        fillMode: Image.PreserveAspectFit
        source: "../img/RestoreBtn@2x.svg"
        anchors.bottom: image_newinstance.top
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Text {
        id: text_restore
        text: qsTr("Restore")
        anchors.leftMargin: dp(20)
        color: "white"
        anchors.left: image_restore.right
        anchors.verticalCenter: image_restore.verticalCenter
        font.pixelSize: dp(18)
    }

    MouseArea {
        anchors.left: image_restore.left
        anchors.top: image_restore.top
        anchors.right: text_restore.right
        anchors.bottom: image_restore.bottom
        onClicked: {
            console.log("Restore Instance Clicked")
        }
    }

    Image {
        id: image_newinstance
        width: dp(30)
        height: dp(30)
        anchors.horizontalCenterOffset: dp(-40)
        anchors.bottomMargin: dp(30)
        fillMode: Image.PreserveAspectFit
        source: "../img/NewInstanceBtn@2x.svg"
        anchors.bottom: image_help.top
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Text {
        id: text_newinstance
        text: qsTr("New Instance")
        anchors.leftMargin: dp(20)
        color: "white"
        anchors.left: image_newinstance.right
        anchors.verticalCenter: image_newinstance.verticalCenter
        font.pixelSize: dp(18)
    }

    MouseArea {
        anchors.left: image_newinstance.left
        anchors.top: image_newinstance.top
        anchors.right: text_instances.right
        anchors.bottom: image_newinstance.bottom
        onClicked: {
            stateMachine.setActionWindow(2)
        }
    }

    Image {
        id: image_help
        width: dp(30)
        height: dp(30)
        anchors.horizontalCenterOffset: dp(-40)
        anchors.bottomMargin: dp(90)
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
    }

}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:4;anchors_width:248;anchors_x:220}D{i:7;anchors_width:150;anchors_x:270}
}
##^##*/
