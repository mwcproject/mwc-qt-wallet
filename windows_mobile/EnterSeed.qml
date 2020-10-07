import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Window 2.0
import InitAccountBridge 1.0
import UtilBridge 1.0

Item {
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    InitAccountBridge {
        id: initAccount
    }

    UtilBridge {
        id: util
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            textfield_seed.focus = false
        }
    }

    Image {
        id: image_phrase
        source: "../img/RecoveryPhrase@2x.svg"
        width: dp(20)
        height: dp(20)
        anchors.left: parent.left
        anchors.leftMargin: dp(50)
        anchors.bottom: rect_splitter.top
        anchors.bottomMargin: dp(15)
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
        anchors.leftMargin: dp(50)
        anchors.right: parent.right
        anchors.rightMargin: dp(50)
        anchors.bottom: textfield_seed.top
        anchors.bottomMargin: dp(25)
    }

    TextField {
        id: textfield_seed
        height: dp(300)
        padding: dp(20)
        anchors.left: parent.left
        anchors.leftMargin: dp(50)
        anchors.right: parent.right
        anchors.rightMargin: dp(50)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: dp(-40)
        color: "#ffffff"
        text: ""
        placeholderText: "Please enter your entire \nrecovery phrase here"
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        font.pixelSize: dp(22)
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignTop

        background: Rectangle {
            color: "#8633E0"
            radius: dp(5)
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                textfield_seed.focus = true
            }
        }
    }

    Button {
        id: button_next
        height: dp(70)
        width: dp(180)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: textfield_seed.bottom
        anchors.topMargin: dp(80)
        background: Rectangle {
            color: "#00000000"
            radius: dp(5)
            border.color: "white"
            border.width: dp(2)
            Text {
                id: loginText
                text: qsTr("Submit")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
            const seedStr = textfield_seed.text.toLowerCase().toString().trim()
            const seed = util.parsePhrase2Words(seedStr)

            if (seed.length !== 24) {
                messagebox.open(qsTr("Verification error"), qsTr("Your phrase should contain 24 words. You entered " + Number(seed.length).toString() +  " words."))
                return
            }

            const words = util.getBip39words()
            let nonDictWord = "";

            seed.forEach(s => {
                 if (words.indexOf(s) === -1) {
                     if (nonDictWord !== "")
                         nonDictWord += ", "
                     nonDictWord += s
                 }
             })

            if (nonDictWord !== "") {
                messagebox.open(qsTr("Verification error"), qsTr("Your phrase contains non dictionary words: " + nonDictWord))
                return
            }

            initAccount.createWalletWithSeed(seed)
        }
    }

    MessageBox {
        id: messagebox
        anchors.verticalCenter: parent.verticalCenter
    }
}
