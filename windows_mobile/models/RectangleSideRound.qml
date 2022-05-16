import QtQuick 2.12

Item {
    property int rec_height
    property int rec_width
    property int rec_radius
    property string rec_color
    property bool upRound: false
    property bool downRound: false

    height: roundRect.height
    width: roundRect.width

    Rectangle {
        id: roundRect
        color: rec_color
        height: rec_height
        width: rec_width
        radius: rec_radius

    }

    Rectangle {
        id: squareRect_up
        color: rec_color
        height: roundRect.radius
        visible: upRound? false : true
        anchors.top : upRound? roundRect.top : undefined
        anchors.left :roundRect.left
        anchors.right:roundRect.right

    }

    Rectangle {
        id: squareRect_down
        color: rec_color
        height: roundRect.radius
        visible: downRound? false : true
        anchors.bottom : downRound? undefined : roundRect.bottom
        anchors.left :roundRect.left
        anchors.right:roundRect.right

    }
}

