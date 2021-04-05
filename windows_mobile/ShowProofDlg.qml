import QtQuick 2.0
import QtQuick.Controls 2.13
import ConfigBridge 1.0

Item {
    property string blockExplorerUrl
    property var proof
    property bool isTxDetailView: false

    id: showProofDlg
    visible: false

    ConfigBridge {
        id: config
    }

    function parseProofText(proof) {
        // this file proves that [0.100000000] MWCs was sent to [xmgEvZ4MCCGMJnRnNXKHBbHmSGWQchNr9uZpY5J1XXnsCFS45fsU] from [xmiuyC3sdhXpJnR7pvQ8xNgZLWQRQziZ1FxhEQd8urYWvSusuC69]
        const idx0 = proof.indexOf("this file proves that");
        if (idx0<0)
            return false

        const mwcIdx1 = proof.indexOf('[', idx0)
        const mwcIdx2 = proof.indexOf(']', mwcIdx1)

        const addrToIdx1 = proof.indexOf('[', mwcIdx2)
        const addrToIdx2 = proof.indexOf(']', addrToIdx1)
        const addrFromIdx1 = proof.indexOf('[', addrToIdx2)
        const addrFromIdx2 = proof.indexOf(']', addrFromIdx1)

        const outputIdx0 = proof.indexOf( "outputs:", addrFromIdx2)
        const outputIdx1 = proof.indexOf( ':', outputIdx0)
        const outputIdx2 = proof.indexOf( ':', outputIdx1 + 1)

        const kernelIdx0 = proof.indexOf( "kernel:", outputIdx2)
        const kernelIdx1 = proof.indexOf( ':', kernelIdx0)
        const kernelIdx2 = proof.indexOf( ':', kernelIdx1 + 1)

        if ( idx0<0 || mwcIdx1<0 || mwcIdx2<0 || addrToIdx1<0 || addrToIdx2<0 || addrFromIdx1<0 || addrFromIdx2<0 ||
                     outputIdx0<0 || outputIdx1<0 || outputIdx2<0 ||
                     kernelIdx0<0 || kernelIdx1<0 || kernelIdx2<0 )
            return false

        let mwc = proof.substring(mwcIdx1+1, mwcIdx2 )
        while (mwc.length > 0 && mwc[mwc.length - 1] === '0')
            mwc = mwc.substring(0, mwc.length - 1 )

        if (mwc.length > 0 && mwc[mwc.length - 1] === '.')
            mwc = mwc.substring(0, mwc.length - 1 )

        const toAddress = proof.substring( addrToIdx1+1, addrToIdx2)
        const fromAddress = proof.substring( addrFromIdx1+1, addrFromIdx2)

        const output = proof.substring( outputIdx1+1, outputIdx2).trim()
        const kernel = proof.substring( kernelIdx1+1, kernelIdx2).trim()

        if ( mwc === "" || toAddress === "" || fromAddress === "" || output === "" || kernel === "" )
            return false

        return {
            mwc,
            toAddress,
            fromAddress,
            output,
            kernel
        }
    }

    function open(fileName, proofInfo, isExport = false) {
        proof = proofInfo
        text_location.text = fileName
        text_info.text = 'this file proves that <b>'+ proofInfo.mwc + 'MWCs</b> was sent to<br /><b>' + proofInfo.toAddress + '</b><br/>from<br/><b>' + proofInfo.fromAddress
        text_outputs.text = proofInfo.output
        text_kernel.text = proofInfo.kernel
        blockExplorerUrl = config.getBlockExplorerUrl(config.getNetwork())
        rect_dialog.height = dp(520) + text_location.height + text_warning.height
        isTxDetailView = transactionDetail.visible
        transactionDetail.visible = false
        showProofDlg.visible = true
        if (isExport) {
            text_title.text = "Proof Exported"
        } else {
            text_title.text = "Transaction Proof"
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#00000000"

        MouseArea {
            anchors.fill: parent
        }
    }

    Rectangle {
        id: rect_dialog
        anchors.left: parent.left
        anchors.leftMargin: dp(30)
        anchors.right: parent.right
        anchors.rightMargin: dp(30)
        anchors.verticalCenter: parent.verticalCenter

        Rectangle {
            id: rectangle
            color: "#ffffff"
            anchors.fill: parent

            Text {
                id: text_title
                text: qsTr("Transaction Proof")
                font.bold: true
                anchors.top: parent.top
                anchors.topMargin: dp(25)
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: dp(22)
                color: "#3600c9"
            }

            Text {
                id: label_location
                color: "#3600c9"
                text: qsTr("Location")
                anchors.left: parent.left
                anchors.leftMargin: dp(20)
                anchors.top: text_title.bottom
                anchors.topMargin: dp(25)
                font.bold: true
                font.pixelSize: dp(15)
            }

            Text {
                id: text_location
                color: "#3600c9"
                text: qsTr("proof.proof")
                anchors.left: parent.left
                anchors.topMargin: dp(3)
                anchors.top: label_location.bottom
                font.pixelSize: dp(15)
                anchors.leftMargin: dp(20)
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }

            Rectangle {
                id: rect_info
                anchors.top: text_location.bottom
                anchors.topMargin: dp(20)
                anchors.left: parent.left
                anchors.leftMargin: dp(20)
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                border.color: "#9E00E7"
                border.width: dp(1)
                height: dp(150)

                Text {
                    id: text_info
                    anchors.fill: parent
                    anchors.topMargin: dp(15)
                    anchors.leftMargin: dp(20)
                    anchors.rightMargin: dp(20)
                    text: "html text"
                    font.pixelSize: dp(15)
                    horizontalAlignment: Text.AlignHCenter
                    lineHeight: 1.2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    color: "#3600c9"

                }
            }

            Text {
                id: label_outputs
                color: "#3600c9"
                text: qsTr("outputs")
                anchors.left: parent.left
                anchors.leftMargin: dp(20)
                anchors.top: rect_info.bottom
                anchors.topMargin: dp(20)
                font.bold: true
                font.pixelSize: dp(15)
            }

            Text {
                id: text_outputs
                color: "#3600c9"
                text: qsTr("outputs")
                elide: Text.ElideMiddle
                anchors.left: parent.left
                anchors.topMargin: dp(3)
                anchors.top: label_outputs.bottom
                anchors.leftMargin: dp(20)
                anchors.right: button_outputs_view.left
                anchors.rightMargin: dp(20)
                font.pixelSize: dp(15)
            }

            Button {
                id: button_outputs_view
                height: dp(35)
                width: dp(80)
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                anchors.bottom: text_outputs.bottom
                background: Rectangle {
                    color: "#00000000"
                    radius: dp(4)
                    border.color: "#3600C9"
                    border.width: dp(2)
                    Text {
                        text: qsTr("View")
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: dp(14)
                        color: "#3600C9"
                    }
                }

                onClicked: {
                    Qt.openUrlExternally("https://" + blockExplorerUrl + "/#o" + proof.output)
                }
            }

            Text {
                id: label_kernel
                color: "#3600c9"
                text: qsTr("kernel")
                anchors.left: parent.left
                anchors.leftMargin: dp(20)
                anchors.top: button_outputs_view.bottom
                anchors.topMargin: dp(20)
                font.bold: true
                font.pixelSize: dp(15)
            }

            Text {
                id: text_kernel
                color: "#3600c9"
                text: qsTr("kernel")
                elide: Text.ElideMiddle
                anchors.left: parent.left
                anchors.topMargin: dp(3)
                anchors.top: label_kernel.bottom
                anchors.leftMargin: dp(20)
                anchors.right: button_kernel_view.left
                anchors.rightMargin: dp(20)
                font.pixelSize: dp(15)
            }

            Button {
                id: button_kernel_view
                height: dp(35)
                width: dp(80)
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                anchors.bottom: text_kernel.bottom
                background: Rectangle {
                    color: "#00000000"
                    radius: dp(4)
                    border.color: "#3600C9"
                    border.width: dp(2)
                    Text {
                        text: qsTr("View")
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: dp(14)
                        color: "#3600C9"
                    }
                }

                onClicked: {
                    Qt.openUrlExternally("https://" + blockExplorerUrl + "/#k" + proof.output)
                }
            }

            Text {
                id: label_warning
                color: "#3600c9"
                text: qsTr("WARNING:")
                anchors.left: parent.left
                anchors.leftMargin: dp(20)
                anchors.top: button_kernel_view.bottom
                anchors.topMargin: dp(20)
                font.bold: true
                font.pixelSize: dp(15)
            }

            Text {
                id: text_warning
                color: "#3600c9"
                text: qsTr("This proof should only be considered valid if the kernel is actually on-chain with sufficient confirmations. Please use a MWC block explorer to verify this is the case by pressing 'View' buttons")
                anchors.left: parent.left
                anchors.leftMargin: dp(20)
                anchors.top: label_warning.bottom
                anchors.topMargin: dp(3)
                anchors.right: parent.right
                anchors.rightMargin: dp(20)
                font.pixelSize: dp(15)
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }

            Button {
                id: button_ok
                height: dp(40)
                width: dp(135)
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: text_warning.bottom
                anchors.topMargin: dp(25)
                background: Rectangle {
                    color: "#6F00D6"
                    radius: dp(4)
                    Text {
                        text: qsTr("OK")
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: dp(15)
                        color: "white"
                    }
                }

                onClicked: {
                    showProofDlg.visible = false
                    if (isTxDetailView) {
                        isTxDetailView = false
                        transactionDetail.visible = true
                    }
                }
            }
        }
    }
}
