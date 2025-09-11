# Wallets setup for Swap testing

New Swap version include support for Dash, Doge, Ltc and ZCash. To test them we need to setup the wallets, get coins.

This document trying to make it easier.

The LTC and Zec has nice fauset but for other coins it was hard to get something. When your wallets will be ready, send me an address, I will send you some testnet coins.

# Dash

Install effort: low and fast.

For Dash the ElectrumX wallet is available. 

https://github.com/akhavr/electrum-dash/releases/download/4.0.9.1/Dash-Electrum-4.0.9.1-macosx.dmg

It support adresses:
- Standand
- Multisig

We are supporting all of them.

### Mainnet 
To run at mainnet, run it normally from Application.

### Testnet 
To run at testnet (MacOS) use the command line: 
```bash
/Applications/Dash\ Electrum.app/Contents/MacOS/electrum-dash-4.0.9.1.bin --testnet
```

# Doge

Install effort: high, requre a lot of time to sync.

Adresses:  
- standard

If you found wallet that support multisig for Doge, please let me know. We should support such adreeses, 
but it was never tested.  

### Testnet

Doge wallet runs it's own node and it is very slow. It will require fee days to sync.

Get a version at   https://github.com/dogecoin/dogecoin/releases  
Install it.

For MacOS to run as testnet, use the command line:
```bash
/Applications/Dogecoin-Qt.app/Contents/MacOS/Dogecoin-Qt --testnet
```

To view the logs, go into the system menu: 

"Help" -> "Debug Window" -> "Information" (selected by default) => "Debug log file", press "Open"

### Mainnet

Get Exadus multicoin wallet. It support Doge.

https://www.exodus.io/download/

Exadus doesn't require the local node, so there is no sync. 


# Litecoin (LTC)

Install effort: Low, fast

Adresses the same as BTC has:
- Legacy
- miltisig Legacy
- Segwit
- miltisig Segwit

We are supporting all type of addresses form LTC.

For Litecoin there is Electrum Wallet available 

https://electrum-ltc.org/

### Testnet

Run from the command line:
```bash
/Applications/Electrum-LTC.app/Contents/MacOS/Electrum-LTC --testnet
```

### Mainnet

Run it normally from Application.

# ZCash

Install effort: High, Slow

ZCash has Addresses
- Shielded
- **Transparent**.

We are supported only **Transparent** addresses. If there are wallet with Transparent multisig address, please let me know.
It should be supported but was never tested.

### Testnet

For the testnet we are going to install the Zecwallet Fullnode.

https://www.zecwallet.co/fullnode.html

- Install and start the wallet. The wallet will start at mainnet.
- Close the wallet.
- Switch to the test net
```
vi ~/Library/Application\ Support/Zcash/zcash.conf
```
add those two line into the config.
```
addnode=testnet.z.cash
testnet=1
```
- Start the wallet. Now wallet will start at the test network. The transparent address must start with 'tm'

The node logs location:  
```bash
~/Library/Application\ Support/Zcash/debug.log
```
### Mainnet

Get Exadus multicoin wallet. It support ZCash. https://www.exodus.io/download/

Exadus doesn't require the local node, so there is no sync. 

Or use Zecwallet Lite:
https://www.zecwallet.co/index.html

Both wallets can be run normally form Applications.