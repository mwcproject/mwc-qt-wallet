import QtQuick 2.0
import QtQml 2.2

Item {
    id: root

    width: size
    height: size

    property int size: 200               // The size of the circle in pixel
    property real arcBegin: 0          // start arc angle in degree
    property real arcEnd: 270             // end arc angle in degree
    property real arcOffset: 0           // rotation
    property bool isPie: false           // paint a pie instead of an arc
    property bool showBackground: false  // a full circle as a background of the arc
    property real lineWidth: 20          // width of the line
    property string colorCircle: "#252525"
    property string colorBackground: "#000000"


    property int animationDuration: 1000

    onArcBeginChanged: canvas.requestPaint()
    onArcEndChanged: canvas.requestPaint()

   Behavior on arcBegin {
       id: animationArcBegin
       enabled: true
       NumberAnimation {
           duration: root.animationDuration

           //easing.type: Easing.InCubic
       }
    }

    Behavior on arcEnd {
       id: animationArcEnd
       enabled: true
       NumberAnimation {
           duration: root.animationDuration
           //easing.type: Easing.InQuad
       }
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        rotation: -90 + parent.arcOffset

        onPaint: {
            var ctx = getContext("2d")
            var x = width / 2
            var y = height / 2
            var start = Math.PI * (parent.arcBegin / 180)
            var end = Math.PI * (parent.arcEnd / 180)
            ctx.reset()

            if (root.isPie) {
                if (root.showBackground) {
                    ctx.beginPath()
                    ctx.fillStyle = root.colorBackground
                    ctx.moveTo(x, y)
                    ctx.arc(x, y, width / 2, 0, Math.PI * 2, false)
                    ctx.lineTo(x, y)
                    ctx.fill()
                }
                ctx.beginPath()
                ctx.fillStyle = root.colorCircle
                ctx.moveTo(x, y)
                ctx.arc(x, y, width / 2, start, end, false)
                ctx.lineTo(x, y)
                ctx.fill()
            } else {
                if (root.showBackground) {
                    ctx.beginPath();
                    ctx.arc(x, y, (width / 2) - parent.lineWidth / 2, 0, Math.PI * 2, false)
                    ctx.lineWidth = root.lineWidth
                    ctx.strokeStyle = root.colorBackground
                    ctx.stroke()
                }
                ctx.beginPath();
                ctx.arc(x, y, (width / 2) - parent.lineWidth / 2, start, end, false)
                ctx.lineWidth = root.lineWidth
                ctx.strokeStyle = root.colorCircle
                ctx.stroke()
            }
        }
    }
    Timer {
            running: true; repeat: true; interval: 1000
             onTriggered: {
                 arcBegin+= 270
                 arcEnd += 270
                 //root.state = root.state === "start" ? "end": "start"
             }
            //onTriggered: root.state = root.state === "start" ? "end": "start"
        }

    /*states: [
        State {
            name: "start"
            PropertyChanges {
                target: root; arcBegin: arcBegin +270; arcEnd: arcEnd +270
            }
        },
        State {
            name: "end"
            PropertyChanges {
                target: root; arcBegin: arcBegin +270; arcEnd: arcEnd +270
            }
        }]

    transitions:
    Transition {
        PropertyAnimation {
            properties: "arcBegin";
            easing.type: Easing.InQuad
            duration: animationDuration
        }
        PropertyAnimation {
            properties: "arcEnd";
            //easing.type: Easing.InQuad
            duration: 1000
        }
    }*/
}
