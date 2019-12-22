# Overview

This project allows the user to easily configure a cold storage USB stick for Ubuntu 18.04.3.

# Prerequisites

### Cold Wallet host

You will need an x64 laptop or desktop that has two USB drives or more. Generally speaking most 
pc/mac/linux hardware will work. You also need two USB sticks. The first USB stick will contain 
Ubuntu 18.04.03 or later (requires 2 GB+ of storage space). You can find instructions for installing 
this here: https://tutorials.ubuntu.com/tutorial/tutorial-create-a-usb-stick-on-ubuntu#0. The second USB 
stick will contain the binary to mwc-qt-wallet. 

This USB stick also will be used to transfer the data between this Cold Wallet Host and Online Node Host.
Currently for mainnet almost any size will be fine since the chain state is very small, but at some 
point a larger size might be required. We suggest getting a 128 GB stick for now which will be 
sufficient for the foreseable future and since they cost about $25 or less it is fairly reasonable. 

When installing Ubuntu, qt libraries are required. If you boot from USB, you need to use try-ubuntu 
which will have the qt libraries installed. **Please note, try-ubuntu doesn't have any permanent storage.
As a result you will not be able to keep your transaction history or any other historical data.
You will need to recover from the recovery seed each time to get access to your coins.** 

The better option is probably to dedicate a laptop or other hardware device that you will 
never connect to the internet. In this case, install the OS and ensure the Qt libraries are present. 
There is some information on how to do this on this page: 
[https://askubuntu.com/questions/974/how-can-i-install-software-or-packages-without-internet-offline].

Full explanation is out of scope for this document.

### Online Node Host

Second pc/laptop with internet access will be needed to run online MWC Node. Internet access is required for that host.

Cold Wallet and Online Node can be different OS. Data is compartible on OS level.

Please note, this online host will never have access to your wallet secrets.


# Setup

### Online Node Setup

1) Install mwc-qt-wallet from [Wallet Downlod](https://www.mwc.mw/downloads) page.

2) Press as 'Run as MWC Node' button

    ![](cold_wallet_images/start.png)

Press 'Continue'

3) After restart you should see:

    ![](cold_wallet_images/node_in_sync.png)

Until your node finish syncing, 'Save Blockchain Data' and 'Publish Transaction' will be disabled.

4) You can select Node network from Configuration menu.

    ![](cold_wallet_images/node_network.png)

5) Switch back to 'Node overview' and please wait until synchronization process is finished.

### Cold Wallet Setup

1) Follow the Ubuntu tutorial for creating a bootable USB stick Ubuntu instance: https://tutorials.ubuntu.com/tutorial/tutorial-create-a-usb-stick-on-ubuntu#0

2) Insert your second USB stick and ensure it is empty. If it is not delete any existing files on it.

3) Download mwc-qt-wallet from  [Wallet Downlod](https://www.mwc.mw/downloads) page and save it at your second USB stick.

4) Boot your USB stick with Ubuntu OS so that you are in Ubuntu Linux.

    IMPORTANT: Do not configure networking as that will compromize the cold storage nature of the setup.

5) Once you are in Ununtu, insert second USB stick with mwc-qt-wallet installer and install mwc-qt-wallet.

6) Start mwc-qt-wallet and follow directions. At your first run you will create a wallet with a new seed, 
or restore your wallet from existing passphrase. Please note, at this moment you rwallet is running as online wallet.
Because your host is offline, you will see many messages about that. Please ignore them until you switch to 'Cold Wallet' mode.

7) Switch to Cold Wallet mode:

    ![](cold_wallet_images/switch_to_cold_wallet.png)

8) Switch to 'Node Overiview' page. You should see that node is not able to collect, it has 0 blocks.
Now, we should move data from 'Online Node' at your online Host. 

# Transfer Node Data

1) At your 'Online Node' wait until synchronization will be finished and button "Save Blockchain Data" will be enabled. 
Press this button and save the blockchain at your USB drive.

2) Unplug USB drive from 'Online Node' and plug it in 'Cold Wallet'

3) At cold Wallet, at 'mwc node status' page press "Load Blockchain Data" and read that data from your USB stick.

    ![](cold_wallet_images/load_chain_data.png)

    You should be able to see that your node numbers are updated.

4) New you can veiw your transactions, output. The data will be updated with a new blockchin.

# Receive/Send MWC

Since the Cold Wallet is offline, you will be able to work with only file based transactions.

### Receive MWC

1) You probably receive file MWC transaction at one of your online PC/Laptops. You copy that file to your USB stick.
Initial transaction file should have an extention '.tx'

2) Plug in USB stick with a file transaction into your 'Cold Wallet'

3) It 'Receive' page please sign transaction.

    ![](cold_wallet_images/receive_mwc.png)

    Signed transaction file will be created at the same path. New file extention will be '.response'

4) Move signed transaction file to online host and send to your sender for finalization.

5) Wait until you get a note that your transaction expected to be finalized. 

6) To verify the fact that your transaction was finalized, 'Transfer Node Data' as explained above.

7) Now you 'Cold Wallet' has updated blockchain data that should contain output with your transaction.

8) Go to 'Transaction' page at 'Cold Wallet' and check that "Confirm" for your transaction is 'Yes'

9) Congratulations, you receive your MWC.

### Send MWC

1) With "Cold Wallet" initiate file base send transaction normally

    ![](cold_wallet_images/send_cold_wallet.png)

    Save transaction file to the USB stick.

2) From online host send Transaction file to the person whom you want to send MWC.

3) Wait for singed transaction file and save to USB stick.

4) Move that USB stick to "Cold Wallet".

5) Finalize transaction from "Finalize" page

    ![](cold_wallet_images/finalize1.png)

    ![](cold_wallet_images/finalize2.png)
    
    Please note **"Result location"** points to the resulted slate that need to be broadcasted to the network.
    
6) Save resulting 'mwctx' file to the USB drive and move it to "Online Node" host

7) At "Online Node", node status page press "Publish Transaction" button and select mwctx file.

    ![](cold_wallet_images/node_publish_trans.png) 
  
    You should see seccess message about that. If you see error message, it is likely that this transaction is already published to the network or 
    it has imvalid output.
      
8) Wait some time until transaction will be exepted by the network and few new blocks will be mained.

9) To verify the fact that your transaction was finalized sucessfully, 'Transfer Node Data' as explained above.

10) Now you 'Cold Wallet' has updated blockchain data that should contain output with your transaction.

11) Go to 'Transaction' page at 'Cold Wallet' and check that "Confirm" for your transaction is 'Yes'

12) Congratulations, you sent your MWC.
