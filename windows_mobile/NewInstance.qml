import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Window 2.0

Item {
    property int currentStep: 0
//    property var testSeed: []

    function updateCurrentStep(params) {
        currentStep = params.currentStep
        if (currentStep === 1) {
            newSeedItem.init(params.seed, params.hideSubmitButton)
        } else if (currentStep === 2) {
            newSeedTestItem.init(params.wordIndex)
        }
    }

    Image {
        id: image_logo
        width: dp(105)
        fillMode: Image.PreserveAspectFit
        source: "../img/mwc-logo.svg"
        anchors.top: parent.top
        anchors.topMargin: dp(50)
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Rectangle {
        color: "#00000000"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: dp(120)
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: dp(110)

        InitAccount {
            id: initAccountItem
            anchors.fill: parent
            visible: currentStep === 0
        }

        NewSeed {
            id: newSeedItem
            anchors.fill: parent
            visible: currentStep === 1
        }

        NewSeedTest {
            id: newSeedTestItem
            anchors.fill: parent
            visible: currentStep === 2
        }

        EnterSeed {
            id: enterSeedItem
            anchors.fill: parent
            visible: currentStep === 3
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

//    MouseArea {
//        anchors.left: image_help.left
//        anchors.top: image_help.top
//        height: dp(30)
//        width: text_help.width + dp(50)
//        onClicked: {
//            console.log("help instance clicked")
//        }
//    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:4;anchors_width:248;anchors_x:220}D{i:7;anchors_width:150;anchors_x:270}
}
##^##*/
