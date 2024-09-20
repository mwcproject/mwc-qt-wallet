import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import UtilBridge 1.0
import QtGraphicalEffects 1.0


Item {
    property alias qr: img

    Rectangle {
        id: rect_qrcode
        anchors.fill: parent
        color: "white"
        radius: dp(20)

        Image {
            id: img
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            sourceSize.width: parent.width/1.10
            sourceSize.height: parent.width/1.10
            source: ""//img
            fillMode: Image.PreserveAspectFit

        }
        Rectangle {
            id: rect_logo
            anchors.horizontalCenter: img.horizontalCenter
            anchors.verticalCenter: img.verticalCenter
            height: img.width/3.2
            width: img.width/3.2
            color: "white"
            radius: dp(150)
            Rectangle {
                id: circle_logo
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                height: parent.width/1.2
                width: parent.width/1.2
                color: "black"
                radius: dp(150)
                ImageColor {
                    id: logo
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    img_width: parent.width/1.5
                    img_source: "../../img/mwc-logo.svg"
                    img_color: "#ffffff"
                }
            }
        }
    }
}

/**/
