import QtQuick 2.15
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
import WalletBridge 1.0
import SendBridge 1.0
import ConfigBridge 1.0
import UtilBridge 1.0

import "./models"

Item {
    id: control
    anchors.fill: parent
    property string address
    property string type

    function init(initParams) {
        console.log(initParams.address)
       qrCode.qr.source = "data:image/svg+xml;utf8,".concat(initParams.svg)
       address = initParams.address
       type = initParams.typeAddress

    }

    Rectangle {
        id: rect_qr
        anchors.fill: parent
        color: Theme.gradientTop
        ImageQrCode {
            id: qrCode
            height: parent.width/1.5
            width: parent.width/1.5
            anchors.top: parent.top
            anchors.topMargin: parent.height/14
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Label {
                id: label_address
                text: qsTr("%1 Address").arg(type)
                color: "white"
                anchors.top: qrCode.bottom
                anchors.topMargin: dp(60)
                font.capitalization: Font.AllUppercase
                font.pixelSize: dp(16)
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
            }

        Rectangle {
            id: rect_address
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: label_address.bottom
            anchors.topMargin: dp(10)
            color: "#252525"
            radius: dp(15)
            height: dp(16) + text_address.height
            width: parent.width*0.8

            TextEdit {
                id: text_address
                readOnly: true
                anchors.top: parent.top
                anchors.topMargin: dp(8)
                anchors.left: parent.left
                anchors.leftMargin: dp(10)
                anchors.right: parent.right
                anchors.rightMargin: dp(10)
                wrapMode: Text.WrapAnywhere
                font.pixelSize: dp(15)
                color: "white"
                text: address
            }
        }

        Label {
                id: label_help
                text: qsTr("Touch to copy the address")
                color: "gray"
                anchors.top: rect_address.bottom
                anchors.topMargin: dp(10)
                font.capitalization: Font.AllLowercase
                font.pixelSize: dp(13)
                anchors.horizontalCenter: parent.horizontalCenter
            }


        SecondaryButton{
            title: "Share"
            height: dp(40)
            width: dp(100)
            anchors.top: label_help.bottom
            anchors.topMargin: dp(60)
            anchors.horizontalCenter: parent.horizontalCenter

        }
    }

    /**/







}
