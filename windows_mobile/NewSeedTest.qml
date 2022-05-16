import QtQuick 2.15
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.15
import QtQuick.Window 2.0
import NewSeedBridge 1.0
import "./models"

Item {
    property bool isLongSeed: true
    property var seedWordsIndex: undefined

    function init(wordIndex) {
        console.log("log_: " + wordIndex)
        seedWordsIndex = wordIndex//.slice(0, wordIndex.length)
    }

    NewSeedBridge {
        id: newSeed
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            console.log("index_: "+ seedWordsIndex.length)
            for (let i = 0; i<= seedWordsIndex.length; i++) {
                console.log("iteamAt: "+ rep.itemAt(i).text)
                console.log("iteamAt: "+ rep.itemAt(i).field_focus)
            }
            rep.itemAt(rep.index).field_focus = false

        }
    }

    Text {
        id: title
        anchors.top: parent.top
        anchors.topMargin: dp(30)
        text: "Confirm Seed Words"
        font.pixelSize: dp(22)
        font.bold: true
        color: "white"
        width: parent.width
        elide: Text.ElideMiddle
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WrapAnywhere

    }

    Text {
        id: desc
        width: parent.width
        anchors.top: title.bottom
        padding: dp(25)
        text: qsTr("Please save these words on paper (order is important). This passphrase will allow you to recover your wallet in case of computer failure.")
        font.pixelSize: dp(15)
        color: "#c2c2c2"
        elide: Text.ElideLeft
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap

    }

    Rectangle {
        id: rect_phrase
        height: parent.height/1.5
        width: parent.width
        color: grid_seed.Layout.minimumHeight <= height ? "#00000000" :"#0f0f0f"
        anchors.top: desc.bottom

        Flickable {
            id: scroll
            height: parent.height
            width: parent.width
            contentHeight: grid_seed.Layout.minimumHeight
            boundsMovement: Flickable.StopAtBounds
            clip: true
            ScrollBar.vertical: ScrollBar {
                policy: grid_seed.Layout.minimumHeight <= rect_phrase.height ? Qt.ScrollBarAlwaysOff : Qt.ScrollBarAlwaysOn
            }


            GridLayout {
                id: grid_seed
                columns: 2
                columnSpacing: 0
                rowSpacing: dp(5)
                width: rect_phrase.width

                Repeater {
                    id: rep
                    model: seedWordsIndex
                    SeedInput {
                        id: seed_input
                        height: dp(40)
                        width: parent.width/2.5
                        radius: dp(25)
                        text_index: modelData
                        Layout.topMargin: dp(10)
                        Layout.bottomMargin: dp(10)
                        Layout.alignment: Qt.AlignHCenter

                    }
                }
            }
        }

        ConfirmButton {
            id: button_next
            title: "Next"
            anchors.top: rect_phrase.bottom
            anchors.topMargin: dp(30)
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                let x = Array()
                for (let i = 0; i<= seedWordsIndex.length -1 ; i++) {
                    let word = rep.itemAt(i).text.toLowerCase()
                    let filter = word.trim()
                    x.push(filter)
                }
                newSeed.submitSeedWord(x, false)
            }
        }
    }
}
