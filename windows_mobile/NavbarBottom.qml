import QtQuick 2.12
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.15
import QtQuick.Templates 2.15 as T
import WalletBridge 1.0
import StateMachineBridge 1.0
import ConfigBridge 1.0
import AccountsBridge 1.0
import UtilBridge 1.0
import "./models"

Item {
    id: controle
    state: "wallet_state"

    Rectangle {
        id: navbarBottom
        height: parent.height/14
        width: parent.width
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: Theme.card

        RowLayout {
            id: layoutNavBottom
            spacing: 0
            Rectangle {
                id: wallet
                color: "#00000000"
                Layout.fillHeight: true
                Layout.minimumHeight: parent.parent.height
                Layout.minimumWidth: parent.parent.width/4
                Layout.preferredWidth: 100
                Layout.maximumWidth: 300
                ImageColor {
                    id: image_wallet
                    img_height: parent.height* 0.5
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    img_source: "../../img/wallet.svg"
                    img_color: currentState === 8 || currentState === 9 || currentState === 19 || currentState === 21 ? "white" : "grey"


                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (currentState !== 8 && currentState !== 9 && currentState !== 19 && currentState !== 21) {
                            stateMachine.setActionWindow(21)
                            controle.state = "wallet_state"
                        }

                    }
                }
            }
            Rectangle {
                id: txs
                color: "#00000000"
                Layout.fillHeight: true
                Layout.minimumHeight: parent.parent.height
                Layout.minimumWidth: parent.parent.width/4
                Layout.preferredWidth: 100
                Layout.maximumWidth: 300
                ImageColor {
                    id: image_txs
                    img_height: parent.height* 0.5
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    img_source: "../../img/list.svg"
                    img_color: currentState === 11 ? "white" : "grey"

                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        stateMachine.setActionWindow(11)
                        controle.state = "txs_state"
                    }
                }
            }
            Rectangle {
                id: contact
                color: "#00000000"
                Layout.fillHeight: true
                Layout.minimumHeight: parent.parent.height
                Layout.minimumWidth: parent.parent.width/4
                Layout.preferredWidth: 100
                Layout.maximumWidth: 300

                ImageColor {
                    id: image_account
                    img_height: parent.height* 0.5
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    img_source: "../../img/list-contact.svg"
                    img_color: currentState === 13 ? "white" : "grey"

                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        stateMachine.setActionWindow(13)
                        controle.state = "contact_state"
                    }
                }
            }
            Rectangle {
                id: setting
                color: "#00000000"
                Layout.fillHeight: true
                Layout.minimumHeight: parent.parent.height
                Layout.minimumWidth: parent.parent.width/4
                Layout.preferredWidth: 100
                Layout.maximumWidth: 300
                ImageColor {
                    id: image_settings
                    img_height: parent.height* 0.5
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    img_source: "../../img/setting.svg"
                    img_color: currentState === 23 ? "white" : "grey"


                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        stateMachine.setActionWindow(23)
                        controle.state = "setting_state"
                    }
                }
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:640;width:320}D{i:43;anchors_x:38}D{i:44;anchors_x:116}
}
##^##*/
