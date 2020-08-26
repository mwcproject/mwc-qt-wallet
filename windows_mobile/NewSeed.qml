import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import NewSeedBridge 1.0

Item {
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    function init(seed, hideSubmitButton) {
        text_seed.text = seed.replace(/\s/g, '    ')
//        testSeed = seed.split(" ")
        button_next.visible = !hideSubmitButton
    }

    NewSeedBridge {
        id: newSeed
    }

    Connections {
        target: newSeed
        onSgnShowSeedData: {
            console.log("Retrieving seeds from mwc713: ", seed)
//            if (seed.size()<2) {
//                control::MessageBox::messageText( this, "Getting Passphrase Failure", "Unable to retrieve a passphrase from mwc713. " + (seed.size()>0 ? seed[0] : "") );
//                return;
//            }
//            updateSeedData("Mnemonic passphrase:", seed);
        }
    }

    Rectangle {
        id: rect_phrase
        height: text_seed.height + dp(90)
        color: "#33bf84ff"
        anchors.verticalCenterOffset: -rect_phrase.height/2 - dp(20)
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: dp(15)
        anchors.left: parent.left
        anchors.leftMargin: dp(15)

        Image {
            id: image_phrase
            source: "../img/RecoveryPhrase@2x.svg"
            width: dp(20)
            height: dp(20)
            anchors.left: parent.left
            anchors.leftMargin: dp(35)
            anchors.top: parent.top
            anchors.topMargin: dp(30)
        }

        Text {
            id: label_phrase
            text: qsTr("Recovery Phrase")
            anchors.verticalCenter: image_phrase.verticalCenter
            anchors.left: image_phrase.right
            anchors.leftMargin: dp(10)
            font.bold: true
            font.pixelSize: dp(16)
            color: "white"
        }

        Rectangle {
            id: rect_splitter
            height: dp(1)
            color: "white"
            anchors.left: parent.left
            anchors.leftMargin: dp(35)
            anchors.right: parent.right
            anchors.rightMargin: dp(35)
            anchors.top: image_phrase.bottom
            anchors.topMargin: dp(15)
        }

        Text {
            id: text_seed
            color: "#ffffff"
            text: qsTr("New Seed")
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            anchors.top: rect_splitter.bottom
            anchors.topMargin: dp(20)
            anchors.right: parent.right
            anchors.rightMargin: dp(35)
            anchors.left: parent.left
            anchors.leftMargin: dp(35)
            font.pixelSize: dp(18)
            lineHeight: 2.0
        }
    }

    Text {
        id: label_description
        color: "#ffffff"
        text: qsTr("Please save these words on paper (order is important). This passphrase will allow you to recover your wallet in case of computer failure.\n\nWARNING\n- Never disclose your passphrase.\n- Never type it on a website.\n- Do not store it electronically")
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        anchors.right: parent.right
        anchors.rightMargin: dp(50)
        anchors.left: parent.left
        anchors.leftMargin: dp(50)
        anchors.top: rect_phrase.bottom
        anchors.topMargin: dp(30)
        font.pixelSize: dp(14)
    }

    Button {
        id: button_next
        height: dp(70)
        width: dp(180)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: label_description.bottom
        anchors.topMargin: dp(50)
        background: Rectangle {
            id: rectangle
            color: "#00000000"
            radius: dp(5)
            border.color: "white"
            border.width: dp(2)
            Text {
                id: loginText
                text: qsTr("Next")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
            newSeed.doneWithNewSeed()
        }
    }
}
