import QtQuick 2.0
import QtQuick.Controls 2.13
import ReceiveBridge 1.0
import Clipboard 1.0
import Qt.labs.platform 1.1

Item {
    ReceiveBridge {
        id: bridge
    }

    Clipboard {
        id: clipboard
    }

    Connections {
        target: bridge
        onDoInit: {
            accountItems.clear()
        }

        onAddAccountItem: {
            accountItems.append({text: account})
        }

        onSelectCurrentAccount: {
            accountComboBox.currentIndex = index
        }

        onDoUpdateMwcMqState: {
            image_mwcmq.source = online ? "../img/StatusOk@2x.svg" : "../img/StatusEmpty@2x.svg"
        }

        onDoUpdateKeybaseState: {
            image_keybase.source = online ? "../img/StatusOk@2x.svg" : "../img/StatusEmpty@2x.svg"
        }

        onDoUpdateHttpState: {
            image_http.source = online ? "../img/StatusOk@2x.svg" : "../img/StatusEmpty@2x.svg"
        }

        onDoUpdateMwcMqAddress: {
            mwcMqAddress.text = mwcAddress
        }

        onDoHttpsSupport: {
            text_http.text = qsTr("Http")
        }
    }

    ComboBox {
        id: accountComboBox

        onCurrentIndexChanged: {
            bridge.changeCurrentAccount(accountComboBox.currentIndex)
        }

        delegate: ItemDelegate {
            width: accountComboBox.width
            contentItem: Text {
                text: modelData
                color: "#7579ff"
                font: accountComboBox.font
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
            highlighted: accountComboBox.highlightedIndex === index
        }

        indicator: Canvas {
            id: canvas
            x: accountComboBox.width - width - accountComboBox.rightPadding
            y: accountComboBox.topPadding + (accountComboBox.availableHeight - height) / 2
            width: 12
            height: 8
            contextType: "2d"

            Connections {
                target: accountComboBox
                function onPressedChanged() { canvas.requestPaint(); }
            }

            onPaint: {
                context.reset();
                context.moveTo(0, 0);
                context.lineTo(width, 0);
                context.lineTo(width / 2, height);
                context.closePath();
                context.fillStyle = "white";
                context.fill();
            }
        }

        contentItem: Text {
            text: accountComboBox.displayText
            font: accountComboBox.font
            color: "white"
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            border.color: "white"
            implicitHeight: 40
            radius: 5
            color: "#00000000"
        }

        popup: Popup {
            y: accountComboBox.height - 1
            width: accountComboBox.width
            implicitHeight: contentItem.implicitHeight
            padding: 1

            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: accountComboBox.popup.visible ? accountComboBox.delegateModel : null
                currentIndex: accountComboBox.highlightedIndex

                ScrollIndicator.vertical: ScrollIndicator { }
            }

            background: Rectangle {
                border.color: "white"
                radius: 5
            }
        }

        model: ListModel {
            id: accountItems
        }
        anchors.top: text_http.bottom
        anchors.topMargin: 50
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 20
        leftPadding: 10
        rightPadding: 5
    }

    TextArea {
        id: mwcMqAddress
        font.pointSize: 13
        color: "white"
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignTop
        anchors.verticalCenterOffset: 75
        anchors.verticalCenter: accountComboBox.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 20
        readOnly: true
        padding: 10
        background: Rectangle {
            color: "white"
            opacity: 0.1
            radius: 5
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                clipboard.text = mwcMqAddress.text
                notification.text = "Address copied to the clipboard"
                notification.open()
            }
        }
    }

    Text {
        id: text_mwcmq
        text: qsTr("MWC Message Queue")
        anchors.verticalCenterOffset: -200
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenterOffset: -50
        anchors.horizontalCenter: parent.horizontalCenter
        font.bold: true
        font.pixelSize: 20
        color: "white"
    }

    Image {
        id: image_mwcmq
        anchors.horizontalCenterOffset: 100
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: text_mwcmq.verticalCenter
        width: 20
        height: 20
    }

    Text {
        id: text_keybase
        text: qsTr("Keybase")
        anchors.top: text_mwcmq.bottom
        anchors.topMargin: 50
        anchors.horizontalCenterOffset: -50
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: 20
        font.bold: true
        color: "white"
    }

    Image {
        id: image_keybase
        anchors.horizontalCenterOffset: 100
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: text_keybase.verticalCenter
        width: 20
        height: 20
    }

    Text {
        id: text_http
        text: qsTr("Http")
        anchors.top: text_keybase.bottom
        anchors.topMargin: 50
        anchors.horizontalCenter: parent.horizontalCenter
        font.bold: true
        font.pixelSize: 20
        anchors.horizontalCenterOffset: -50
        color: "white"
    }

    Image {
        id: image_http
        anchors.verticalCenterOffset: 0
        anchors.verticalCenter: text_http.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 100
        width: 20
        height: 20
    }

    Button {
        id: button_receivebyfile
        width: 250
        height: 40
        anchors.verticalCenterOffset: 100
        anchors.verticalCenter: mwcMqAddress.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        background: Rectangle {
            color: "#00000000"
            radius: 10
            border.color: "white"
            border.width: 2
            Text {
                text: qsTr("Receive MWC by file")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pointSize: 20
                color: "white"
            }
        }
        onClicked: {
            fileDialog.open();
        }
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Open initial transaction file")
        folder: bridge.getFileGenerationPath()
        nameFilters: ["MWC transaction (*.tx *.input);;All files (*.*)"]
        onAccepted: {
            console.log("Accepted: " + fileDialog.file)
//            bridge.updateFileGenerationPath(fileDialog.file)
//            ui->progress->show();
            bridge.signTransaction(fileDialog.file);
        }
    }

    Rectangle {
        width: 270
        anchors.verticalCenterOffset: 50
        anchors.verticalCenter: button_receivebyfile.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
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
