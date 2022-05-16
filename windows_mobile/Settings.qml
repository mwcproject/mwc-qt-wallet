import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import QtGraphicalEffects 1.0
import StateMachineBridge 1.0
import WalletConfigBridge 1.0
import WalletBridge 1.0
import "utils.js" as Util
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
            case 0:
            case 1:
                return "../../img/dollar.svg"
            case 2:
                return "../../img/dark.svg"
            case 3:
                return "../../img/notification.svg"
            case 4:
                return "../../img/wallet.svg"
            case 5:
                return "../../img/key.svg"
            case 6:
                return "../../img/listener.svg"

        }
    }

    function setCurrencyList(){
        const currencyList = Util.json_currency
        for (let i=0; i <= currencyList.length - 1; i++) {
            let name = currencyList[i].name
            let ticker = currencyList[i].ticker
           currency_list.append({ticker, name})
        }
    }

    property bool isInitWallet: true

    onVisibleChanged: {
        if (visible && isInitWallet) {
            setCurrencyList()
            isInitWallet = false

        }
    }

    ListModel {
        id: currency_list
    }

    DrawerList {
        id: language
        repeater.model: currency_list
        repeater.delegate: instanceItems
    }

    DrawerList {
        id: currency
        repeater.model: currency_list
        repeater.delegate: instanceItems
    }



    Component {
        id: instanceItems
        Rectangle {
            id: rec_acc_balance
            height: dp(60)
            width: root.width
            color: currencyTicker === ticker? "#252525" : "#00000000"

            ImageColor {
                id: img_check
                img_height: parent.height/2.5
                img_source: "../../img/check.svg"
                img_color: currencyTicker === ticker? Theme.textPrimary : "#151515"
                anchors.left: parent.left
                anchors.leftMargin: dp(25)
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                id: acc_ticker
                text: ticker
                color: Theme.textPrimary
                font.pixelSize: rec_acc_balance.height/4
                font.bold: true
                font.capitalization: Font.AllUppercase
                anchors.left: img_check.right
                anchors.leftMargin: dp(25)
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: -font.pixelSize/2
            }
            Text {
                id: acc_name
                text: name
                color: Theme.textSecondary
                font.pixelSize: rec_acc_balance.height/4
                font.italic: true
                font.weight: Font.Light
                anchors.left: img_check.right
                anchors.leftMargin: dp(25)
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: font.pixelSize/1.5
            }
            MouseArea {
                id: mouse
                anchors.fill: parent
                onClicked: {
                    currencyTicker = ticker.toLowerCase()
                    currencyIndex(index)
                    walletConfig.setPriceCurrency(index)
                    Util.getPrice()
                    walletItem.setChart()

                }
            }
        }
    }



    ListModel {
        id: settingModal

        ListElement {
            name: "Currency"
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
            name: "Wallet"
            index: 4
        }
        ListElement {
            name: "Show mnemonic"
            index: 5
        }
        ListElement {
            name: "Resync"
            index: 6
        }
    }

    Rectangle {
           height: dp(67) * settingModal.count
           width: root.width
           color: "#7D252525"
           anchors.top: parent.top
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
                       img_height: parent.height*0.4
                       img_color: Theme.textSecondary
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

                   Text {
                       color: "#ffffff"
                       text: index === 0? "Francais": currencyTicker.toUpperCase()
                       font.bold: true
                       font.pixelSize: dp(15)
                       anchors.right: parent.right
                       anchors.rightMargin: dp(15)
                       //anchors.horizontalCenter: parent.horizontalCenter
                       anchors.verticalCenter: parent.verticalCenter
                       visible: index === 0 || index === 1? true: false
                   }




                   // Dark mode and notification
                   ToggleButton {
                       id: toggle
                       height: parent.height*0.5
                       width: height*2
                       anchors.right: parent.right
                       anchors.rightMargin: dp(15)
                       anchors.verticalCenter: parent.verticalCenter
                       state: index === 2? (isDarkMode? "enabled" : "disabled") : "disabled"
                       visible: (index === 2 || index === 3)? true : false
                       mouse.onClicked: {
                           toggle.state = toggle.state === "disabled" ? "enabled" : "disabled"
                           let enable = toggle.state === "disabled" ? false : true
                           if (index === 2) {
                               isDarkMode = enable
                               walletConfig.setDarkModeEnabled(enable)
                           } else {
                               walletConfig.setNotificationAndroidEnabled(enable)
                           }
                       }
                   }
                   MouseArea {
                       id: mouse
                       anchors.fill: parent
                       visible: (index !==2 && index !==3)? true: false
                       onClicked: {
                           switch (index) {
                               case 0:
                                   language.open()
                                   break;
                               case 1:
                                   currency.open()
                                   break;
                               case 2:
                               case 3:
                                   break;
                               case 4:
                                   stateMachine.setActionWindow(14)
                                   break;
                               case 5:
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
                               case 6:
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
