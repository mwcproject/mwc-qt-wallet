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
    id: root
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
            isDarkMode = walletConfig.getDarkModeEnabled()
            rect_progress.visible = false
            textfield_password.text = ""
            textfield_password.enabled = true
            button_login.enabled = true
            text_syncStatusMsg.text = ""
            instanceList.clear()
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
                    instanceList.append({
                        instanceName: networkSet.length > 1 ? name + (nw === "Mainnet"? "" : " - "+ nw) : name,
                        name,
                        pathId
                    })
                }
                selectedInstance = instanceList.get(0).instanceName
                selectedPathId = instanceList.get(0).pathId
            }
        }
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

   /* Rectangle {
        height: 150
        width: 50
        anchors.top: parent.top
        color: "red"
        MouseArea {
            id: mo
            anchors.fill: parent
            onClicked: {
                walletConfig.setLanguage("fr")
            }
        }
    }*/



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
        color: "#242424"
        anchors.bottom: textfield_password.top
        anchors.bottomMargin: dp(20)
        anchors.horizontalCenter: parent.horizontalCenter
        //color: "#111111"
        Rectangle {
            id: visualImageRectangle
            height: parent.height*0.8
            width: height
            color: "#00000000"
            radius: dp(50)
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: dp(10)

            ImageColor {
                id: visualImage
                img_source: "../../img/wallet.svg"
                img_height: visualImageRectangle.height*0.8
                img_color: "#181818"
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
                //if (instanceList.count > 1)
                    nav.open()
            }
        }
    }

    PasswordField {
        id: textfield_password
        height: dp(55)
        width: rec_wallet_account.width
        placeHolder: qsTr("Password")
        mainColor: "#242424"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        focus: false
    }


    ConfirmButton {
        id: button_login
        height: dp(50)
        width: rec_wallet_account.width
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

    ListModel {
        id: instanceList
    }




    DrawerList {
        id: nav
        repeater.model: instanceList
        repeater.delegate: instanceItems
    }


    Component {
        id: instanceItems
        Rectangle {
            id: rec_acc_balance
            height: dp(60)
            width: root.width
            color: root.selectedPathId === pathId? "#252525" : "#00000000"

            ImageColor {
                id: img_check
                img_height: parent.height/2.5
                img_source: "../../img/check.svg"
                img_color: root.selectedPathId === pathId? "white" : "#151515"
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
                id: mouse
                anchors.fill: parent
                onClicked: {
                    root.selectedInstance = instanceName
                    root.selectedPathId = pathId
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
