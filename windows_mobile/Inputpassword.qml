import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Templates 2.15 as T
import QtQuick.Window 2.0
import InputPasswordBridge 1.0
import WalletBridge 1.0
import UtilBridge 1.0
import StartWalletBridge 1.0
import ConfigBridge 1.0
import QtAndroidService 1.0
import QtQuick.Layouts 1.15


import "./models"

Item {
    id: control
    state: "close"
    property string selectedInstance
    property string selectedPathId
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

    QtAndroidService {
        id: qtAndroidService
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
            let singleInstance = (instanceData.length <= 4) ? true : false
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
                        instanceName: networkSet.length > 1 ? name + (nw === "Mainnet"? "" : " - "+ nw) : name,
                        name,
                        pathId
                    })
                }
                selectedInstance = instanceItems.get(0).instanceName
                selectedPathId = instanceItems.get(0).pathId
            }
        }
    }

    ListModel {
        id: instanceItems
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: {
            textfield_password.field_focus = false
        }
    }

    Image {
        id: image_logo
        width: dp(105)
        //height: dp(30)
        fillMode: Image.PreserveAspectFit
        source: "../img/mwc-logo.svg"
        anchors.top: parent.top
        anchors.topMargin: dp(50)
        anchors.horizontalCenter: parent.horizontalCenter
    }



    Text {
        id: text_version
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: dp(20)
        color: "white"
        font.pixelSize: dp(14)
        text: config.get_APP_NAME() + " v" + qtAndroidService.getApkVersion()
    }

    Rectangle {
        id: rec_wallet_account
        height: dp(55)
        width: parent.width/1.3
        radius: dp(20)
        anchors.bottom: textfield_password.top
        anchors.bottomMargin: dp(20)
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#111111"
        Rectangle {
            id: visualImageRectangle

            height: parent.height*0.8
            width: height
            color: "#2a122c"
            radius: dp(50)
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: dp(10)
            //opacity: 0.5
            ImageColor {
                id: visualImage
                img_source: "../../img/eye.svg"
                img_height: visualImageRectangle.height*0.8
                anchors.centerIn: visualImageRectangle
            }
        }
        Text {
            id: buttontext
            font.pixelSize: dp(17)
            text: selectedInstance
            color: "#ffffff"
            font.family: barlow.medium
            font.letterSpacing: dp(1)
            anchors.left: visualImageRectangle.right
            anchors.leftMargin: dp(20)
            anchors.verticalCenter: parent.verticalCenter
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                nav.open()
            }
        }
    }







    PasswordField {
        id: textfield_password
        height: dp(55)
        width: rec_wallet_account.width
        anchors.topMargin: dp(80)
        anchors.bottomMargin: dp(40)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        focus: false
    }


    ConfirmButton {
        id: button_login
        anchors.top: textfield_password.bottom
        anchors.topMargin: dp(80)
        anchors.horizontalCenter: parent.horizontalCenter


        onClicked: {
            textfield_password.field_focus = false
            if (!textfield_password.text) {
                messagebox.open(qsTr("id-password"), qsTr("Please input your wallet password"))
                return
            }

            const validation = util.validateMwc713Str(textfield_password.text)
            if (validation) {
                messagebox.open(qsTr("id-password"), qsTr(validation))
                return
            }

            console.log(123, selectedPathId)
            config.setActiveInstance(selectedPathId)

            rect_progress.visible = true

            // Submit the password and wait until state will push us.
            inputPassword.submitPassword(textfield_password.text)
        }
    }

    MouseArea {
        id: mousearea_restore
        anchors.right: text_restore.right
        onClicked: {
            startWallet.createNewWalletInstance("", true);
        }
    }


    Text {
        id: text_newinstance
        text: qsTr(" ")
        //anchors.leftMargin: dp(20)
        color: "white"
        font.pixelSize: dp(18)
    }

    MouseArea {
        id: mousearea_newinstance
        anchors.right: text_newinstance.right
        onClicked: {
            startWallet.createNewWalletInstance("", false);
        }
    }

    Text {
        id: text_syncStatusMsg
        anchors.bottom: rect_progress.top
        anchors.bottomMargin: dp(10)
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: dp(16)
        color: "white"
    }

    Rectangle {
        id: rect_progress
        width: dp(60)
        height: dp(30)
        anchors.bottom: parent.bottom
        anchors.bottomMargin: dp(50)
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#00000000"
        visible: false
        AnimatedImage {
            id: animation
            source: "../img/loading.gif"
        }
    }


    Rectangle {
        id: rect_network
        anchors.top: button_login.bottom
        anchors.topMargin: dp(80)
        anchors.horizontalCenter: parent.horizontalCenter
        radius: dp(50)
        color: "#00000000"
        height: dp(40)
        width: parent.width/1.5

        Button {
            id: mainnetButton
            height: parent.height - dp(10)
            width: parent.width/2 - dp(15)
            anchors.rightMargin: dp(20)
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left

            background: Rectangle {
                id: mainnetRectangle
                color: "#343b47"
                radius: dp(10)
                Text {
                    id: mainnetText
                    //% "Password"
                    text: qsTr("id-password")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.letterSpacing: dp(2)
                    font.pixelSize: dp(16)
                    color: "white"
                }
            }
            onClicked: {
                startWallet.createNewWalletInstance("", false);
            }
        }

        Button {
            id: testnetButton
            height: parent.height -dp(10)
            width: parent.width/2 - dp(15)
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            background: Rectangle {
                id: testnetRectangle
                color: "#111111"
                radius: dp(10)
                Text {
                    id: testnetText
                    text: qsTr("Restore")
                    font.letterSpacing: dp(2)
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: dp(16)
                    color: "white"
                }
            }
            onClicked: {
                qtAndroidService.setNotification("Reki")
                startWallet.createNewWalletInstance("", true);
            }
        }
    }

    Drawer {
        id: nav
        enter: Transition {
            SmoothedAnimation {
                velocity: -1
                duration: 750
                easing.type: Easing.OutCirc
                easing.amplitude: 2.0
                easing.period: 1.5
            }
        }
        exit: Transition {
            SmoothedAnimation {
                velocity: -1
                duration: 500
                easing.type: Easing.OutCirc
                easing.amplitude: 2.0
                easing.period: 1.5

            }

        }
        height: grid_seed.Layout.minimumHeight + dp(50)
        width: parent.width
        edge: Qt.BottomEdge
        interactive: position == 1.0? true : false
        Rectangle {
            anchors.top: parent.top
            anchors.topMargin: dp(50)
            height: Math.min(grid_seed.Layout.minimumHeight, control.height/2)
            width: parent.width
            color: "#00000000"
            Flickable {
                id: scroll
                width: parent.width
                height: parent.height
                contentHeight: grid_seed.Layout.minimumHeight
                clip: true
                ScrollBar.vertical: ScrollBar {
                    policy: Qt.ScrollBarAlwaysOn
                }
                ColumnLayout {
                    id: grid_seed
                    spacing: 0
                    width: rect_phrase.width
                    Repeater {
                        id: rep
                        model: instanceItems

                        Rectangle {
                            id: rec_acc_balance
                            height: dp(60)
                            width: nav.width
                            color: selectedPathId === pathId? "#252525" : "#00000000"

                            ImageColor {
                                id: img_check
                                img_height: parent.height/2.5
                                img_source: "../../img/check.svg"
                                img_color: selectedPathId === pathId? "white" : "#151515"
                                anchors.left: parent.left
                                anchors.leftMargin: dp(25)
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                id: acc_name
                                text: instanceName
                                color: "white"
                                font.pixelSize: dp(18)
                                font.italic: true
                                font.weight: Font.Light
                                anchors.left: img_check.right
                                anchors.leftMargin: dp(25)
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    selectedInstance = instanceName
                                    selectedPathId = pathId
                                }
                            }
                        }
                    }

                }
            }
        }

        background: Rectangle {
            radius: 50
            height: grid_seed.Layout.minimumHeight+ dp(50) + parent.height/5
            width: parent.width
            //y: -parent.height/5
            color: "#151515"
        }

        T.Overlay.modal: Rectangle {
            color: "#80000000"
            Behavior on opacity {
                NumberAnimation {
                    duration: 150
                }
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:4;anchors_width:248;anchors_x:220}D{i:7;anchors_width:150;anchors_x:270}
}
##^##*/
