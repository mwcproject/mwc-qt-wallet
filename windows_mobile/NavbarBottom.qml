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
        color: "#15171c"
        Rectangle {
            id: rect
            height: navbarBottom.height/14
            width: parent.width/8
            radius: dp(50)
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop {
                    position: 0
                    color: "#9d00e7"
                }

                GradientStop {
                    position: 1
                    color: "#3700c9"
                }
            }
        }

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
                    anchors.bottom: text_wallet.top
                    anchors.bottomMargin: dp(2)
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
                    anchors.bottom: text_txs.top
                    anchors.bottomMargin: dp(2)
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
                    anchors.bottom: text_account.top
                    anchors.bottomMargin: dp(2)
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
                    anchors.bottom: text_settings.top
                    anchors.bottomMargin: dp(2)
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


    states: [

            State {
                name: "wallet_state"
                PropertyChanges {
                    target: rect; anchors.leftMargin: (wallet.width - width)/2
                }
            },
            State {
                name: "txs_state"
                PropertyChanges {
                    target: rect; anchors.leftMargin:  wallet.width + (wallet.width - width)/2
                }
            },
            State {
                name: "contact_state"
                PropertyChanges {
                    target: rect; anchors.leftMargin:  wallet.width*2 + (wallet.width - width)/2
                }
            },
            State {
                name: "setting_state"
                PropertyChanges {
                    target: rect; anchors.leftMargin:  wallet.width*3 + (wallet.width - width)/2
                }
            }]

        transitions: Transition {
            PropertyAnimation {
                properties: "anchors.leftMargin";
                duration: 250
            }
        }






}

/*##^##
Designer {
    D{i:0;autoSize:true;height:640;width:320}D{i:43;anchors_x:38}D{i:44;anchors_x:116}
}
##^##*/
