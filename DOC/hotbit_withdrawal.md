This page will explain how to withdraw MWC from Hotbit into the mwc-qt-wallet. When using MWC you will want to become familiar with how *Listening* works.

# Before you start

This is a Legacy Document and may become out-dated as Exchanges evolve and Implement different Methods to Deposit and Withdraw.
As of writing this Document Hotbit allows withdrawals and deposits using the Online method and serves as an example for an "HTTP Exchange"
To Setup your Wallet for Listening please reffer to this [Guides](https://github.com/mwcproject/docs) depending on your Setup.
Futhermore it is recommended that you use Chrome on Hotbit. Other browsers may work but everything in this tutorial has been tested using Chrome.

You will want to make sure that your MWC wallet is:
* Online
* Connected to other peers in the MWC network
* Listening for an HTTP address

# Listening

MWC operates slightly differently than the other crypto-assets most people are familiar with. Transactions can be completed either with Files or Online. Hotbit uses the Online method.

When using the Online method the Receiver's wallet *must* be listening for the transaction. Currently, the Receiver's wallet will only listen for *ONE* address. Consequently, it is *extremely important* that the address you have Hotbit send the MWC to is *being listened* to by your wallet *when* Hotbit sends the transaction.

Hotbit usually processes the MWC withdrawals within a few minutes, usually less than 5-10, of the request being made. However, there have been some reports of it taking Hotbit over an hour to broadcast the transaction.

For Detailed Instructions on how to receive using different Methods to Setup everything please reffer to this [Guides](https://github.com/mwcproject/docs) depending on your wanted Setup)

# Go to Withdrawals page.

Click on the "Fund" tab then "Deposits & Withdrawals".

![Deposits and Withdrawals](exchange_images/Screen%20Shot%202019-12-04%20at%204.57.38%20PM.png)

# Select the Receive tab from your wallet.

![Receive Tab](exchange_images/Screen_Shot_2019-12-04_at_8.45.02_AM_50.png)

# Enter your http(s) receiving address into Hotbit's UI.

First click "Withdrawals" under MWC. You will see a section that looks like this:

![Withdrawals](exchange_images/Screen%20Shot%202019-12-04%20at%205.11.18%20PM.png)

Click the +Add button you will now see two text areas: "Label" and "Withdraw Address".
You may put anything you want in the "label" field.

For "Withdraw address" enter your HTTP address, depending on how you Setup your Wallet. 

This is *very important* as if you put ANYTHING other than the address your MWC wallet is listening for then the withdrawal process will fail and you will have to open a support issue with Hotbit to complete your withdrawal.

Note: you must include the "http(s)://" part of the address.

So for instance your address that you enter into Hotbit may look something like this:

"http://123.12.5.2:3415"

*Please Note* that NGrok addresses are only valid for 8 hours without a bought plan.
If you are a Ngrok User and an Exchange requires you to presave addresses keep this in Mind and consider forwardig a Port. 

To save the new address, you will need to have an amount set in the "MWC Withdrawal" field to proceed. Once you have added you may click "Submit".

You will see a dialog box that looks like this:

![Dialog](exchange_images/Screen%20Shot%202019-12-04%20at%205.18.29%20PM.png)

Follow the instructions in the dialog box by entering your email and submitting the code sent to you via email and enter your Google authenticator code.

Once complete you can click "Confirm".

Before clicking confirm make sure your MWC qt-wallet is open, connected to the network and listening.

Your wallet will still be listening even if you are locked out due to inactivity but do not close the wallet.

After your Withdrawal is processed by Hotbit you will see a dialog box like this in your mwc-qt-wallet:

![Confirm](exchange_images/Screen%20Shot%202019-12-04%20at%205.20.58%20PM.png)

You should now have the funds in your wallet.

IMPORTANT: After an hour or so, please make sure to resync with your full node. You can do this from the menu at the top right of the screen. This will ensure the funds are really there in case there was a reorg on the network.

# Cold Storage

At this point you may want to look into our cold storage project here: https://github.com/mwcproject/coldstorage

Above version uses command line. A GUI version for cold storage can be achieved using the QT Wallet, see: https://github.com/mwcproject/mwc-qt-wallet/blob/master/DOC/cold_wallet.md
