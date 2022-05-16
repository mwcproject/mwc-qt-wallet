import QtQuick 2.12
import QtQuick.Controls 2.12
ListView {
    id: listView



    width: parent.width
    height: parent.height

    focus: true
    boundsBehavior: Flickable.StopAtBounds
    section.property: "name"
    section.criteria: ViewSection.FirstCharacter
    section.delegate: ToolBar {
        height: dp(30)
        background: Rectangle {
            anchors.fill: parent
            color: "#00000000"
        }

        Label {
            id: label
            text: section
            anchors.left: parent.left
            anchors.leftMargin: dp(15)
            anchors.fill: parent
            font.pixelSize: dp(16)
            font.bold: true
            verticalAlignment: Qt.AlignVCenter
            color: "gray"
        }
    }

    delegate: ContactDelegate {
        id: delegate
        width: listView.width
    }

    model: contactModel

    ScrollBar.vertical: ScrollBar { }
}
