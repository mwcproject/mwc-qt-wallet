import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import NewSeedBridge 1.0

Item {
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    function init(wordIndex) {
        textfield_word.placeholderText = "Please enter word number " + Number(wordIndex).toString()
        textfield_word.clear()
        const words = ["angry","animal","ankle","announce",
                       "actress", "actual", "adapt", "add",
                       "addict", "address", "adjust", "admit",
                       "adult", "advance", "advice", "aerobic",
                       "affair", "afford", "afraid", "again",
                       "age", "agent", "agree", "ahead"]
        newSeed.submitSeedWord(words[wordIndex-1])
    }

    NewSeedBridge {
        id: newSeed
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            textfield_word.focus = false
        }
    }

    Text {
        id: label_description
        color: "#ffffff"
        text: qsTr("Your passphrase is important! If you lose your passphrase, your money will be permanently lost. To make sure that you have properly saved your passphrase, please retype it here:")
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        anchors.right: parent.right
        anchors.rightMargin: dp(50)
        anchors.left: parent.left
        anchors.leftMargin: dp(50)
        anchors.bottom: label_word.top
        anchors.bottomMargin: dp(50)
        font.pixelSize: dp(16)
    }

    Text {
        id: label_word
        text: qsTr("Verify Recovery Phrase")
        color: "#ffffff"
        anchors.left: parent.left
        anchors.leftMargin: dp(45)
        anchors.bottom: textfield_word.top
        anchors.bottomMargin: dp(10)
        font.pixelSize: dp(16)

    }

    TextField {
        id: textfield_word
        height: dp(50)
        padding: dp(10)
        leftPadding: dp(20)
        font.pixelSize: dp(18)
        placeholderText: qsTr("Please enter word number 1")
        color: "white"
        text: ""
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: dp(25)
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
                textfield_word.focus = true
            }
        }
    }

    Button {
        id: button_back
        height: dp(70)
        width: parent.width / 2 - dp(60)
        anchors.top: textfield_word.bottom
        anchors.topMargin: dp(40)
        anchors.left: parent.left
        anchors.leftMargin: dp(50)
        background: Rectangle {
            color: "#00000000"
            radius: dp(5)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Back")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
            newSeed.restartSeedVerification()
        }
    }

    Button {
        id: button_next
        height: dp(70)
        width: parent.width / 2 - dp(60)
        anchors.top: textfield_word.bottom
        anchors.topMargin: dp(40)
        anchors.right: parent.right
        anchors.rightMargin: dp(50)
        background: Rectangle {
            color: "#00000000"
            radius: dp(5)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Next")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
            newSeed.submitSeedWord(textfield_word.text.trim())
        }
    }
}
