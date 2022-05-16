import QtQuick 2.15
import QtQuick.Window 2.0
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtCharts 2.15
import QtGraphicalEffects 1.15
import StateMachineBridge 1.0
import NodeInfoBridge 1.0
import WalletBridge 1.0
import QtQuick3D.Helpers 1.15
import QtQuick3D 1.15
import UtilBridge 1.0
import WalletConfigBridge 1.0
import "utils.js" as Request
import "./models"


Item {

    property var priceList
    property var networkStats
    property var marketChart
    property bool isInitWallet: true

    property string lastShownErrorMessage
    property int nodeHeightDiffLimit: 5
    property int heightBlock

    property var chartPointPos: []
    property int chartRange: 3


    StateMachineBridge {
        id: stateMachine
    }

    UtilBridge {
        id: util
    }

    WalletBridge {
        id: wallet
    }

    WalletConfigBridge {
        id: walletConfig
    }

    NodeInfoBridge {
        id: nodeInfo
    }

    Connections {
        target: nodeInfo
        onSgnSetNodeStatus: (localNodeStatus, online, errMsg, nodeHeight, peerHeight, totalDifficulty2show, connections) => {
            if (!online) {
                text_indicator.text = "Offline"
                indicator_node.color = Theme.indicatorOffline
                text_peers.text = "-"
                text_height.text = "-"
                text_diff.text = "-"
                if (lastShownErrorMessage !== errMsg) {
                    messagebox.open(qsTr("MWC Node connection error"), qsTr("Unable to retrieve MWC Node status.\n" + errMsg))
                    lastShownErrorMessage = errMsg
                }
            } else {
                if (nodeHeight + nodeHeightDiffLimit < peerHeight) {
                    text_indicator.text = "Syncing"
                    indicator_node.color = Theme.indicatorPending
                }
                else {
                    text_indicator.text= "Online"
                    indicator_node.color = Theme.indicatorOnline
                }

                if (connections <= 0) {
                    text_peers.text = "None"
                    //text_connections.color = "#CCFF33"
                }
                else {
                    text_peers.text = Number(connections).toString()
                    //text_connections.color = Theme.textPrimary
                }
                text_diff.text = totalDifficulty2show
                text_height.text = util.longLong2Str(nodeHeight)
                heightBlock = nodeHeight
                console.log("hight " + heightBlock + " node " + nodeHeight)
            }
        }
    }

    Timer {
        id: timer
        interval: 10*60*1000 // milliseconds // Update every 10 Minutes
        running: true
        repeat: true
        onTriggered: {
            Request.getPrice()
            Request.getNetworkStats()
            setChart()
        }
    }


    /*Timer {
        id: timer_startup
        interval: 10*1000 // milliseconds // Update every 10 Minutes
        running: true
        repeat: false
        onTriggered: {

        }
    }*/

    Connections {
        target: wallet

        onSgnMwcAddressWithIndex: {
            updateStatus()
        }

        onSgnTorAddress: {
            updateStatus()
        }

        onSgnUpdateListenerStatus: {
            updateStatus()
        }
    }

    onVisibleChanged: {
        if (visible) {
            updateStatus()
            if (isInitWallet) {
                hiddenAmount = walletConfig.getWalletAmountHiddenEnabled()
                let currency_idx = walletConfig.getPriceCurrency()
                if (currency_idx !== -1) {
                    currencyIndex(currency_idx)
                }
                Request.getPrice()
                transactionsItem.requestTransactions()
                receiveItem.requestAddress()
                Request.getNetworkStats()
                isInitWallet = false
            }
        }
    }

    function setChart() {
        switch (chartRange) {
        case 1:
            text_d1.color = "purple"
            text_d7.color = Theme.textPrimary
            text_m.color = Theme.textPrimary
            text_m3.color = Theme.textPrimary
            text_y.color = Theme.textPrimary
            Request.getMarketChart(currencyTicker, 1, 600)
            break;
        case 2:
            text_d1.color = Theme.textPrimary
            text_d7.color =  Theme.chartSelectedRange
            text_m.color = Theme.textPrimary
            text_m3.color = Theme.textPrimary
            text_y.color = Theme.textPrimary
            Request.getMarketChart(currencyTicker, 7, 3000)
            break;
        case 3:
            text_d1.color = Theme.textPrimary
            text_d7.color = Theme.textPrimary
            text_m.color = Theme.chartSelectedRange
            text_m3.color = Theme.textPrimary
            text_y.color = Theme.textPrimary
            Request.getMarketChart(currencyTicker, 30, "daily")
            break;
        case 4:
            text_d1.color = Theme.textPrimary
            text_d7.color = Theme.textPrimary
            text_m.color = Theme.textPrimary
            text_m3.color = Theme.chartSelectedRange
            text_y.color = Theme.textPrimary
            Request.getMarketChart(currencyTicker, 90, "daily")
            break;
        case 5:
            text_d1.color = Theme.textPrimary
            text_d7.color = Theme.textPrimary
            text_m.color = Theme.textPrimary
            text_m3.color = Theme.textPrimary
            text_y.color = Theme.chartSelectedRange
            Request.getMarketChart(currencyTicker, 365, "daily")
            break;
        }
    }

    function updateStatus() {
        let mqs = wallet.getMqsListenerStatus()
        let tor = wallet.getTorListenerStatus()
        if (tor && mqs) {
            indicator_listener.color = Theme.indicatorOnline
            text_indicator_listener.text = "TOR | MQS"
        } else if (mqs) {
            indicator_listener.color = Theme.indicatorOnline
            text_indicator_listener.text = "MQS"
        } else if (tor) {
            indicator_listener.color = Theme.indicatorOnline
            text_indicator_listener.text = "TOR"
        } else {
            indicator_listener.color = "red"
            text_indicator_listener.text = "Offline"
        }

    }

    function updateChart() {
        chartPointPos = []
        spline.removePoints(0, spline.count)

        //chart.removeAllSeries();
        //var spline = chart.createSeries(ChartView.SeriesTypeSpline, "Line series", valueAxisX, valueAxisY);
        let max = 0
        let min = dataMarketChart[0][1]
        console.log(max, min)
        let txs = transactionsItem.txsModal

        let listTx = []
        let balance = 0
        for (let a=txs.count -1; a >= 0 ; a--) {
            if (txs.get(a).isConf) {
                const time = txs.get(a).timestamp
                balance += Number(txs.get(a).txCoinNano)
                let itemListTx = listTx.length -1
                if (itemListTx >= 0 && listTx[itemListTx][0] === time) {
                    listTx[itemListTx][1] = balance
                } else {
                    listTx.push([time, balance])
                }
                console.log("ListTx ", listTx)
            }

        }
        //spline.width = dp(2)chart
        let balanceMWC = 0
        for (let y=0; y<= dataMarketChart.length -1 ; y++) {
            for (let r = 0; r <= listTx.length -1 ; r++) {
                if (dataMarketChart[y][0] < listTx[0][0]) {
                    break;
                }
                if (dataMarketChart[y][0] >= listTx[r][0]) {
                    balanceMWC = listTx[r][1]
                    listTx.shift()
                    console.log("listTx ", listTx)
                }
            }
            dataMarketChart[y][1] = balanceMWC * dataMarketChart[y][1]
            if (dataMarketChart[y][1]>max) {
                max = dataMarketChart[y][1]
            }
            if (dataMarketChart[y][1]<min) {
                min = dataMarketChart[y][1]
            }
            if (y === (dataMarketChart.length -1)) {
                valueAxisY.max = max == 0? 1 : max*1.2
                valueAxisY.min = min == 0? (max != 0? -max/4 :-1 ): min/1.2
                valueAxisX.min = dataMarketChart[0][0]
                valueAxisX.max = dataMarketChart[y][0]
                console.log(valueAxisY.max,valueAxisY.min )
            }
            let time = dataMarketChart[y][0]
            spline.append(time,dataMarketChart[y][1])
        }

        for (let k=0; k<= spline.count-1; k++) {
            var chartPoint = spline.at(k)
            var screenPoint = chart.mapToPosition(chartPoint )
            chartPointPos.push([chartPoint, screenPoint])

        }
    }

    function pointerChart(mouseX, mouseY){
        var mousePoint = chart.mapToValue(Qt.point(mouseX,mouseY))
        for (let k=0; k<= chartPointPos.length -1 ; k++){
            //var chartPoint = chart.mapToPosition(chartPointPos[k])
            if(mousePoint.x <= chartPointPos[k][0].x) {
                pointer.x = chartPointPos[k][1].x - pointer.width/2
                pointer_bar.x = chartPointPos[k][1].x - pointer_bar.width/2
                pointer.y = chartPointPos[k][1].y  - pointer.height/2
                time_chart.text = timeChart(chartPointPos[k][0].x)
                price_chart.text = "%1 %2".arg(hiddenAmount? hidden : Number(chartPointPos[k][0].y).toFixed(currencyPriceRound)).arg(currencyTicker)
                return
            }
        }
    }


    function timeChart(timestamp){
        let date = new Date(timestamp)
        if (chartRange<3) {
            return date.toLocaleString(locale, "M/d/yy hh:mm ap")
        }
        else {
            return date.toLocaleString(locale, "M/d/yy")
        }
    }

    function numberWithSpaces(x) {
        return x.toString().replace(/\B(?=(\d{3})+(?!\d))/g, " ");
    }

    // --------------------------------------------------------
    // ----------------- MAIN BALANCE DASHBOARD ---------------
    // --------------------------------------------------------

    Rectangle {
        id: market
        anchors.top: parent.top
        height: parent.height*0.25
        width: parent.width
        anchors.topMargin: dp(10)
        color: "#00000000"
        clip: false
        anchors.horizontalCenter: parent.horizontalCenter
        SwipeView {
            id: view

            currentIndex: 0
            anchors.fill: parent

            Item {
                id: firstPa

                Rectangle {
                    id: rect_0
                    color: '#00000000'

                    height: market.height*0.92
                    width: market.width*0.8
                    anchors.horizontalCenter: parent.horizontalCenter

                    RectangleSideRound {
                        id: rect_0_0
                        rec_height: parent.height*0.7
                        rec_width: parent.width
                        rec_radius: dp(25)
                        rec_color: Theme.card
                        upRound: true
                        downRound: false
                        anchors.top: parent.top
                        LinearGradient {
                            start: Qt.point(0,0)
                            end: Qt.point(parent.width, parent.height)
                            gradient: Gradient {
                                orientation: Gradient.Vertical

                                GradientStop {
                                    position: 0
                                    color: "#3700c9"
                                }
                                GradientStop {
                                    position: 1
                                    color: "#c300ff"
                                }

                            }
                            anchors.fill:parent
                            source: parent

                        }
                        Text {
                            id: text_account
                            text: qsTr("current account: #%1").arg(selectedAccount)
                            color: Theme.textPrimary
                            font.capitalization: Font.AllUppercase
                            font.pixelSize: dp(13)
                            font.letterSpacing: dp(0.5)
                            horizontalAlignment: Text.AlignHCenter
                            font.family: barlow.medium
                            anchors.bottom: text_spend_balance.top
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            id: text_spend_balance
                            text: (hiddenAmount? hidden : spendableBalance)+ " MWC"
                            color: Theme.textPrimary
                            font.family: barlow.bold
                            font.bold : true
                            font.pixelSize: dp(25)
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            id: text_secondary_currency
                            text: (hiddenAmount? hidden : (currencyPrice * spendableBalance).toFixed(currencyPriceRound)) + " %1".arg(currencyTicker.toUpperCase())
                            color: Theme.textPrimary
                            font.pixelSize: dp(13)
                            font.letterSpacing: dp(1)
                            anchors.top: text_spend_balance.bottom
                            anchors.topMargin: dp(2)
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                    RectangleSideRound {
                        id: rect_0_1
                        rec_height: parent.height*0.3
                        rec_width: parent.width
                        rec_radius: dp(25)
                        rec_color: Theme.card
                        upRound: false
                        downRound: true
                        anchors.top: rect_0_0.bottom

                        Rectangle {
                            height: parent.height
                            width: parent.width/2
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            color: "#00000000"


                            ImageColor {
                                id: img_await
                                img_height: parent.height/3
                                img_source: "../../img/hourglass.svg"
                                img_color: Theme.textPrimary
                                anchors.right: text_await.left
                                anchors.rightMargin:dp(7)
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                id: text_await
                                text: qsTr(awaitBalance + " MWC")
                                color: Theme.textPrimary
                                font.pixelSize: dp(15)
                                font.italic: true
                                font.weight: Font.Light
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }

                        Rectangle {
                            height: parent.height
                            width: parent.width/2
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            color: "#00000000"


                            ImageColor {
                                id: img_locked
                                img_height: parent.height/3
                                img_source: "../../img/lock.svg"
                                img_color: Theme.textPrimary
                                anchors.right: text_locked.left
                                anchors.rightMargin:dp(7)
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                id: text_locked
                                text: qsTr(lockedBalance + " MWC")
                                color: Theme.textPrimary
                                font.pixelSize: dp(15)
                                font.italic: true
                                font.weight: Font.Light
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }


                    }
                }
            }

            Item {
                id: firstPage
                Rectangle {
                    id: rect_1
                    color: Theme.card

                    height: market.height*0.92
                    width: market.width*0.8
                    radius: dp(15)
                    anchors.horizontalCenter: parent.horizontalCenter

                    LinearGradient {
                        start:Qt.point(0,0)
                        end:Qt.point(rect_1.width, rect_1.height)
                        gradient: Gradient {
                            orientation: Gradient.Vertical

                            GradientStop {
                                position: 0
                                color: "#1e0253"
                            }
                            /*GradientStop {
                                position: 0.4
                                color: "#C850C0"
                            }*/
                            GradientStop {
                                position: 1
                                color: "#c637a0"
                            }


                        }
                        anchors.fill: rect_1
                        source: rect_1
                    }

                    Text {
                        id: text_node
                        text: "Node"
                        color: Theme.textPrimary
                        font.family: barlow.bold
                        font.bold: true
                        font.pixelSize: dp(22)
                        anchors.top: parent.top
                        anchors.topMargin: dp(10)
                        anchors.left: parent.left
                        anchors.leftMargin: dp(15)
                    }



                    Rectangle {
                        id: indicator_node
                        color: Theme.indicatorOnline
                        height: dp(10)

                        width: height
                        radius: width/2
                        anchors.left: text_node.left
                        anchors.verticalCenter: text_indicator.verticalCenter
                    }

                    Text {
                        id: text_indicator
                        text: "online"
                        color: Theme.textPrimary
                        font.capitalization: Font.AllUppercase
                        font.family: barlow.bold
                        font.pixelSize: dp(14)
                        anchors.top: text_node.bottom
                        anchors.topMargin: dp(0)
                        anchors.left: indicator_node.right
                        anchors.leftMargin: dp(5)
                    }


                    Text {
                        id: text_listener
                        text: "Listener"
                        color: Theme.textPrimary
                        font.family: barlow.bold
                        font.bold: true
                        font.pixelSize: dp(22)
                        anchors.top: parent.top
                        anchors.topMargin: dp(10)
                        anchors.right: parent.right
                        anchors.rightMargin: dp(15)
                    }

                    Rectangle {
                        id: indicator_listener
                        color: Theme.indicatorOnline
                        height: dp(10)
                        width: height
                        radius: width/2
                        anchors.left: text_listener.left
                        anchors.verticalCenter: text_indicator.verticalCenter
                    }




                    Text {
                        id: text_indicator_listener
                        text: "-"
                        color: Theme.textPrimary
                        font.capitalization: Font.AllUppercase
                        font.family: barlow.bold
                        font.pixelSize: dp(14)
                        anchors.top: text_node.bottom
                        anchors.topMargin: dp(0)
                        anchors.left: indicator_listener.right
                        anchors.leftMargin: dp(7)
                    }

                    Label {
                        id: text_label_height
                        text: "block height"
                        color: Theme.textPrimary
                        font.capitalization: Font.AllUppercase
                        font.pixelSize: dp(13)
                        anchors.bottom: text_height.top
                        anchors.left: text_height.left
                    }

                    Text {
                        id: text_height
                        text: "N/A"
                        color: Theme.textPrimary
                        font.family: barlow.bold
                        font.bold: true
                        font.pixelSize: dp(22)
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: dp(20)
                        anchors.left: parent.left
                        anchors.leftMargin: dp(10)
                    }

                    Rectangle {
                        anchors.top: text_label_height.top
                        anchors.bottom: text_height.bottom
                        anchors.left: text_label_height.right
                        anchors.leftMargin: dp(10)
                        anchors.right: rect_peers.left
                        anchors.rightMargin: dp(10)
                        color: "#00000000"

                        Label {
                            id: label_diff
                            text: "Difficulty"
                            color: Theme.textPrimary
                            font.capitalization: Font.AllUppercase
                            font.pixelSize: dp(13)
                            anchors.bottom: text_height.top
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            id: text_diff
                            text: "N/A"
                            color: Theme.textPrimary
                            font.family: barlow.bold
                            elide: Text.ElideMiddle
                            font.bold: true
                            font.pixelSize: dp(22)
                            anchors.bottom: parent.bottom
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }

                    Label {
                        id: label_peers
                        text: "peers"
                        color: Theme.textPrimary
                        font.pixelSize: dp(13)
                        font.capitalization: Font.AllUppercase
                        anchors.verticalCenter: text_label_height.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: dp(15)
                    }

                    Rectangle {
                        id: rect_peers
                        height: text_height.height
                        width: height
                        opacity: 0.5
                        radius: width/2
                        color: "black"
                        anchors.verticalCenter: text_height.verticalCenter
                        anchors.horizontalCenter: label_peers.horizontalCenter

                    }

                    Text {
                        id: text_peers
                        text: "N/A"
                        color: Theme.textPrimary
                        font.family: barlow.bold
                        font.bold: true
                        font.pixelSize: dp(13)
                        anchors.verticalCenter: rect_peers.verticalCenter
                        anchors.horizontalCenter: rect_peers.horizontalCenter
                    }


                    /*Text {
                        id: text_block
                        text: "00060558090bbc16e3aefc9f0cff0c7523db5fec881e635d669f562fca4a5682"
                        color: Theme.textPrimary
                        elide: Text.ElideMiddle
                        font.family: barlow.bold
                        font.pixelSize: dp(16)
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: dp(10)
                        anchors.left: parent.left
                        anchors.leftMargin: dp(10)
                        anchors.right: parent.right
                        anchors.rightMargin: dp(10)
                        anchors.horizontalCenter: parent.horizontalCenter
                    }*/
                }
            }
            Item {
                id: secondPage
                Rectangle {
                    id: rect_2
                    color: Theme.card

                    height: market.height*0.92
                    width: market.width*0.8
                    radius: dp(15)
                    anchors.horizontalCenter: parent.horizontalCenter

                    LinearGradient {
                        start:Qt.point(0,0)
                        end:Qt.point(rect_2.width, rect_2.height)
                        gradient: Gradient {
                            orientation: Gradient.Vertical

                            GradientStop {
                                position: 0.8

                                color: "#240b36"
                            }
                            GradientStop {
                                position: -0.3
                                color:  d7_change[currencyTicker] > 0? Theme.indicatorOnline : "#c31432" ////"#c31432"
                            }

                            /*GradientStop {
                                        position: 0.5
                                        color: "#203A43"
                                    }*/


                        }
                        anchors.fill: rect_2
                        source: rect_2
                    }

                    Text {
                        id: text_market
                        text: "Market"
                        color: Theme.textPrimary
                        font.family: barlow.bold
                        font.bold: true
                        font.pixelSize: dp(22)
                        anchors.top: parent.top
                        anchors.topMargin: dp(10)
                        anchors.left: parent.left
                        anchors.leftMargin: dp(15)
                    }
                    Label {
                        id: label_price
                        text: "Price"
                        color: Theme.textPrimary
                        font.capitalization: Font.AllUppercase
                        font.pixelSize: dp(13)
                        anchors.left: text_price.left
                        anchors.bottom: text_price.top
                    }

                    Text {
                        id: text_price
                        text: "%1 %2".arg(currencyPrice.toFixed(currencyPriceRound)).arg(currencyTicker)
                        color: Theme.textPrimary
                        font.family: barlow.bold
                        font.capitalization: Font.AllUppercase
                        font.bold: true
                        font.pixelSize: dp(22)
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: dp(15)
                    }

                    Label {
                        id: label_change_7d
                        text: "7D"
                        color: Theme.textPrimary
                        font.capitalization: Font.AllUppercase
                        font.pixelSize: dp(13)
                        anchors.verticalCenter: text_market.verticalCenter
                        anchors.left: text_change_7d.left
                    }

                    Text {
                        id: text_change_7d
                        text: "%1 %".arg(d7_change[currencyTicker].toFixed(2))
                        color: d7_change[currencyTicker]> 0? "green" : "red"
                        font.family: barlow.bold
                        font.capitalization: Font.AllUppercase
                        font.bold: true
                        font.pixelSize: dp(13)
                        anchors.top: label_change_7d.bottom
                        anchors.right: text_change_m.left
                        anchors.rightMargin: dp(15)



                    }

                    Label {
                        id: label_m
                        text: "M"
                        color: Theme.textPrimary
                        font.capitalization: Font.AllUppercase
                        font.pixelSize: dp(13)
                        anchors.verticalCenter: text_market.verticalCenter
                        anchors.left: text_change_m.left
                    }

                    Text {
                        id: text_change_m
                        text: "%1 %".arg(d30_change[currencyTicker].toFixed(2))
                        color: d30_change[currencyTicker]> 0? "green" : "red"
                        font.family: barlow.bold
                        font.capitalization: Font.AllUppercase
                        font.bold: true
                        font.pixelSize: dp(13)
                        anchors.top: label_m.bottom
                        anchors.right: text_change_y.left
                        anchors.rightMargin: dp(15)
                    }

                    Label {
                        id: label_y
                        text: "Y"
                        color: Theme.textPrimary
                        font.capitalization: Font.AllUppercase
                        font.pixelSize: dp(13)
                        anchors.verticalCenter: text_market.verticalCenter
                        anchors.left: text_change_y.left
                    }

                    Text {
                        id: text_change_y
                        text: "%1 %".arg(y_change[currencyTicker].toFixed(2))
                        color: y_change[currencyTicker] > 0? "green" : "red"
                        font.family: barlow.bold
                        font.capitalization: Font.AllUppercase
                        font.bold: true
                        font.pixelSize: dp(13)
                        anchors.top: label_y.bottom
                        anchors.right: parent.right
                        anchors.rightMargin: dp(15)
                    }

                    Label {
                        id: label_mc
                        text: "Market cap"
                        color: Theme.textPrimary
                        font.pixelSize: dp(13)
                        font.capitalization: Font.AllUppercase
                        anchors.bottom: text_circu.top
                        anchors.left: parent.left
                        anchors.leftMargin: dp(15)
                    }

                    Text {
                        id: text_mc
                        text: "%1 %2".arg(numberWithSpaces((circu_supply*currencyPrice).toFixed(0))).arg(currencyTicker)
                        color: Theme.textPrimary
                        font.family: barlow.bold
                        font.bold: true
                        font.capitalization: Font.AllUppercase
                        font.pixelSize: dp(18)
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: dp(20)
                        anchors.left: parent.left
                        anchors.leftMargin: dp(15)
                    }




                    Label {
                        id: label_circu
                        text: "Circulating Supply"
                        color: Theme.textPrimary
                        font.pixelSize: dp(13)
                        font.capitalization: Font.AllUppercase
                        anchors.bottom: text_circu.top
                        anchors.right: parent.right
                        anchors.rightMargin: dp(15)
                    }

                    Text {
                        id: text_circu
                        text: "%1 MWC".arg(numberWithSpaces(circu_supply.toFixed(0)))
                        color: Theme.textPrimary
                        font.family: barlow.bold
                        font.bold: true
                        font.capitalization: Font.AllUppercase
                        font.pixelSize: dp(18)
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: dp(20)
                        anchors.right: parent.right
                        anchors.rightMargin: dp(15)
                    }
                }
            }
            Item {
                id: thirdPage
                Rectangle {
                    id: rect_3
                    color: Theme.card
                    height: market.height*0.92
                    width: market.width*0.8
                    radius: dp(15)
                    anchors.horizontalCenter: parent.horizontalCenter

                    LinearGradient {
                        start:Qt.point(0,0)
                        end:Qt.point(rect_3.width, rect_3.height)
                        gradient: Gradient {
                            orientation: Gradient.Vertical

                            GradientStop {
                                position: 0
                                color: "#0f0c29"
                            }
                            GradientStop {
                                position: 1
                                color:  "#302b63"
                            }
                        }

                        anchors.fill: rect_3
                        source: rect_3
                    }


                    Text {
                        id: text_stats
                        text: "Network"
                        color: Theme.textPrimary
                        font.family: barlow.bold
                        font.bold: true
                        font.pixelSize: dp(22)
                        anchors.top: parent.top
                        anchors.topMargin: dp(10)
                        anchors.left: parent.left
                        anchors.leftMargin: dp(15)
                    }
                    Label {
                        id: label_hashrate
                        text: "hashrate"
                        color: Theme.textPrimary
                        font.capitalization: Font.AllUppercase
                        font.pixelSize: dp(13)
                        anchors.bottom: text_hashrate.top
                        anchors.left: text_hashrate.left
                    }

                    Text {
                        id: text_hashrate
                        text: "%1 GHS".arg(hashrate)
                        color: Theme.textPrimary
                        font.family: barlow.bold
                        font.capitalization: Font.AllUppercase
                        font.bold: true
                        font.pixelSize: dp(22)
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: dp(15)
                    }

                    Label {
                        id: label_reward
                        text: "Block Reward"
                        color: Theme.textPrimary
                        font.pixelSize: dp(13)
                        font.capitalization: Font.AllUppercase
                        anchors.bottom: text_reward.top
                        anchors.left: text_reward.left
                    }

                    Text {
                        id: text_reward
                        text: "%1 MWC".arg(block_reward)
                        color: Theme.textPrimary
                        font.family: barlow.bold
                        font.bold: true
                        font.capitalization: Font.AllUppercase
                        font.pixelSize: dp(18)
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: dp(20)
                        anchors.left: parent.left
                        anchors.leftMargin: dp(15)
                    }




                    Label {
                        id: label_hashra
                        text: "Hashrate"
                        color: Theme.textPrimary
                        font.pixelSize: dp(13)
                        font.capitalization: Font.AllUppercase
                        anchors.bottom: text_hashra.top
                        anchors.left: text_hashra.left
                    }

                    Text {
                        id: text_hashra
                        text: "%1 GHS".arg(numberWithSpaces(hashrate))
                        color: Theme.textPrimary
                        font.family: barlow.bold
                        font.bold: true
                        font.capitalization: Font.AllUppercase
                        font.pixelSize: dp(18)
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: dp(20)
                        anchors.right: parent.right
                        anchors.rightMargin: dp(15)
                    }
                }
             }
        }

        /*PageIndicator {
            id: indicator
            anchors.top: view.bottom
            anchors.topMargin: dp(0)
            spacing: dp(10)
            delegate: Rectangle {
                implicitWidth: dp(8)
                implicitHeight: dp(8)

                radius: width / 2
                //color: "#21be2b"

                opacity: index === view.currentIndex ? 0.95 : pressed ? 0.7 : 0.45

                Behavior on opacity {
                    OpacityAnimator {
                        duration: 100
                    }
                }
            }
            count: view.count
            currentIndex: view.currentIndex
            anchors.horizontalCenter: parent.horizontalCenter
        }*/
    }



    // --------------------------------------------------------
    // --------------- LOCKED/AWAITING BALANCE  ---------------
    // --------------------------------------------------------






    // --------------------------------------------------------
    // -------- MAIN NAV BUTTON (SEND-RECEIVE-FINALIZE) -------
    // --------------------------------------------------------

    Rectangle {
        id: navbarButton
        height: parent.width/7
        width: parent.width/1.25
        anchors.top: market.bottom
        anchors.topMargin: dp(20)
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#00000000"

        RowLayout {
            id: layoutNavBottom
            spacing: 0
            Rectangle {
                color: "#00000000"
                Layout.fillHeight: true
                Layout.minimumHeight: parent.parent.height
                Layout.minimumWidth: parent.parent.width/3
                Rectangle {
                    id: rec_send
                    height: parent.height
                    width: height
                    color: Theme.navButton
                    radius: dp(50)
                    anchors.horizontalCenter: parent.horizontalCenter
                    ImageColor {
                        id: img_send
                        img_height: rec_send.height/1.7
                        img_source:"../../img/export.svg"
                        img_rotation: 0
                        img_color: Theme.icon
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {

                            stateMachine.setActionWindow(8)
                        }
                    }
                }
                Text {
                    id: text_send
                    text: qsTr("Send")
                    color: Theme.textPrimary
                    font.pixelSize: dp(12)
                    font.letterSpacing: dp(0.5)
                    anchors.top: rec_send.bottom
                    anchors.topMargin: dp(5)
                    anchors.horizontalCenter: parent.horizontalCenter
                }


            }
            Rectangle {
                color: "#00000000"
                Layout.fillHeight: true
                Layout.minimumHeight: parent.parent.height
                Layout.minimumWidth: parent.parent.width/3
                Rectangle {
                    id: rec_receive
                    height: parent.height
                    width: height
                    color: Theme.navButton
                    radius: dp(50)
                    anchors.horizontalCenter: parent.horizontalCenter
                    ImageColor {
                        id: img_receive
                        img_height: rec_receive.height/1.7
                        img_source:"../../img/export.svg"
                        img_rotation: 180
                        img_color: Theme.icon
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            stateMachine.setActionWindow(9)
                        }
                    }
                }
                Text {
                    id: text_receive
                    text: qsTr("Receive")
                    color: Theme.textPrimary
                    font.pixelSize: dp(12)
                    font.letterSpacing: dp(0.5)
                    anchors.top: rec_receive.bottom
                    anchors.topMargin: dp(5)
                    anchors.horizontalCenter: parent.horizontalCenter
                }


            }
            Rectangle {
                color: "#00000000"
                Layout.fillHeight: true
                Layout.minimumHeight: parent.parent.height
                Layout.minimumWidth: parent.parent.width/3
                Rectangle {
                    id: rec_finalize
                    height: parent.height
                    width: height
                    color: Theme.navButton
                    radius: dp(50)
                    anchors.horizontalCenter: parent.horizontalCenter

                    ImageColor {
                        id: img_finalize
                        img_height: rec_finalize.height/1.7
                        img_source:"../../img/check.svg"
                        img_rotation: 0
                        img_color: Theme.icon
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            stateMachine.setActionWindow(19)
                        }
                    }
                }

                Text {
                    id: text_finalize
                    text: qsTr("Finalize")
                    color: Theme.textPrimary
                    font.pixelSize: dp(12)
                    font.letterSpacing: dp(0.5)
                    anchors.top: rec_finalize.bottom
                    anchors.topMargin: dp(5)
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }

    // --------------------------------------------------------
    // -------- SLIDE VIEW CARD (NODE/MARKET/NETWORK ----------
    // --------------------------------------------------------


    Rectangle {
        id: stats
        anchors.top: navbarButton.bottom
        anchors.bottom: parent.bottom
        anchors.topMargin: dp(40)
        color: "#00000000"
        width: parent.width
        anchors.horizontalCenter: parent.horizontalCenter


        // --------------------------------------------------------
        // ------------------- CHART BALANCE ----------------------
        // --------------------------------------------------------

        Rectangle {
            id: info_chart
            anchors.top: parent.top
            height: parent.height*0.2
            width: parent.width
            color: "#00000000"
            Text {
                id: price_chart
                text: ""
                color: Theme.textPrimary
                font.pixelSize: dp(22)
                font.bold: true
                font.capitalization: Font.AllUppercase
                visible: pointer.visible
                anchors.right: parent.right
                anchors.rightMargin: dp(15)
                anchors.left: parent.left
                anchors.leftMargin: dp(15)

            }
            Text {
                id: time_chart
                text: ""
                color: Theme.textSecondary
                font.pixelSize: dp(16)
                font.italic: true
                visible: pointer.visible
                anchors.left: price_chart.left
                anchors.top: price_chart.bottom
                //anchors.topMargin: dp(10)
            }



        }
        Rectangle {
            id: network
            anchors.top: info_chart.bottom
            height: parent.height*0.55
            width: parent.width*1.07
            color: "#00000000"
            radius: dp(25)
            anchors.horizontalCenter: parent.horizontalCenter
            ChartView {
                id: chart
                title: ""
                anchors.top: info_chart.bottom
                anchors.topMargin: dp(10)
                width: parent.width
                height: parent.height
                //anchors.left: parent.left
                //anchors.leftMargin:
                antialiasing: true
                backgroundColor: "#00000000"
                legend.visible: false
                anchors.verticalCenter: parent.verticalCenter

                ValueAxis {
                    id: valueAxisX
                    min: minimum
                    max: maximum
                    visible: false
                }
                ValueAxis {
                    id: valueAxisY
                    min: 0
                    max: 10
                    visible: false
                    

                }

                SplineSeries {
                        id: spline
                        axisX: valueAxisX
                        axisY: valueAxisY
                        width: dp(2)
                        pointsVisible: false
                    }

            }



            MouseArea {
                    id: chart_hover
                    anchors.fill: chart
                    onPressed: {
                        pointer.visible = true
                        pointer_bar.visible = true
                        pointerChart(mouseX, mouseY)
                    }
                    onPositionChanged: {
                        pointerChart(mouseX, mouseY)
                    }

                    onReleased: {
                        pointer.visible = false
                        pointer_bar.visible = false
                    }
                }

            Glow {
                id: glow
                anchors.fill: chart
                radius: 18
                samples: 168
                spread: 0.15
                color: "#312c36"
                visible: trues
                source: chart
            }
            LinearGradient {
                id: gradi
                // 200 is length of gamut
                start:Qt.point(0,0)
                end:Qt.point(0,chart.height)
                gradient: Gradient {
                    GradientStop {
                        position: 1;
                        color: "#03001e";
                    }
                    GradientStop {
                        position: 0.75;
                        color: "#7303c0";
                    }
                    GradientStop {
                        position: 0.5;
                        color: "#7303c0";
                    }
                    GradientStop {
                        position: 0.25;
                        color: "#ec38bc";
                    }
                    GradientStop {
                        position: 0;
                        color: "#fdeff9";
                    }
                }
                anchors.fill: glow
                source: glow
            }
            Rectangle {
                id: pointer_bar
                height: parent.height
                width: dp(1)
                radius: width/2

                color: Theme.chartPointer
                x: 0
                y: 0
                visible: false
            }
            Rectangle {
                id: pointer
                height:dp(9)
                radius: width/2
                width: height
                color: Theme.chartPointerBar
                border.color: Theme.textPrimary
                border.width: dp(2)
                x: 0
                y: 0
                visible: false
            }
        }

        Rectangle {
            anchors.top: network.bottom
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            color: "#00000000"
            Rectangle {
                id: button_time
                width: parent.width/1.3
                height: Math.min(dp(40), parent.height*0.7)
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                color: Theme.card
                radius: dp(25)
                RowLayout {
                    id: layout
                    spacing: 0
                    Rectangle {
                        id: rec_d1
                        Layout.fillHeight: true
                        Layout.minimumHeight: button_time.height
                        Layout.minimumWidth: button_time.width/5
                        color: "#00000000"
                        Text {
                            id: text_d1
                            text: "1D"
                            color: Theme.textPrimary
                            font.capitalization: Font.AllUppercase
                            font.pixelSize: dp(15)
                            font.bold: true
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                chartRange = 1
                                setChart()
                            }
                        }
                    }
                    Rectangle {
                        id: rec_d7
                        Layout.fillHeight: true
                        Layout.minimumHeight: button_time.height
                        Layout.minimumWidth: button_time.width/5
                        color: "#00000000"
                        Text {
                            id: text_d7
                            text: "7D"
                            color: Theme.textPrimary
                            font.capitalization: Font.AllUppercase
                            font.bold: true
                            font.pixelSize: dp(15)
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                chartRange = 2
                                setChart()
                            }
                        }
                    }
                    Rectangle {
                        id: rec_m
                        Layout.fillHeight: true
                        Layout.minimumHeight: button_time.height
                        Layout.minimumWidth: button_time.width/5
                        color: "#00000000"
                        Text {
                            id: text_m
                            text: "M"
                            color: "purple"
                            font.bold: true
                            font.capitalization: Font.AllUppercase
                            font.pixelSize: dp(15)
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                chartRange = 3
                                setChart()
                            }
                        }
                    }
                    Rectangle {
                        id: rec_m3
                        Layout.fillHeight: true
                        Layout.minimumHeight: button_time.height
                        Layout.minimumWidth: button_time.width/5
                        color: "#00000000"
                        Text {
                            id: text_m3
                            text: "3M"
                            color: Theme.textPrimary
                            font.capitalization: Font.AllUppercase
                            font.bold: true
                            font.pixelSize: dp(15)
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                chartRange = 4
                                setChart()

                            }
                        }
                    }
                    Rectangle {
                        id: rec_y
                        Layout.fillHeight: true
                        Layout.minimumHeight: button_time.height
                        Layout.minimumWidth: button_time.width/5
                        color: "#00000000"
                        Text {
                            id: text_y
                            text: "Y"
                            color: Theme.textPrimary
                            font.capitalization: Font.AllUppercase
                            font.pixelSize: dp(15)
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                chartRange = 5
                                setChart()
                            }
                        }
                    }



                }
            }
        }



        ListModel {
            id: list_info_io
        }

        Component {
            id: items_info_io
            Rectangle {
                height: label_txnum.height + text_txnum.height + dp(10)
                width: network.width/3 -dp(25)
                color: Theme.card
                radius: dp(15)
                Layout.alignment: Qt.AlignHCenter
                visible: true
                Layout.topMargin: dp(10)
                Layout.bottomMargin: dp(10)
                Text {
                    id: label_txnum
                    color: Theme.textPrimary
                    text: "label"
                    anchors.topMargin: dp(5)
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    font.pixelSize: dp(15)
                }

                Text {
                    id: text_txnum
                    color: Theme.textPrimary
                    text: "value"
                    anchors.topMargin: dp(3)
                    anchors.top: label_txnum.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: dp(15)
                }
            }

        }
    }
}


/*##^##
Designer {
    D{i:0;autoSize:true;height:640;width:0}
}
##^##*/
