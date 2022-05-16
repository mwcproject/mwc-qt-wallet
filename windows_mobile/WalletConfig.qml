import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import QtGraphicalEffects 1.0
import ConfigBridge 1.0
import WalletConfigBridge 1.0
import UtilBridge 1.0
import "./models"

Item {
    id: root

    property bool isInitWalletConfig:true

    WalletConfigBridge {
        id: walletConfig
    }

    ConfigBridge {
        id: config
    }

    function logEnableUpdate(needCleanupLogs) {
        walletConfig.updateWalletLogsEnabled(false, needCleanupLogs)

    }

    function _callbackInstanceName(action, name) {
        if (action) {
            walletInstanceName = name
            config.updateActiveInstanceName(name)
            configModal.setProperty(2, "value", name)
        }
    }



    function path_icon(index){
        switch (index) {
            case 0:
                return "../../img/mwc-logo.svg"
            case 1:
                return "../../img/dark.svg"
            case 2:
                return "../../img/notification.svg"
            case 3:
                return "../../img/key.svg"
            case 4:
                return "../../img/wallet.svg"
            case 5:
                return "../../img/listener.svg"

        }
    }
    property string walletInstanceName
    property bool isOutputs
    property bool walletLogsEnabled : walletConfig.getWalletLogsEnabled()
    property bool autoStartMQSEnabled :  walletConfig.getAutoStartMQSEnabled()
    property bool autoStartTOREnabled : walletConfig.getAutoStartTorEnabled()
    property int confirmationsNumber : walletConfig.getInputConfirmationsNumber()
    property int changeOutputs : walletConfig.getChangeOutputs()



    function updateButton(){
        const instanceInfo = config.getCurrentWalletInstance()
        walletInstanceName = instanceInfo[2]
    }

    onVisibleChanged: {
        if (visible && isInitWalletConfig) {
            walletConfig.canApplySettings(false)
            isInitWalletConfig = false
            updateButton()
        }
        if (!visible) {

        }
    }

    Rectangle {
        id: container
        height: dp(67) * configModal.count
        width: root.width
        color: Theme.field
        anchors.horizontalCenter: parent.horizontalCenter
        ListView {
            id: contactsList
            //flicking: false
            interactive: false
            anchors.fill: parent
            model: configModal
            delegate: configDelegate
            clip: true
            focus: true
        }
    }
    ListModel {
        id: configModal
        ListElement {
            name: "MQS Auto Start"
        }
        ListElement {
            name: "TOR Auto Start"
        }
        ListElement {
            name: "Instance Name"
        }
        ListElement {
            name: "Confirmation Number"
        }
        ListElement {
            name: "Change Outputs"
        }
        ListElement {
            name: "Logs"
        }
    }

    Component {
        id: configDelegate
        Rectangle {
            id: rec
            height: dp(67)
            color: "#00000000"
            width: root.width
            Rectangle {
                id: rectangle
                height: dp(65)
                width: parent.width
                color: Theme.card
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter

                ImageColor {
                    id: icon
                    img_source: path_icon(index)
                    img_height: index === 0? parent.height*0.2 : parent.height*0.4
                    img_color: Theme.icon
                    anchors.left: parent.left
                    anchors.leftMargin: dp(15)
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    color: Theme.textPrimary
                    text: name
                    font.bold: true
                    font.pixelSize: dp(15)
                    anchors.left: icon.right
                    anchors.leftMargin: dp(15)
                    //anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    color: Theme.textPrimary
                    text: index === 2? walletInstanceName : (index === 3? confirmationsNumber : (index === 4? changeOutputs : ""))
                    font.bold: true
                    font.pixelSize: dp(15)
                    anchors.right: parent.right
                    anchors.rightMargin: dp(15)
                    //anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    visible: (index >1 && index !== 6)? true : false
                }




                // Dark mode and notification
                ToggleButton {
                    id: toggle
                    height: parent.height*0.5
                    width: height*2
                    anchors.right: parent.right
                    anchors.rightMargin: dp(15)
                    anchors.verticalCenter: parent.verticalCenter
                    state: index === 0? (autoStartMQSEnabled? "enabled" : "disabled") : (index === 1? (autoStartTOREnabled? "enabled" : "disabled") : (index === 6? (autoStartTOREnabled? "enabled" : "disabled") : "disabled"))
                    visible: (index === 0 || index === 1 || index === 5)? true : false
                    mouse.onClicked: {
                        toggle.state = toggle.state === "disabled" ? "enabled" : "disabled"
                        let enable = toggle.state === "disabled" ? false : true
                        if (index === 0) {
                            walletConfig.updateAutoStartMQSEnabled(enable)
                        } else if (index === 1) {
                            walletConfig.updateAutoStartTorEnabled(enable)
                        } else {
                            if (enable) {
                                messagebox.open(qsTr("Wallet Logs"), qsTr("You just enabled the logs. Log files location:\n~/mwc-qt-wallet/logs\n\nPlease note, the logs can contain private infromation about your transactions and accounts."))
                                walletConfig.updateWalletLogsEnabled(enable, false)
                            } else {
                                messagebox.open("Wallet Logs",
                                    "You just disabled the logs. Log files location: ~/mwc-qt-wallet/logs\n\nPlease note, the logs can contain private information about your transactions and accounts.\n\nDo you want to clean up existing logs from your wallet?",
                                    true, "Keep", "Clean up", "", "", "", logEnableUpdate)
                            }
                        }
                    }
                }

                MouseArea {
                    id: mouse
                    anchors.fill: parent
                    visible: (index>1 && index<5)? true: false
                    onClicked: {
                        switch (index) {
                            case 0:
                            case 1:
                                break;
                            case 2:
                                inputDlg.open("Wallet Instance Name", "Change the wallet Instance Name", "Instance Name", "", 13, _callbackInstanceName)
                                break;
                            case 3:
                                isOutputs = false
                                numberList.open()
                                break;
                            case 4:
                                isOutputs = true
                                numberList.open()
                                break;
                            case 5:
                                break;
                        }
                    }
                }

                ColorOverlay {

                    id: overlay
                    anchors.fill: rectangle
                    source: rectangle
                    opacity: 0
                }
                states: [
                    State {
                        name: "pressed"; when: mouse.pressed && index !==2 && index !==3
                        PropertyChanges {
                            target: overlay; color: "black"; opacity: 0.5
                        }
                    },
                    State {
                        name: "unpressed"; when: !mouse.pressed
                        PropertyChanges {
                            target: overlay; color: "black"; opacity: 0
                        }
                    }]

                transitions: Transition {
                    NumberAnimation {
                        properties: "opacity"; duration: 250; easing.type: Easing.InOutQuad
                    }
                }
            }
        }
    }

    DrawerList {
        id: numberList
        repeater.model: 10
        repeater.delegate: numberModal
    }

    Component {
        id: numberModal
        Rectangle {
            id: rec_number
            height: dp(60)
            width: dp(180)
            color: (isOutputs? changeOutputs : confirmationsNumber) === index +1? Theme.field : "#00000000"

            ImageColor {
                id: img_check_number
                img_height: parent.height/2.5
                img_source: "../../img/check.svg"
                img_color: (isOutputs? changeOutputs : confirmationsNumber) === index +1 ? Theme.iconSelected : Theme.icon
                anchors.left: parent.left
                anchors.leftMargin: dp(25)
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                id: number
                text: index +1
                color: Theme.textPrimary
                font.pixelSize: rec_number.height/4
                font.bold: true
                font.capitalization: Font.AllUppercase
                anchors.left: img_check_number.right
                anchors.leftMargin: dp(25)
                anchors.verticalCenter: parent.verticalCenter
            }

            MouseArea {
                id: mouse_number
                anchors.fill: parent
                onClicked: {
                    if (isOutputs) {
                        changeOutputs = index + 1
                    } else {
                        confirmationsNumber = index + 1
                    }
                    walletConfig.canApplySettings(true);
                    walletConfig.setSendCoinsParams(confirmationsNumber, changeOutputs)
                    walletConfig.canApplySettings(false);
                }
            }
        }
    }
   }
