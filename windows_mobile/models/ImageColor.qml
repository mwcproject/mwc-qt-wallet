import QtQuick 2.12
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.15


Item {
    property alias img_source: img.source
    property alias img_height: img.height
    property alias img_width: img.width
    property alias img_color: overlay.color
    property int img_rotation: 0
    height: img.height
    width: img.width
    Image {
        id: img
        source: img_source
        rotation: img_rotation
        fillMode: Image.PreserveAspectFit
    }
    ColorOverlay {
        id: overlay
        anchors.fill: img
        rotation: img_rotation
        source: img
    }
}

/**/
