import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
import SendBridge 1.0
import UtilBridge 1.0

Item {
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    UtilBridge {
        id: util
    }

    SendBridge {
        id: send
    }

    Connections {
        target: send
        onSgnShowSendResult: {

        }
    }

    onVisibleChanged: {
        if (visible) {
//            ui->contactNameLable->setText("");
//            ui->contactNameLable->hide();
//            ui->apiSecretEdit->hide();
            const account = "Default"
            text_from_account.text = qsTr("From account: " + account)
            const amount = -1
            text_amount_to_send.text = qsTr("Amount to send: " + ( amount < 0 ? "All" : util.nano2one(Number(amount))) + " MWC" );
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            textfield_send_to.focus = false
            textarea_description.focus = false
        }
    }

    Image {
        id: image_send_online
        width: dp(100)
        height: dp(100)
        anchors.top: parent.top
        anchors.topMargin: dp(50)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        source: "../img/SendOnline@2x.svg"
        fillMode: Image.PreserveAspectFit
    }

    Text {
        id: text_send_to_address
        color: "#ffffff"
        text: qsTr("Send to Address")
        anchors.left: image_send_online.right
        anchors.leftMargin: dp(30)
        anchors.top: parent.top
        anchors.topMargin: dp(60)
        font.bold: true
        font.pixelSize: dp(22)
    }

    Text {
        id: text_from_account
        color: "#ffffff"
        text: qsTr("From account:")
        anchors.bottom: text_amount_to_send.top
        anchors.bottomMargin: dp(5)
        anchors.left: image_send_online.right
        anchors.leftMargin: dp(30)
        font.pixelSize: dp(15)
    }

    Text {
        id: text_amount_to_send
        color: "#ffffff"
        text: qsTr("Amount to send:")
        anchors.left: image_send_online.right
        anchors.leftMargin: dp(30)
        anchors.bottom: image_send_online.bottom
        anchors.bottomMargin: dp(5)
        font.pixelSize: dp(15)
    }

    TextField {
        id: textfield_send_to
        height: dp(50)
        padding: dp(5)
        leftPadding: dp(10)
        font.pixelSize: dp(18)
        placeholderText: qsTr("Send to")
        color: "white"
        text: ""
        anchors.top: image_send_online.bottom
        anchors.topMargin: dp(50)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.right: parent.right
        anchors.rightMargin: dp(90)
        horizontalAlignment: Text.AlignLeft
        background: Rectangle {
            color: "#8633E0"
            radius: dp(5)
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                textfield_send_to.focus = true
            }
        }
    }

    Button {
        id: button_contacts
        width: dp(50)
        height: dp(50)
        anchors.verticalCenter: textfield_send_to.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("#")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
        }
    }

    Text {
        id: text_formats
        color: "#b3ffffff"
        text: qsTr("FORMATS:    [mwcmqs://]<mqs_address>        keybase://<name>         http(s)://<host>:<port>")
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        anchors.top: textfield_send_to.bottom
        anchors.topMargin: dp(20)
        font.pixelSize: dp(12)
    }

    TextArea {
        id: textarea_description
        height: dp(200)
        padding: dp(10)
        font.pixelSize: dp(20)
        placeholderText: qsTr("Description")
        color: "white"
        text: ""
        anchors.top: text_formats.bottom
        anchors.topMargin: dp(20)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        horizontalAlignment: Text.AlignLeft
        background: Rectangle {
            color: "#8633E0"
            radius: dp(5)
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                textarea_description.focus = true
            }
        }
    }

    Button {
        id: button_settings
        width: parent.width / 2 - dp(45)
        height: dp(50)
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.top: textarea_description.bottom
        anchors.topMargin: dp(30)
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Settings")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
        }
    }

    Button {
        id: button_send
        width: parent.width / 2 - dp(45)
        height: dp(50)
        anchors.verticalCenter: button_settings.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Send")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
        }
    }

}
