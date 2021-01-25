import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import NewSeedBridge 1.0

Item {
    function init(seed, hideSubmitButton) {
        const words = seed.split(" ")
//        testSeed = words
        setTextSeed(words.slice(0, 24))
        button_next.visible = !hideSubmitButton
    }

    function setTextSeed(words) {
        text_seed.text = ""
        for(let i = 0; i < words.length; i++) {
            text_seed.text += words[i]
            if (i === words.length - 1)
                break
            if (i % 6 === 5) {
                text_seed.text += "\n"
            } else {
                text_seed.text += "    "
            }
        }
    }

    NewSeedBridge {
        id: newSeed
    }

    Connections {
        target: newSeed
        onSgnShowSeedData: (seed) => {
            if (seed.length < 2) {
                messagebox.open(qsTr("Getting Passphrase Failure"), qsTr("Unable to retrieve a passphrase from mwc713. " + (seed.length > 0 ? seed[0] : "")))
                return;
            }
            setTextSeed(seed)
        }
    }

    Rectangle {
        id: rect_phrase
        height: text_seed.height + dp(90)
        color: "#33bf84ff"
        anchors.top: parent.top
        anchors.topMargin: dp(20)
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
            text: qsTr("Waiting for the mnemonic passphrase from mwc713...")
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            font.capitalization: Font.AllLowercase
            anchors.top: rect_splitter.bottom
            anchors.topMargin: dp(20)
            anchors.right: parent.right
            anchors.rightMargin: dp(35)
            anchors.left: parent.left
            anchors.leftMargin: dp(35)
            font.pixelSize: dp(16)
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
        height: dp(50)
        width: dp(150)
        visible: false
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

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
