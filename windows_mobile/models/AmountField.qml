import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import UtilBridge 1.0
import QtGraphicalEffects 1.0
import "../."







Rectangle {
    id: control
    property bool field_focus: false
    property string mainColor: Theme.field
    property alias text: field_password.text
    property alias textField: field_password
    property alias mouse: mouse
    property bool isCurrencySecond: false
    property double amountSend

    color: mainColor
    radius: dp(20)

    function setAmountSend(amount, isMax) {

        if (isMax) {
            const amount_max = send.getSpendAllAmount(selectedAccount)
            if (isCurrencySecond) {
                field_password.text = (currencyPrice*amount_max).toFixed(currencyPriceRound)
                text_numSec.text = amount_max
            } else {
                field_password.text = amount_max
                text_numSec.text = (currencyPrice*amount_max).toFixed(currencyPriceRound)
            }
            amountSend = amount_max
            return
        }
        if (isCurrencySecond) {
            let mwcAmount = (Number(amount)/currencyPrice).toFixed(9)
            text_numSec.text = mwcAmount
            amountSend = mwcAmount
            return
        } else {
            let secondAmount = (Number(amount)*currencyPrice).toFixed(currencyPriceRound)
            text_numSec.text = secondAmount
            amountSend = Number(amount)
            return
        }
    }

    TextFieldCursor {
        id: field_password
        height: parent.height/2
        width: parent.width/1.7
        anchors.left: parent.left
        leftPadding: dp(20)
        topPadding: dp(4)
        inputMethodHints: Qt.ImhDigitsOnly
        anchors.top: parent.top
        font.pixelSize: dp(18)
        echoMode: "Normal"
        color: "white"
        horizontalAlignment: Text.AlignLeft
        focus: field_focus
        colorbg: "#00000000"
        onTextChanged: setAmountSend(text, false)
        MouseArea {
            id: mouse
            anchors.fill: parent
            onClicked: {
                field_focus = true
            }
        }
    }

    Text {
        id: text_amount
        text: isCurrencySecond? currencyTicker :  "MWC"
        color: "white"
        font.bold: true
        font.capitalization: Font.AllUppercase
        font.pixelSize: dp(16)
        font.family: barlow.bold
        font.letterSpacing: dp(0.5)
        anchors.left: field_password.right
        anchors.leftMargin: dp(10)
        anchors.verticalCenter: field_password.verticalCenter
    }

    Rectangle {
        height: parent.height/2 - dp(1)
        width: parent.width/1.7
        anchors.bottom: parent.bottom
        color: "#00000000"
        Text {
            id: text_numSec
            text: ""
            color: "gray"
            font.capitalization: Font.AllUppercase
            font.pixelSize: dp(14)
            font.family: barlow.bold
            font.letterSpacing: dp(0.5)
            anchors.left: parent.left
            anchors.leftMargin: dp(20)
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            id: text_second
            text: isCurrencySecond? "MWC" : currencyTicker
            color: "gray"
            font.capitalization: Font.AllUppercase
            font.pixelSize: dp(14)
            font.family: barlow.bold
            font.letterSpacing: dp(0.5)
            anchors.left: parent.right
            anchors.leftMargin: dp(10)
            anchors.verticalCenter: parent.verticalCenter
        }
    }






   SecondaryButton {
            id: button_max
            height: field_password.height/1.5
            width: (parent.width - field_password.width - text_amount.width - dp(10)) /2
            anchors.leftMargin: dp(10)
            anchors.left: text_amount.right
            anchors.verticalCenter: field_password.verticalCenter
            title: "Max"
            color: "#181818"
            onClicked: {
                setAmountSend("", true)

            }
    }
    Rectangle {
        id: bar_separator
        height: dp(1)
        radius: dp(50)
        anchors.right: button_max.right
        //anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: field_password.bottom
        anchors.left: parent.left
        anchors.leftMargin: dp(20)
        color: "gray"
    }

    ImageColor {
        img_color: "#181818"
        img_source: "../../img/swap.svg"
        img_height: parent.height/4
        img_rotation: 90
        anchors.left: bar_separator.right
        anchors.leftMargin: (parent.width - bar_separator.width - dp(20) - width)/2
        anchors.rightMargin: (parent.width - bar_separator.width - dp(20) - width)/2
        anchors.verticalCenter: parent.verticalCenter
        MouseArea {
            id: mouseSwap
            anchors.fill: parent
            onClicked: {
                isCurrencySecond = !isCurrencySecond
                let amt2 = text_numSec.text
                field_password.text = amt2

            }
        }

    }
}




