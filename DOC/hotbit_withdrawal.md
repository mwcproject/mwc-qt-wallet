This page will explain how to withdraw MWC from Hotbit into the mwc-qt-wallet. When using MWC you will want to become familiar with how *Listening* works.

# Before you start

It is recommended that you use Chrome on Hotbit. Other browsers may work but everything in this tutorial has been tested using Chrome.

You will want to make sure that your MWC wallet is:
* Online
* Connected to other peers in the MWC network
* Listening for a MWC address

# Listening

MWC operates slightly differently than the other crypto-assets most people are familiar with. Transactions can be completed either with File or Online. Hotbit uses the Online method.

When using the Online method the Receiver's wallet *must* be listening for the transaction. Currently, the Receiver's wallet will only listen for *ONE* address. Consequently, it is *extremely important* that the address you have Hotbit send the MWC to is *being listened* to by your wallet *when* Hotbit sends the transaction.

In your MWC wallet you can verify that your node is Online and connected to the network with green bubbles on the Mainnet button in the lower right and that the address you are listening for on the Receive tab or the MWC MQS tab (click on the button).

Hotbit usually processes the MWC withdrawals within a few minutes, usually less than 5-10, of the request being made. However, there have been some reports of it taking Hotbit over an hour to broadcast the transaction.

# Go to Withdrawals page.

Click on the "Fund" tab then "Deposits & Withdrawals".

![Deposits and Withdrawals](https://raw.githubusercontent.com/mwcproject/mwc-qt-wallet/master/DOC/Screen%20Shot%202019-12-04%20at%204.57.38%20PM.png)

# Select the Receive tab from your wallet.

![Receive Tab](https://raw.githubusercontent.com/mwcproject/mwc-qt-wallet/master/DOC/Screen_Shot_2019-12-04_at_8.45.02_AM_50.png)

# Enter the mwcmqs address into Hotbit's UI.

First click "Withdrawals" under MWC. You will see a section that looks like this:

![Withdrawals](https://raw.githubusercontent.com/mwcproject/mwc-qt-wallet/master/DOC/Screen%20Shot%202019-12-04%20at%205.11.18%20PM.png)

Click the +Add button you will now see two text areas: "Label" and "Withdraw Address".
You may put anything you want in the "label" field.

For "Withdraw address" enter the exact address you saw in the "Select the Receive tab from your wallet section" of this tutorial.

This is *very important* as if you put ANYTHING other than the address your MWC wallet is listening for then the withdrawal process will fail and you will have to open a support issue with Hotbit to complete your withdrawal.

Note: you must include the "mwcmqs://" part of the address.

So for instance your address that you enter into Hotbit may look something like this:

"mwcmqs://q5XLmBpaqstWg5S4S3m4umwgiYxF49tC4GkJ26QvvmkRA7HU7wXM"

To save the address, you will need to have an amount set in the "MWC Withdrawal" field to proceed. Once you have added you may click "Submit".

You will see a dialog box that looks like this:

![Dialog](https://raw.githubusercontent.com/mwcproject/mwc-qt-wallet/master/DOC/Screen%20Shot%202019-12-04%20at%205.18.29%20PM.png)

Follow the instructions in the dialog box by entering your email and submitting the code sent to you via email and enter your Google authenticator code.

Once complete you can click "Confirm".

Before clicking confirm make sure your MWC qt-wallet is open, connected to the network and listening.

Your wallet will still be listening even if you are locked out due to inactivity but do not close the wallet.

After your Withdrawal is processed by Hotbit you will see a dialog box like this in your mwc-qt-wallet:

![Confirm](https://raw.githubusercontent.com/mwcproject/mwc-qt-wallet/master/DOC/Screen%20Shot%202019-12-04%20at%205.20.58%20PM.png)

You should now have the funds in your wallet.

IMPORTANT: After an hour or so, please make sure to resync with your full node. You can do this from the menu at the top right of the screen. This will ensure the funds are really there in case there was a reorg on the network.

# Cold Storage

At this point you may want to look into our cold storage project here: https://github.com/mwcproject/coldstorage

This version uses command line. A GUI version for cold storage is being worked on.
