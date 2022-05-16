

const api_price = "https://api.coingecko.com/api/v3/coins/mimblewimblecoin?localization=false&tickers=false&market_data=true&community_data=false&developer_data=false&sparkline=false"
const api_market = "https://api.coingecko.com/api/v3/coins/mimblewimblecoin/market_chart?"
const api_network = "https://explorer.mwc.mw/GHs"


function setPrice(res){
    currencyPrice = res.market_data.current_price[currencyTicker]
    circu_supply = res.market_data.total_supply.toFixed(2)
    h_change = res.market_data.price_change_percentage_1h_in_currency
    d_change = res.market_data.price_change_24h_in_currency
    d7_change = res.market_data.price_change_percentage_7d_in_currency
    d14_change = res.market_data.price_change_percentage_14d_in_currency
    d30_change = res.market_data.price_change_percentage_30d_in_currency
    d60_change = res.market_data.price_change_percentage_60d_in_currency
    d200_change = res.market_data.price_change_percentage_200d_in_currency
    y_change = res.market_data.price_change_percentage_1y_in_currency
}

function setMarketChart(res){
    dataMarketChart= []
    dataMarketChart = res.prices
    walletItem.updateChart()


}

function setNetworkStats(res){
    console.log()
    hashrate = Number(res.C31_ghs).toFixed(0)
    block_reward = Number(res.reward)
}


function getPrice() {
    var request = new XMLHttpRequest()
    request.open('GET', api_price, true);
    request.onreadystatechange = function() {
        if (request.readyState === XMLHttpRequest.DONE) {
            if (request.status && request.status === 200) {
                var result = JSON.parse(request.responseText)
                setPrice(result)
            } else {
                console.log("Request getPrice: ", request.status, request.statusText)
            }
        }
    }
    request.send()
}


function getMarketChart(currency, days, interval) {
    const endpoint = api_market + "vs_currency=%1&days=%2&interval=%3".arg(currency).arg(days).arg(interval)
    console.log("endpoint",  endpoint )
    var request = new XMLHttpRequest()
    request.open('GET', endpoint, true);
    request.onreadystatechange = function() {
        if (request.readyState === XMLHttpRequest.DONE) {
            if (request.status && request.status === 200) {
                var result = JSON.parse(request.responseText)
                setMarketChart(result)

            } else {
                console.log("Request getMarketChart: ", request.status, request.statusText)
            }
        }
    }
    request.send()
}

function getNetworkStats(currency, days, interval) {
    var request = new XMLHttpRequest()
    request.open('GET', api_network, true);
    request.onreadystatechange = function() {
        if (request.readyState === XMLHttpRequest.DONE) {
            if (request.status && request.status === 200) {
                console.log("coingecko response", request.responseText)
                var result = JSON.parse(request.responseText)
                setNetworkStats(result)

            } else {
                console.log("Request getNetworkStats: ", request.status, request.statusText)
            }
        }
    }
    request.send()
}

const json_currency = [
  {
    "ticker": "aed",
    "name": "United Arab Emirates Dirham",
    "round": 2
  },
  {
    "ticker": "ars",
    "name": "Argentine Peso",
    "round": 2
  },
  {
    "ticker": "aud",
    "name": "Australian dollar",
    "round": 2
  },
  {
    "ticker": "bch",
    "name": "Bitcoin Cash",
    "round": 5
  },
  {
    "ticker": "bdt",
    "name": "Bangladeshi Taka",
    "round": 2
  },
  {
    "ticker": "bhd",
    "name": "Bahraini Dinar",
    "round": 2
  },
  {
    "ticker": "bmd",
    "name": "Bermudan Dollar",
    "round": 2
  },
  {
    "ticker": "bnb",
    "name": "Binance Coin",
    "round": 5
  },
  {
    "ticker": "brl",
    "name": "Brazilian Real",
    "round": 2
  },
  {
    "ticker": "btc",
    "name": "Bitcoin",
    "round": 8
  },
  {
    "ticker": "cad",
    "name": "Canadian Dollar",
    "round": 2
  },
  {
    "ticker": "chf",
    "name": "Swiss Franc",
    "round": 2
  },
  {
    "ticker": "clp",
    "name": "Chilean Peso",
    "round": 2
  },
  {
    "ticker": "cny",
    "name": "Chinese Yuan",
    "round": 2
  },
  {
    "ticker": "czk",
    "name": "Czech Koruna",
    "round": 2
  },
  {
    "ticker": "dkk",
    "name": "Danish Krone",
    "round": 2
  },
  {
    "ticker": "dot",
    "name": "Polkadot",
    "round": 2
  },
  {
    "ticker": "eos",
    "name": "Eosio",
    "round": 2
  },
  {
    "ticker": "eth",
    "name": "Ethereum",
    "round": 6
  },
  {
    "ticker": "eur",
    "name": "Euro",
    "round": 2

  },
  {
    "ticker": "gbp",
    "name": "Pound sterling",
    "round": 2
  },
  {
    "ticker": "hkd",
    "name": "Hong Kong Dollar",
    "round": 2
  },
  {
    "ticker": "huf",
    "name": "Hungarian Forint",
    "round": 2
  },
  {
    "ticker": "idr",
    "name": "Indonesian Rupiah",
    "round": 2
  },
  {
    "ticker": "ils",
    "name": "Israeli New Shekel",
    "round": 2
  },
  {
    "ticker": "inr",
    "name": "Indian Rupee",
    "round": 2
  },
  {
    "ticker": "jpy",
    "name": "Japanese Yen",
    "round": 2
  },
  {
    "ticker": "krw",
    "name": "South Korean won",
    "round": 2
  },
  {
    "ticker": "kwd",
    "name": "Kuwaiti Dinar",
    "round": 2
  },
  {
    "ticker": "lkr",
    "name": "Sri Lankan Rupee",
    "round": 2
  },
  {
    "ticker": "ltc",
    "name": "Litecoin",
    "round": 5
  },
  {
    "ticker": "mmk",
    "name": "Myanmar Kyat",
    "round": 2
  },
  {
    "ticker": "mxn",
    "name": "Mexican Peso",
    "round": 2
  },
  {
    "ticker": "myr",
    "name": "Malaysian Ringgit",
    "round": 2
  },
  {
    "ticker": "ngn",
    "name": "Nigerian Naira",
    "round": 2
  },
  {
    "ticker": "nok",
    "name": "Norwegian Krone",
    "round": 2
  },
  {
    "ticker": "nzd",
    "name": "New Zealand Dollar",
    "round": 2
  },
  {
    "ticker": "php",
    "name": "Philippine peso",
    "round": 2
  },
  {
    "ticker": "pkr",
    "name": "Pakistani Rupee",
    "round": 2
  },
  {
    "ticker": "pln",
    "name": "Poland złoty",
    "round": 2
  },
  {
    "ticker": "rub",
    "name": "Russian Ruble",
    "round": 2
  },
  {
    "ticker": "sar",
    "name": "Saudi Riyal",
    "round": 2
  },
  {
    "ticker": "sek",
    "name": "Swedish Krona",
    "round": 2
  },
  {
    "ticker": "sgd",
    "name": "Singapore Dollar",
    "round": 2
  },
  {
    "ticker": "thb",
    "name": "Thai Baht",
    "round": 2
  },
  {
    "ticker": "try",
    "name": "Turkish lira",
    "round": 2
  },
  {
    "ticker": "twd",
    "name": "New Taiwan dollar",
    "round": 2
  },
  {
    "ticker": "uah",
    "name": "Ukrainian hryvnia",
    "round": 2
  },
  {
    "ticker": "usd",
    "name": "United States Dollar",
    "round": 2
  },
  {
    "ticker": "vef",
    "name": "Venezuelan bolívar",
    "round": 2
  },
  {
    "ticker": "vnd",
    "name": "Vietnamese dong",
    "round": 2
  },
  {
    "ticker": "xag",
    "name": "Silver",
    "round": 5
  },
  {
    "ticker": "xau",
    "name": "Gold",
    "round": 5
  },
  {
    "ticker": "xdr",
    "name": "Special Drawing Rights",
    "round": 2
  },
  {
    "ticker": "xlm",
    "name": "Stellar",
    "round": 2
  },
  {
    "ticker": "xrp",
    "name": "Ripple",
    "round": 2
  },
  {
    "ticker": "yfi",
    "name": "Yearn",
    "round": 8
  },
  {
    "ticker": "zar",
    "name": "South African Rand",
    "round": 2
  },
  {
    "ticker": "bits",
    "name": "1 millionth of a BTC",
    "round": 2
  },
  {
    "ticker": "link",
    "name": "Chainlink",
    "round": 4
  },
  {
    "ticker": "sats",
    "name": "Satoshi",
    "round": 0
  }
]



const json_language = [{
                           "lang": qsTr("Arabic"),
                           "lang_tr": "عربي"
                         },
                         {
                           "ticker": qsTr("Chinese"),
                           "name": "中国人",
                         },
                         {
                           "ticker": qsTr("Czech"),
                           "name": "čeština",
                         },
                         {
                           "lang": qsTr("English"),
                           "lang_tr": "English"
                         },
                         {
                           "ticker": qsTr("French"),
                           "name": "Français",
                         },
                         {
                           "ticker": qsTr("German"),
                           "name": "Deutsch",
                         },
                         {
                           "ticker": qsTr("Hebrew"),
                           "name": "עִברִית",
                         },
                         {
                           "ticker": qsTr("Italian"),
                           "name": "Italiano",
                         },
                         {
                           "ticker": qsTr("Korean"),
                           "name": "한국어",
                         },
                         {
                           "ticker": qsTr("Persian"),
                           "name": "فارسی",
                         },
                         {
                           "ticker": qsTr("Portuguese"),
                           "name": "Português",
                         },
                         {
                           "ticker": qsTr("Spanish"),
                           "name": "Español",
                         },

                         {
                           "ticker": qsTr("Russian"),
                           "name": "русский",
                         },

                         {
                           "ticker": qsTr("Vietnamese"),
                           "name": "Tiếng Việt",
                         }]



