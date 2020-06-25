import QtQuick 2.0
import QtQuick.Controls 2.13
import Qt.labs.platform 1.1
import QtQuick.Window 2.0
import FinalizeBridge 1.0
import ConfigBridge 1.0

Item {
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160) }

    FinalizeBridge {
        id: finalize
    }

    ConfigBridge {
        id: config
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
        anchors.verticalCenterOffset: dp(-50)
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: dp(25)
    }

    Button {
        id: button_upload
        width: dp(250)
        height: dp(50)
        anchors.verticalCenterOffset: dp(50)
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
//                control::MessageBox::messageText(this, "Unable to finalize", "Your MWC Node, that wallet connected to, is not ready to finalize transactions.\n"
//                                                                             "MWC Node need to be connected to few peers and finish blocks synchronization process");
                console.log("Unable to finalize", "Your MWC Node, that wallet connected to, is not ready to finalize transactions.\nMWC Node need to be connected to few peers and finish blocks synchronization process")
                return
            }

            fileDialog.open()
        }
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Finalize transaction file")
        folder: config.getPathFor("fileGen")
        nameFilters: ["MWC response transaction (*.tx.response *.response);;All files (*.*)"]
        onAccepted: {
            console.log("Accepted: " + fileDialog.file)
            // Logic is implemented into This Window
            // It is really wrong, but also we don't want to have special state for that.
//            const fileName = QFileDialog::getOpenFileName(this, tr("Finalize transaction file"),
//                                                            config->getPathFor("fileGen"),
//                                                            tr("MWC response transaction (*.tx.response *.response);;All files (*.*)"));

//            if (fileName.length() == 0)
//                return;
//            auto fileOk = util::validateMwc713Str(fileName);
//            if (!fileOk.first) {
//                core::getWndManager()->messageTextDlg("File Path",
//                                                      "This file path is not acceptable.\n" + fileOk.second);
//                return;
//            }

//            // Update path
//            QFileInfo flInfo(fileName);
//            config->updatePathFor("fileGen", flInfo.path());

//            finalize->uploadFileTransaction(fileName);
        }
    }
}
