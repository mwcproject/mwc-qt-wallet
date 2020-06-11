import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.13
import CoreWindowBridge 1.0

Window {
    id: window
    visible: true
    title: qsTr("MWC-Mobile-Wallet")

    CoreWindowBridge {
        id: coreWindow
    }

    Connections {
        target: coreWindow
        onSgnUpdateActionStates: {
            console.log(111, actionState);
            currentState = actionState;
        }
    }

    Rectangle
    {
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0
                color: "#9E00E7"
            }

            GradientStop {
                position: 1
                color: "#3600C9"
            }
        }
        anchors.fill: parent
    }

    property int currentState

    Inputpassword {
        id: inputpassword
        anchors.fill: parent
        visible: currentState === 3
    }

//    Wallet {
//        id: walletTab
//        anchors.fill: parent
//        visible: currentState > 3
//    }

//    Receive {
//        id: receiveTab
//        anchors.fill: parent
//        visible: currentState === 9
//    }

    Navbar {
        id: navbar
        anchors.fill: parent
        visible: currentState > 3
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:14;anchors_height:200}D{i:19;anchors_y:0}
D{i:21;anchors_width:200}
}
##^##*/
