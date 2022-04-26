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
    //property int seedLenght: undefined
    //property string seedWords: ""

    function init(seed, hideSubmitButton) {
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
            if (seed.length < 2) {
                messagebox.open(qsTr("Getting Passphrase Failure"), qsTr("Unable to retrieve a passphrase from mwc713. " + (seed.length > 0 ? seed[0] : "")))
                return;
            }
            setTextSeed(seed)
        }
    }

    Text {
        id: title
        anchors.top: parent.top
        anchors.topMargin: dp(30)
        text: "Seed Words"
        font.pixelSize: dp(22)
        font.bold: true
        color: "white"
        width: parent.width
        //elide: Text.ElideMiddle
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
                height: rect_phrase.height

                Repeater {
                    id: rep
                    model: seedWords //control.nbwords
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
        anchors.top: rect_phrase.bottom
        anchors.topMargin: dp(30)
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: {
            newSeed.doneWithNewSeed()
        }
    }

    /*ConfirmButton {
        id: button_skip
        title: "Skip"
        anchors.top: button_next.bottom
        anchors.topMargin: dp(20)
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: {
            // make a better way to skip seed
            let x = Array("","","","","","")
            newSeed.submitSeedWord(x, true)
        }
    }*/




    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: -notification.width / 2
        anchors.bottom: parent.bottom
        anchors.bottomMargin: dp(20)
        Notification {
            id: notification
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
