import QtQuick 2.0
import QtQuick.Controls 2.13
import Qt.labs.platform 1.1
import QtQuick.Window 2.0
import FinalizeBridge 1.0
import ConfigBridge 1.0
import UtilBridge 1.0
import QtAndroidService 1.0

Item {
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

    function slatepackCallback(ok, slatepack, slateJson, sender) {
        if (ok) {
            finalize.uploadSlatepackTransaction(slatepack, slateJson, sender)
        }
    }

    Image {
        id: image_finalize
        width: dp(80)
        height: dp(80)
        anchors.bottom: text_finalize.top
        anchors.bottomMargin: dp(30)
        anchors.horizontalCenter: parent.horizontalCenter
        fillMode: Image.PreserveAspectFit
        source: "../img/FinalizeL@2x.svg"
    }

    Text {
        id: text_finalize
        color: "#ffffff"
        text: qsTr("Finalize Transaction")
        anchors.verticalCenterOffset: dp(-100)
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: dp(25)
    }

    Button {
        id: button_upload
        width: dp(250)
        height: dp(50)
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Upload a File")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
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

    Connections {
        target: qtAndroidService
        onSgnOnFileReady: (eventCode, path ) => {
            if (eventCode == 124 && path) {
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

    Button {
        id: button_slatepack
        width: dp(250)
        height: dp(50)
        anchors.verticalCenterOffset: dp(100)
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        background: Rectangle {
            color: "#00000000"
            radius: dp(4)
            border.color: "white"
            border.width: dp(2)
            Text {
                text: qsTr("Paste Slatepack")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(18)
                color: "white"
            }
        }

        onClicked: {
            inputSlatepack.open("SendResponse", "Send response slate", 2, slatepackCallback)
        }
    }
}
