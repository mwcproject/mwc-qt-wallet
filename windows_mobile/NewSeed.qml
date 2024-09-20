import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import NewSeedBridge 1.0
import Clipboard 1.0
import "./models"

Item {
    id: control

    property bool isLongSeed: true
    property var seedWords: undefined
    property bool isInit: false

    function init(seed, hideSubmitButton) {
        isInit = true
        const words = seed.split(" ")
        seedWords = words.slice(0, words.length-1)
        button_next.visible = !hideSubmitButton
    }

    NewSeedBridge {
        id: newSeed
    }

    Clipboard {
        id: clipboard
    }

    Connections {
        target: newSeed
        onSgnShowSeedData: (seed) => {
            isInit = false
            if (seed.length < 2) {
                messagebox.open(qsTr("Getting Passphrase Failure"), qsTr("Unable to retrieve a passphrase from mwc713. " + (seed.length > 0 ? seed[0] : "")))
                return;
            }
            seedWords = seed.slice(0, seed.length)
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.bg
    }

    Text {
        id: title
        color: "#ffffff"
        text: qsTr("Seed Words")
        anchors.top: parent.top
        anchors.topMargin: dp(15)
        anchors.horizontalCenter: parent.horizontalCenter
        font.bold: true
        font.pixelSize: dp(18)
        font.letterSpacing: dp(0.5)
        font.capitalization:Font.AllUppercase
        font.family: barlow.medium
    }

    Rectangle {
        height: parent.height/28
        width: height
        anchors.left: parent.left
        anchors.leftMargin:  dp(15)
        anchors.verticalCenter: title.verticalCenter
        visible: isInit? false: true
        color: "#00000000"

        ImageColor {
            img_source: "../img/arrow.svg"
            img_height: parent.height
            img_color: "white"
            img_rotation: 180
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter

        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                stateMachine.returnBack()
            }
        }
    }

    Text {
        id: desc
        width: parent.width
        anchors.top: title.bottom
        padding: dp(25)
        text: qsTr("Please save these words on paper in the good order.\n This passphrase will allow you to recover your wallet in case of computer failure.")
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
            width: parent.width
            height: parent.height
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
                //height: rect_phrase.height

                Repeater {
                    id: rep
                    model: seedWords
                    Rectangle {
                        id: rec_seed
                        height: dp(40)
                        width: parent.width/2.5
                        radius: dp(25)
                        color: "#202020"
                        Layout.topMargin: dp(10)
                        Layout.bottomMargin: dp(10)
                        Layout.alignment: Qt.AlignHCenter
                        Rectangle {
                            id: circle_index
                            height: parent.height - dp(15)
                            width: circle_index.height
                            color: "#cccccc"
                            radius: dp(50)
                            anchors.left: parent.left
                            anchors.leftMargin: dp(10)
                            anchors.verticalCenter: parent.verticalCenter
                            Text {
                                id: index_word
                                color: "#000000"
                                text: index + 1
                                font.bold: true
                                font.pixelSize: parent.height*0.55
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                        Text {
                            id: word
                            color: "#ffffff"
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            font.capitalization: Font.AllLowercase
                            font.pixelSize: parent.height*0.4
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: modelData
                        }
                    }
                 }
            }
        }
    }

    ConfirmButton {
        id: button_next
        title: "Next"
        visible: isInit? true : false
        anchors.top: rect_phrase.bottom
        anchors.topMargin: dp(30)
        anchors.horizontalCenter: parent.horizontalCenter
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
