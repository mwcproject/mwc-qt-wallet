import QtQuick 2.12
import QtQuick.Window 2.0
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.15


/*Item {
    id: control

    property bool img_visible
    property string img_source
    property alias img_height: img.height
    property alias img_color: overlay.color

    Image {
        id: img
        height: img_height
        source: img_source
        fillMode: Image.PreserveAspectFit
        visible: img_visible
    }
    ColorOverlay {
        id: overlay
           anchors.fill: img
           source: img.source
           visible: img_visible
           color: img_color
       }
}*/

Item {
    property alias img_source: img.source
    property alias img_height: img.height//: img.height
    property alias img_color: overlay.color
    property int img_rotation: 0
    height: img.height
    width: img.width
    Image {
        id: img
        height: img_height
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
