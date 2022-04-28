import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import QtGraphicalEffects 1.0
import StateMachineBridge 1.0
import WalletConfigBridge 1.0
import WalletBridge 1.0

import "./models"

Item {
    id: root
    property string passwordHash

    StateMachineBridge {
        id: stateMachine
    }

    WalletConfigBridge {
        id: walletConfig
    }

    WalletBridge {
           id: wallet
       }

    function showPassphraseCallback(ret, password) {
            if (ret) {
                stateMachine.activateShowSeed(password)
            }
        }


    function resyncCallback(ret) {
        if (ret) {
            stateMachine.activateResyncState()
        }
    }

    function path_icon(index){
        switch (index) {
            case 1:
                return "../../img/language.svg"
            case 2:
                return "../../img/dark.svg"
            case 3:
                return "../../img/notification.svg"
            case 4:
                return "../../img/key.svg"
            case 5:
                return "../../img/wallet.svg"
            case 6:
                return "../../img/listener.svg"
            case 7:
                return "../../img/node.svg"
            case 8:
                return "../../img/node.svg"

        }
    }

    ListModel {
        id: settingModal
        ListElement {
            name: "Language"
            index: 1
        }
        ListElement {
            name: "Dark mode"
            index: 2
        }
        ListElement {
            name: "Notification"
            index: 3
        }
        ListElement {
            name: "Show mnemonic"
            index: 4
        }
        ListElement {
            name: "Wallet"
            index: 5
        }
        ListElement {
            name: "Listeners"
            index: 6
        }
        ListElement {
            name: "Network"
            index: 7
        }
        ListElement {
            name: "Resync"
            index: 8
        }
    }

    Rectangle {
           height: dp(62) * settingModal.count
           width: root.width
           color: "#252525"
           anchors.top: parent.top
           anchors.topMargin: dp(25)
           anchors.horizontalCenter: parent.horizontalCenter
           ListView {
               id: contactsList
               //flicking: false
               interactive: false
               anchors.fill: parent
               model: settingModal
               delegate: settingDelegate
               clip: true
               focus: true
           }
       }

       Component {
           id: settingDelegate
           Rectangle {
               height: dp(62)
               color: "#00000000"
               width: root.width
               Rectangle {
                   id: rectangle
                   height: dp(60)
                   width: parent.width
                   color: "#181818"
                   anchors.top: parent.top
                   anchors.horizontalCenter: parent.horizontalCenter

                   ImageColor {
                       id: icon
                       img_source: path_icon(index)
                       img_height: parent.height*0.4
                       img_color: "gray"
                       anchors.left: parent.left
                       anchors.leftMargin: dp(15)
                       anchors.verticalCenter: parent.verticalCenter
                   }

                   Text {
                       color: "#ffffff"
                       text: name
                       font.bold: true
                       font.pixelSize: dp(15)
                       anchors.left: icon.right
                       anchors.leftMargin: dp(15)
                       //anchors.horizontalCenter: parent.horizontalCenter
                       anchors.verticalCenter: parent.verticalCenter
                   }


                   // Dark mode and notification
                   ToggleButton {
                       id: toggle
                       height: parent.height*0.3
                       width: height*2
                       anchors.right: parent.right
                       anchors.rightMargin: dp(15)
                       anchors.verticalCenter: parent.verticalCenter
                       state: index === 2? (isDarkMode? "enabled" : "disabled") : "disabled"
                       visible: (index === 2 || index === 3)? true : false
                       MouseArea {
                           anchors.fill: parent
                           onClicked: {
                               toggle.state = toggle.state === "disabled" ? "enabled" : "disabled"
                               let enable = toggle.state === "disabled" ? false : true

                               if (index === 2) {
                                   isDarkMode = enable
                                   console.log("isDarkMode ", isDarkMode)
                                   walletConfig.setDarkModeEnabled(enable)
                               } else {
                                   walletConfig.setNotificationAndroidEnabled(enable)
                               }
                           }
                       }
                   }
                   MouseArea {
                       id: mouse
                       anchors.fill: parent
                       visible: (index !==2 && index !==3)? true: false
                       onClicked: {
                           switch (index) {
                               case 1:
                               case 2:
                               case 3:
                                   break;
                               case 4:
                                   if (stateMachine.canSwitchState(16)) {
                                       passwordHash = wallet.getPasswordHash()

                                       if (passwordHash !== "") {
                                           messagebox.open("Wallet Password", "You are going to view wallet mnemonic passphrase.",
                                               true, "Cancel", "Confirm", passwordHash, 1, "", showPassphraseCallback)
                                           return;
                                       }
                                       // passwordHash should contain raw password value form the messgage box
                                       stateMachine.activateShowSeed(passwordHash)
                                   }
                                   break;
                               case 5:
                                   stateMachine.setActionWindow(14)
                                   break;
                               case 6:
                                   stateMachine.setActionWindow(10)
                                   break;
                               case 7:
                                   stateMachine.setActionWindow(17)
                                   break;
                               case 8:
                                   messagebox.open("Re-sync account with a node", "Account re-sync will validate transactions and outputs for your accounts. Re-sync can take several minutes.\nWould you like to continue?",true, "No", "Yes", "", "", "", resyncCallback)
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
   }
