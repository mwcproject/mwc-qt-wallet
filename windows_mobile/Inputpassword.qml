import QtQuick 2.4
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import InputPasswordBridge 1.0
import WalletBridge 1.0
import UtilBridge 1.0
import StartWalletBridge 1.0
import ConfigBridge 1.0

Item {
    InputPasswordBridge {
        id: inputPassword
    }

    WalletBridge {
        id: wallet
    }

    UtilBridge {
        id: util
    }

    ConfigBridge {
        id: config
    }

    StartWalletBridge {
        id: startWallet
    }

    Connections {
        target: wallet
        onSgnLoginResult: (ok) => {
            if (!ok) {
                rect_progress.visible = false
                messagebox.open(qsTr("Password"), qsTr("Password supplied was incorrect. Please input correct password."))
                textfield_password.text = ""
            }

            textfield_password.enabled = !ok
            button_login.enabled = !ok
            mousearea_newinstance.enabled = !ok
            mousearea_restore.enabled = !ok
        }

        onSgnUpdateSyncProgress: (progressPercent) => {
            text_syncStatusMsg.text = "Wallet state update, " + util.trimStrAsDouble(Number(progressPercent).toString(), 4) + "% complete"
        }

        onSgnStartingCommand: (actionName) => {
            text_syncStatusMsg.text = actionName
        }
    }

    onVisibleChanged: {
        if (visible) {
            rect_progress.visible = false
            textfield_password.text = ""
            textfield_password.enabled = true
            button_login.enabled = true
            text_syncStatusMsg.text = ""

            instanceItems.clear()
            // <selected path_id>, < <path_id>, <instance name>, <network> >, ...  >
            const instanceData = config.getWalletInstances(true)
            // expecting at least 1 instance value
            if (instanceData.length >= 4) {
                // scanning for networks first
                const networkSet = [];
                for (let i = 1; i<=instanceData.length - 3; i += 3) {
                    networkSet.push(instanceData[i+2])
                }

                let selectedIdx = 0;
                for (let j = 1; j <= instanceData.length - 3; j += 3) {
                    const pathId = instanceData[j]
                    const name = instanceData[j+1]
                    const nw = instanceData[j+2]

                    if (pathId === instanceData[0]) {
                        selectedIdx = j/3
                    }
                    instanceItems.append({
                        instanceName: networkSet.length > 1 ? name + "; " + nw : name,
                        name,
                        pathId
                    })
                }
                instanceComboBox.currentIndex = selectedIdx
            }
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
        width: dp(200)
        height: dp(30)
        fillMode: Image.PreserveAspectFit
        source: "../img/BigLogo@2x.svg"
        anchors.top: parent.top
        anchors.topMargin: dp(100)
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Text {
        id: text_version
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: dp(20)
        color: "white"
        font.pixelSize: dp(14)
        text: config.get_APP_NAME() + " v" + config.getBuildVersion()
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
                text: instanceName
                color: "white"
                font: instanceComboBox.font
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                color: instanceComboBox.highlightedIndex === index ? "#955BDD" : "#8633E0"
            }
            topPadding: dp(10)
            bottomPadding: dp(10)
            leftPadding: dp(20)
            rightPadding: dp(20)
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
            text: instanceComboBox.currentIndex >= 0 && instanceItems.get(instanceComboBox.currentIndex).instanceName
            font: instanceComboBox.font
            color: "white"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
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
            implicitHeight: contentItem.implicitHeight + dp(20)
            topPadding: dp(10)
            bottomPadding: dp(10)
            leftPadding: dp(0)
            rightPadding: dp(0)

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

            onVisibleChanged: {
                if (!instanceComboBox.popup.visible) {
                    canvas.requestPaint()
                }
            }
        }

        model: ListModel {
            id: instanceItems
        }
        anchors.bottom: textfield_password.top
        anchors.bottomMargin: dp(30)
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        leftPadding: dp(20)
        rightPadding: dp(20)
        font.pixelSize: dp(18)
    }

    TextField {
        id: textfield_password
        height: dp(50)
        padding: dp(10)
        leftPadding: dp(20)
        font.pixelSize: textfield_password.text ? dp(10) : dp(18)
        placeholderText: qsTr("Password")
        echoMode: "Password"
        color: "white"
        text: ""
        anchors.bottom: button_login.top
        anchors.bottomMargin: dp(40)
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
        height: dp(50)
        anchors.right: parent.right
        anchors.rightMargin: dp(45)
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: dp(50)
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

            const selectedPath = instanceItems.get(instanceComboBox.currentIndex).name
            config.setActiveInstance(selectedPath)

            rect_progress.visible = true

            // Submit the password and wait until state will push us.
            inputPassword.submitPassword(textfield_password.text)
        }
    }

    Rectangle {
        id: rect_progress
        width: dp(60)
        height: dp(30)
        anchors.top: button_login.bottom
        anchors.topMargin: dp(20)
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#00000000"
        visible: false
        AnimatedImage {
            id: animation
            source: "../img/loading.gif"
        }
    }

    Text {
        id: text_syncStatusMsg
        anchors.top: rect_progress.bottom
        anchors.topMargin: dp(15)
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: dp(16)
        color: "white"
    }

    Image {
        id: image_restore
        width: dp(30)
        height: dp(30)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: dp(-40)
        anchors.bottom: image_newinstance.top
        anchors.bottomMargin: dp(30)
        fillMode: Image.PreserveAspectFit
        source: "../img/RestoreBtn@2x.svg"
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
        id: mousearea_restore
        anchors.left: image_restore.left
        anchors.top: image_restore.top
        anchors.right: text_restore.right
        anchors.bottom: image_restore.bottom
        onClicked: {
            startWallet.createNewWalletInstance("", true);
        }
    }

    Image {
        id: image_newinstance
        width: dp(30)
        height: dp(30)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: dp(-40)
        anchors.bottom: parent.bottom
        anchors.bottomMargin: dp(90)
        fillMode: Image.PreserveAspectFit
        source: "../img/NewInstanceBtn@2x.svg"
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
        id: mousearea_newinstance
        anchors.left: image_newinstance.left
        anchors.top: image_newinstance.top
        anchors.right: text_newinstance.right
        anchors.bottom: image_newinstance.bottom
        onClicked: {
            startWallet.createNewWalletInstance("", false);
        }
    }

//    Image {
//        id: image_help
//        width: dp(30)
//        height: dp(30)
//        anchors.horizontalCenterOffset: dp(-40)
//        anchors.bottomMargin: dp(90)
//        fillMode: Image.PreserveAspectFit
//        source: "../img/HelpBtn@2x.svg"
//        anchors.bottom: parent.bottom
//        anchors.horizontalCenter: parent.horizontalCenter
//    }

//    Text {
//        id: text_help
//        text: qsTr("Help")
//        anchors.leftMargin: dp(20)
//        color: "white"
//        anchors.left: image_help.right
//        anchors.verticalCenter: image_help.verticalCenter
//        font.pixelSize: dp(18)
//    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:4;anchors_width:248;anchors_x:220}D{i:7;anchors_width:150;anchors_x:270}
}
##^##*/
