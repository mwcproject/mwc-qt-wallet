**MWC Cold Wallet Usage**

**Contents:**
- [Overview](#overview)
- [Prerequisites](#prerequisites)
  * [Cold Wallet Host](#cold-wallet-host)
  * [Online Node Host](#online-node-host)
- [Setup](#setup)
  * [Online Node Setup](#online-node-setup)
  * [Cold Wallet Setup](#cold-wallet-setup)
  * [First Time Wallet Login](#first-time-wallet-login)
- [Transfer Online Node Blockchain Data](#transfer-online-node-blockchain-data)
  * [Export Blockchain Data From Online Node](#export-blockchain-data-from-online-node)
  * [Import Blockchain Data Into MWC Cold Wallet](#import-blockchain-data-into-mwc-cold-wallet)
- [Trade Using MWC Cold Wallet](#trade-using-mwc-cold-wallet)


# Overview
A cold wallet is an offline wallet used to safely store your MWC funds. An offline wallet is a wallet that runs on hardware which is never connected to a network thru preventing attacking from gaining access to your MWC funds.

You should have already created a bootable USB device and installed MWC Qt Wallet on the USB device. If not, please refer to the following documents:
*    [Creating a USB Device With Persistent Storage For MWC Cold Wallet](https://github.com/mwcproject/mwc-qt-wallet/blob/master/DOC/cold_wallet_persistent_usb_drive_creation.md)

*    [MWC Cold Wallet Software Installation Guide](https://github.com/mwcproject/mwc-qt-wallet/blob/master/DOC/cold_wallet_software_installation_guide.md)

This document will guide you through the steps necessary to:
*    Run and configure your MWC cold wallet on a bootable USB device with MWC Qt Wallet already installed.
*    Run and configure an MWC Online Node on a second computer so that current blockchain data can be exported from the node and then imported into your cold wallet.
*    Perform trades using your MWC cold wallet

# Prerequisites

Two computers are needed to bring up and sync your _MWC cold wallet_. One computer, your _cold wallet host_, is offline and is used to run your _MWC cold wallet_. The second computer, your _online node host_, is online and runs an _MWC online node_. 

The _MWC cold wallet_ and the _online node host_ can run on different OSes. The data to be transferred between the node and the cold wallet is compatible at the OS level.

### Cold Wallet Host

To follow the procedures in this document you will need the following:
*    A bootable Ubuntu USB device with MWC Qt Wallet installed. This is your MWC Cold Wallet USB device.
*    A second USB device with 128GB to use for transferring files. This is your file transfer USB device.
*    An x86 laptop or desktop that has at least 2 USB drives and which can be used to boot your MWC Cold Wallet USB device.


### Online Node Host

A second pc/laptop with internet access will be needed to run an _online MWC node_. Data from the _online MWC node_ will be exported and transferred to your _cold wallet host_ using your file transfer USB device.

Please note, this online host will never have access to your wallet passphrase or other sensitive data.


# Setup

### Cold Wallet Setup

1) If you haven't already, create a bootable Ubuntu USB device with persistent storage following the directions at:<br/>
       
    [Creating a USB Device With Persistent Storage For MWC Cold Wallet](https://github.com/mwcproject/mwc-qt-wallet/blob/master/DOC/cold_wallet_persistent_usb_drive_creation.md)

2) If you haven't already, install MWC Qt Wallet onto your bootable Ubuntu USB device with persistent storage following the directions at:<br/>

    [MWC Cold Wallet Software Installation Guide](https://github.com/mwcproject/mwc-qt-wallet/blob/master/DOC/cold_wallet_software_installation_guide.md)

      If you have followed the directions correctly, you should be at the point where you have booted up your Ubuntu USB device with persistent storage and installed MWC Qt Wallet and its software dependencies. Your USB device should not have any connections to the network. Both wired and wireless connections should have been disabled.

3) Run the mwc-qt-wallet executable you have just installed. You will be presented with a login screen.

   <img src="cold_wallet_images/start.png" max-width="300" height="300"/>

   If this is your first time running your cold wallet, follow the directions for [First Time Wallet Login](#first-time-wallet-login) at the end of this section. Because your host is offline, you will see several failure notifications related to not being online. You can ignore them as they are expected. Then continue with _Step 4_.

   Otherwise, login to your cold wallet instance.

4) Click on the _Wallet Settings_ gear icon at the top right of the page. Scroll down the menu that appears and click _Select Running Mode_.

   <img src="cold_wallet_images/select_running_mode.png" max-width="300" height="300"/>

5) In the _Select Running Mode_ screen that appears, select _Cold Wallet_ and then click _Apply_.

   <img src="cold_wallet_images/cold_wallet_config.png" max-width="300" height="300"/>

6) In the _Wallet Run Mode_ pop-up that appears, click _Continue_ to cause your wallet to restart.

   Your wallet will restart and you will see some failure notifications because your wallet is offline.

7) Switch to the _Node Overview_ screen. 
   * Click the _Wallet Settings_ gear icon at the top right of the page.
   * Scroll down the menu that appears and click _Node Overview_.

   <img src="cold_wallet_images/cold_wallet_node_overview.png" max-width="300" height="300"/>

   You should see that the node is not able to sync.
   The first time you run your cold wallet, it will have 0 blocks.

   Important! If your node is syncing, you are connected to the network. Shutdown MWC Qt Wallet immediately and review the steps for ensuring you are disconnected from the network in the section _Disable All Network Connections_ in:<br/>

   [MWC Cold Wallet Software Installation Guide](https://github.com/mwcproject/mwc-qt-wallet/blob/master/DOC/cold_wallet_software_installation_guide.md)


### Online Node Setup

1) Install mwc-qt-wallet from the [Wallet Downlod](https://www.mwc.mw/downloads) page onto your _online node host_ computer.

2) Run the mwc-qt-wallet executable you have just installed.

   * If this is your first time running an online node, you will be presented with a login screen.

      <img src="cold_wallet_images/start.png" max-width="300" height="300"/>

      Follow the directions for [First Time Wallet Login](#first-time-wallet-login) at the end of this section. Then continue with _Step 3_.

   * If you last used MWC Qt Wallet on this host to run an online node, the online node should have come up and not required a login. Skip to _Step 6_.

   * If you last used MWC Qt Wallet on this host to run MWC Qt Wallet as an online wallet, you will be presented with a login screen. Login to your wallet instance and then continue with _Step 3_.

3) Click on the _Wallet Settings_ gear icon at the top right of the page. Scroll down the menu that appears and click _Select Running Mode_.

   <img src="cold_wallet_images/select_running_mode.png" max-width="300" height="300"/>

4) In the _Select Running Mode_ screen that appears, select _Online Node_ and then click _Apply_.

   <img src="cold_wallet_images/online_node_config.png" max-width="300" height="300"/>

5) In the _Wallet Run Mode_ pop-up that appears, click _Continue_ to cause your wallet to restart.

   Your wallet will restart and you will see some messages about your wallet being out of sync. It will now take a while for your wallet to sync with the network.

6) Switch to the _Node Overview_ screen to watch the progress of your wallet as it is syncing.
   * Click the _Wallet Settings_ gear icon at the top right of the page.
   * Scroll down the menu that appears and click _Node Overview_.

   <img src="cold_wallet_images/node_overview.png" max-width="300" height="300"/>

7) Wait until your node finishes syncing. Once your node has finished syncing you should see the following where your node's status is _Ready_ and your connection status to Mainnet at the bottom of the screen has switched from red to green.

    <img src="cold_wallet_images/node_in_sync.png" max-width="300" height="300"/>

   Until your node finishes syncing, 'Export Blockchain' and 'Publish Transaction' will be disabled.


### First Time Wallet Login
Whether you are running an online node for the first time or a cold wallet for the first time, you will need to create a wallet instace and configure it appropriately.

1) Run the mwc-qt-wallet executable.

2) Create a new wallet instance to dedicate to running the online node or cold wallet by selecting _New Instance_ on the wallet login page.

3) Finish creating your new wallet instance by:<br/>
   * If you do not like the data directory name generated by MWC Qt Wallet at the top of the screen, click _change data directory_ to specify a different data directory for your node. (optional)<br/>
   * Enter a name<br/>
   * Enter a password and confirm the password<br/>
   * Select _MainNet_<br/>
   * Click _Next_<br/>

    <img src="cold_wallet_images/init_wallet.png" max-width="300" height="300"/>

4) In the next screen that appears, copy the mneumonic passphrase onto a sheet of paper. Once we have finished setting up the wallet instance, you will need to keep this passphrase in a safe place where you will remember to find it, if you ever need to restore your wallet.

5) Enter each of the words in your mneumonic passphrase and continue to click _Submit_ until all words of your passphrase have been entered. Once you have entered all of the words in your passphrase successfully, you will see the following:

    <img src="cold_wallet_images/passphrase.png" max-width="300" height="300"/>

   When you see this pop-up, your wallet will be logged in and start running.

6) Click _OK_


# Transfer Online Node Blockchain Data

Once your _cold wallet_ and _online node_ have been set up and they are both running, you are now ready to move blockchain data from the _online node_ to your _offline cold wallet_.

The steps here should be performed any time you want to see the latest status of your transactions from within your _cold wallet_. So you would typically repeat these steps after you send or receive MWC funds using your cold wallet.

### Export Blockchain Data From Online Node

On the machine running your _online node_,  perform the following steps to export the latest blockchain data:

1) In your _online node_, wait until synchronization is finished and the button _Export Blockchain_ is enabled. 

2) Click _Export Blockchain_

3) Save the blockchain data to a file on your file transfer USB drive. These files will by default have an extension of _.mwc-blc_

4) Eject the file transfer USB drive from the machine running your _online node_.

### Import Blockchain Data Into MWC Cold Wallet

Perform the following steps on the machine running your _cold wallet_ to import blockchain data from an online node:

1) Plug your file transfer USB device, with the file containing the exported blockchain data, into the machine running your _cold wallet_.

2) In the MWC Node Status screen:

    <img src="cold_wallet_images/load_chain_data.png" max-width="300" height="300"/>

*  Click _Load Blockchain Data_
*  Select the file, on your file transfer USB device, with the exported blockchain data from your _online node_.
*  Click _Open_ at the top of the file selection screen.

    You should see that _Block Numbers_ is updated.

4) Now, you can view your transaction's, output. The data will be updated with the new blockchain data.

# Trade Using MWC Cold Wallet

Since the Cold Wallet is offline, you will only be able to use file based transactions.

### Receive MWC

1) Copy the MWC transaction file that is sent from another wallet to your file transfer USB device.
The initial transaction file should have an extention '.tx'.

2) Plug the file transfer USB stick with the file transaction into your 'Cold Wallet'.

3) In the 'Receive' tab sign the transaction.

    ![](cold_wallet_images/receive_mwc.png)

    The signed transaction file will be created in the same path as your transaction file. New file extention will be '.response'.

4) Move the signed transaction file to the online host and send the file back for finalization.

5) Check the block explorer to see that the transaction has been finalized. 

6) To verify the fact that your transaction was finalized, use the 'Transfer Online Node Blockchain Data' as explained in the previous steps.

7) Now your "Cold Wallet" has updated blockchain data that should contain the output of this transaction.

8) Go to "Transaction" page at "Cold Wallet" and check that "Confirm" for your transaction is "Yes"

9) Congratulations, you received the MWC in your offline wallet.

### Send MWC

1) In the "Cold Wallet", initiate a file based send transaction normally.

    ![](cold_wallet_images/send_cold_wallet.png)

    Save transaction file to the USB stick.

2) From the online host send Transaction file to the person whom you want to send MWC.

3) Wait for signed transaction file to be returned and save to USB stick.

4) Move the USB stick to the "Cold Wallet".

5) Finalize transaction from the "Finalize" tab.

    ![](cold_wallet_images/finalize1.png)

    ![](cold_wallet_images/finalize2.png)
    
    Please note that **"Result location"** points to the location of the slate that needs to be broadcast to the network.
    
6) Save resulting 'mwctx' file to the USB drive and move it to the "Online Node" host.

7) In the "Online Node", "node status", page press the "Publish Transaction" button and select the .mwctx file.

    ![](cold_wallet_images/node_publish_trans.png) 
  
    You should see a success message about that. If you see an error message, it is likely that this transaction is already published to the network or it has an invalid output.
      
8) Wait for some time until the transaction is accepted by the network and a few blocks have been mined.

9) To verify the fact that your transaction was finalized sucessfully, transfer the blockchain data as explained above.

10) Now, your 'Cold Wallet' has updated blockchain data that should contain the output from your transaction.

11) Go to 'Transaction' page in the 'Cold Wallet' and check that the "Confirm" field for your transaction is 'Yes'

12) Congratulations, you sent MWC.
