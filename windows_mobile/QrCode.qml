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

    function init(initParams) {
       qrCode.qr.source = "data:image/svg+xml;utf8,".concat(initParams.svg)

    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop {
                position: 0
                color: "#4d1d4f"
            }

            GradientStop {
                position: 0.3
                color: "#181818"
            }
        }
        ImageQrCode {
            id: qrCode
            height: parent.width/1.5
            width: parent.width/1.5
            anchors.top: parent.top
            anchors.topMargin: parent.height/14
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }



}
