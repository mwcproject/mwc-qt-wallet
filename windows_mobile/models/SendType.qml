import QtQuick 2.12
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.15


Item {
    property alias img_source: img.source
    property int img_height
    property alias img_color: overlay.color
    property string mainText: ""
    property string secondaryText: ""
    property int img_rotation: 0

    Image {
        id: img
        height: img_height
        source: img_source
        rotation: img_rotation
        fillMode: Image.PreserveAspectFit
        anchors.horizontalCenter: parent.horizontalCenter
    }
    ColorOverlay {
        id: overlay
        anchors.fill: img
        rotation: img_rotation
        source: img
    }
    Text {
        id: main_text
        text: qsTr(mainText)
        font.pixelSize: parent.height/4
        color: "white"
        anchors.top: img.bottom
        anchors.topMargin: parent.height/15
        anchors.horizontalCenter: parent.horizontalCenter
    }
    Text {
        id: secondary_text
        text: qsTr(secondaryText)
        font.pixelSize: parent.height/6
        color: "grey"
        anchors.top: main_text.bottom
        //anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
    }

}

/**/
