import QtQuick 2.15
import QtQuick.Controls 2.15
import Qt.labs.platform 1.1
import QtQuick.Window 2.0
import FinalizeBridge 1.0
import ConfigBridge 1.0
import UtilBridge 1.0
import QtAndroidService 1.0
import "./models"

Item {

    property int selectedFinalizeMethod: 2
    FinalizeBridge {
        id: finalize
    }

    ConfigBridge {
        id: config
    }

    UtilBridge {
        id: util
    }

    QtAndroidService {
        id: qtAndroidService
    }

    Connections {
        target: qtAndroidService
        onSgnOnFileReady: (eventCode, path ) => {
            if (eventCode === 124 && path) {
                        console.log("Open finalize transaction file: " + path)
                        const validation = util.validateMwc713Str(path)
                        if (validation) {
                            messagebox.open(qsTr("File Path"), qsTr("This file path is not acceptable.\n" + validation))
                            return
                        }
                        config.updatePathFor("fileGen", path)
                        finalize.uploadFileTransaction(path);
            }
        }
    }

    onVisibleChanged: {
        selectFinalizeMethod(selectedFinalizeMethod)
    }

    function selectFinalizeMethod(finalizeMethod) {
        selectedFinalizeMethod = finalizeMethod
        switch (selectedFinalizeMethod) {
            case 2: // file_id
                rec_file.color = "#252525"
                rec_slate.color = "#00000000"
                break;
            case 3: // slatepack_id
                rec_file.color = "#00000000"
                rec_slate.color = "#252525"
                 break;
        }

    }

    function slatepackCallback(ok, slatepack, slateJson, sender) {
        if (ok) {
            finalize.uploadSlatepackTransaction(slatepack, slateJson, sender)
        }
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
    }

    Rectangle {
        id: rect_finalizeType
        anchors.top: parent.top
        anchors.topMargin: parent.height/14
        anchors.horizontalCenter: parent.horizontalCenter
        radius: dp(25)
        color: "#191919"
        height: parent.height/6
        width: parent.width/1.7

        Rectangle {
            id: rec_file
            height: parent.height - dp(16)
            width: parent.width/2 - dp(16)
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: dp(8)
            color: "#252525"
            radius: dp(25)
            SendType {
                id: file
                height: parent.height/1.7
                width: parent.width/1.7
                img_height: file.height/2
                img_source: "../../img/file.svg"
                mainText: qsTr("File")
                secondaryText: qsTr("Manual")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                img_color: "#ffffff"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    selectFinalizeMethod(2)
                }
            }
        }

        Rectangle {
            id: rec_slate
            height: parent.height - dp(16)
            width: parent.width/2 - dp(16)
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: dp(8)
            color: "#00000000"
            radius: dp(25)
            SendType {
                id: slate
                height: parent.height/1.7
                width: parent.width/1.7
                img_height: slate.height/2
                img_source: "../../img/slate.svg"
                mainText: qsTr("Slatepack")
                secondaryText: qsTr("Manual")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                img_color: "#ffffff"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    selectFinalizeMethod(3)
                }
            }
        }
    }

    Button {
        id: button_file
        visible: selectedFinalizeMethod === 2? true : false
        height: dp(50)
        width: parent.width/2
        anchors.top: rect_finalizeType.bottom
        anchors.topMargin: dp(80)
        anchors.horizontalCenter: parent.horizontalCenter
        background: Rectangle {
            color: "#252525"
            radius: dp(25)
            Text {
                text: qsTr("Receive by File")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(17)
                color: "white"
            }
        }
        onClicked: {
            if (!finalize.isNodeHealthy()) {
                messagebox.open(qsTr("Unable to finalize"), qsTr("Your MWC Node, that wallet connected to, is not ready to finalize transactions.\nMWC Node need to be connected to few peers and finish blocks synchronization process"))
                return
            }
            if (qtAndroidService.requestPermissions()) {
                qtAndroidService.openFile( config.getPathFor("fileGen"), "*/*", 124 )
            } else {
                messagebox.open("Failure", "Permission Denied")
            }
        }
    }

    InputSlatepack {
        anchors.top: rect_finalizeType.bottom
        anchors.topMargin: dp(40)
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        visible: selectedFinalizeMethod === 3 ? true: false
    }



        /*onClicked: {
            inputSlatepack.open("SendResponse", "Send response slate", 2, slatepackCallback)
        }*/
}
