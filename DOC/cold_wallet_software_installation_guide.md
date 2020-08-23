MWC Cold Wallet Software Installation Guide

Last Updated: Aug. 23, 2020

# Overview

A cold wallet is intended to keep your MWC safe by storing your MWC in offline storage. Once set up, your cold wallet should never be connected to the internet. This will prevent hackers from being able to access your wallet and steal your funds.

An MWC Cold Wallet runs on a USB drive which you boot to whenever you want to access your cold wallet. This guide is intended to help guide you through the process of initially installing the software needed to run your cold wallet and then later upgrading your software while keeping the cold wallet offline.

# Prerequisites

Prior to installing MWC Qt Wallet as a cold wallet, you will need:

* Bootable live Ubuntu USB stick

For the best cold wallet experience, it is recommended that you create a bootable live Ubuntu USB stick with persistent storage. Using a drive with persistent storage will allow you to set up and run your cold wallet using a single USB drive. The following link contains the procedure for creating this type of bootable Ubuntu USB drive depending upon your computer OS.

*  [Creating a USB Drive With Persistent Storage For MWC Cold Wallet](https://github.com/mwcproject/mwc-qt-wallet/blob/master/DOC/cold_wallet_persistent_usb_drive_creation.md)

# Initial Cold Wallet Software Installation

To initially install the software for your cold wallet, we will connect to the internet to make the installation of MWC Qt Wallet and its software dependencies easier. This will be the only time you ever connect your USB stick to the internet. 

## Boot Up Live Ubuntu USB Stick

To install the software needed to run an MWC Cold Wallet, you first need to boot your computer from your USB stick with Ubuntu installed on it. Depending upon your computer, you may need to change the boot order of the bios so that your USB is listed first in the boot sequence your computer follows. The following link contains an article which explains how to boot from a USB drive on Windows, Mac, and Linux:

* [How to Boot From USB Media](https://www.acronis.com/en-us/articles/usb-boot/)

When you boot from your USB drive, you will see whichever drives are connected to your system which can be booted from. You will also see an option to select a network to connect to. We will first connect to your network and then boot from the USB drive.

### Connect to Your Network

Click the up and down arrows in the Choose Network box to see a list of networks available. 

* Select your network and enter its passcode.

* Click on the checkmark

At this point you will return to the original screen which displays the bootable drives connected to your computer and that you are connected to your network.

Note: Booting from a wired network will always work. Depending upon the machine you are booting from, Ubuntu may or may not recognize your wireless device. Be sure to verify your network connection after booting up.

### Boot From USB Drive

To boot from your USB drive:

* Click on the drive named EFI Boot

* Click the up-arrow that appears under the EFI Boot drive

The Ubuntu boot process will begin. At the end of the process the Ubuntu Desktop will appear.

### Verify Network Connection

To verify that you are connected to your network, click the down arrow in the top right of the desktop menu bar.

* Select Settings->Network

* Verify that you have a wired or wireless connection to your network.

## Install mwc-qt-wallet onto USB Drive

Now that we have booted from our Ubuntu USB drive and connected to the network, we can download and install the files necessary to run an MWC Cold Wallet. This will be the only time you ever connect to the internet with this USB device. Once your cold wallet is set up, you should never run it while connected to a network.

Open the Software & Updates application and under Downloadable from the Internet:

* check Source Code

* Click ‘Close’

* Click ‘Reload’

Now update the Ubuntu repository and install Qt5 and ncurses which are needed by MWC Qt Wallet:

* sudo add-apt-repository universe

* sudo apt-get update

* sudo apt-get install qt5-default

* sudo apt-get install libncursesw5

Note: Depending upon the version of Ubuntu you are using, you may not need to add the universe repository or libncursesw5 as one or both may already be installed. This procedure has been tested with Ubuntu 20.04.1 and all of the steps above are required.

Download the latest version of mwc-qt-wallet: 

* Open Firefox

* Enter the URL: [https://www.mwc.mw/downloads](https://www.mwc.mw/downloads)

* Select the latest *standard* version of mwc-qt-wallet for Linux which ends with .deb and download it. For example, mwc-qt-wallet_1.0.22-linux64-standardcpu.deb.

By default, the downloaded file is stored in the Downloads directory under your home directory.

Install MWC Qt Wallet:

* Open the file browser (Files) and double click the Downloads folder

* Double click on the downloaded file icon of MWC Qt Wallet to cause the Ubuntu Software Install utility to launch

* Click Install

Alternatively you can install mwc-qt-wallet from the command line as follows:

* Open a terminal window (Enter: ctrl-alt-t)

* cd Downloads

* sudo apt install ./mwc-qt-wallet_\<version\>-linux64-standardcpu.deb

Where \<version\> is the version in the name of the .deb file that was previously downloaded.

## Shutdown Computer

Now that mwc-qt-wallet and all of its dependencies have been installed on your USB drive:

* Shutdown your computer

* Remove USB drive with mwc-qt-wallet installed on it

* Label the drive

## Finish Cold Wallet Setup

You are now ready to setup your cold wallet following the instructions in:

* [https://github.com/mwcproject/mwc-qt-wallet/blob/master/DOC/cold_wallet.md](https://github.com/mwcproject/mwc-qt-wallet/blob/master/DOC/cold_wallet.md)

However, you will not now need 2 USB drives to startup your wallet. You now only need to use the one we just created with mwc-qt-wallet on it. However, you will still need a second USB drive to transfer files between your online node and cold wallet.

Start with the section "Online Node Host".

When you come to the section "Cold Wallet Setup" you can skip steps 1 thru 5. Instead do the following:

* Boot from your USB drive but *DO NOT CONNECT TO YOUR NETWORK*

* Once you have booted from your USB drive, verify that you are *not* connected to your network. See the section above *Verify Network Connection* for information on how to see if your computer has any network connections. From this point on and whenever you use your cold wallet, you do not want any network connections.

Note: Some versions of Ubuntu (e.g. 18.04.5) will automatically reconnect to the network they have previously been connected to, even though you do not connect to a network during initial boot up.

Now continue to follow the directions in "Cold Wallet Setup" starting with step 6.

# Upgrade MWC Cold Wallet

## Prerequisites

To upgrade the mwc-qt-wallet software package for your cold wallet, you will need a computer with two available USB drives. One will be used to boot from the USB stick containing your cold wallet. The other will be used to transfer the latest MWC Qt Wallet installation file to your cold wallet.

## Download Latest MWC Qt Wallet Installation File

To upgrade the version of mwc-qt-wallet running on the USB drive with your cold wallet, you first need to copy the .deb installation file for the latest version of MWC Qt Wallet onto a separate USB drive to use for transferring the installation file to your cold wallet.

On a system connected to the network, do the following:

* Open a browser

* Enter the URL: [https://www.mwc.mw/downloads](https://www.mwc.mw/downloads)

* Select the latest *standard* version of mwc-qt-wallet for Linux which ends with .deb and download it. For example, mwc-qt-wallet_1.0.22-linux64-standardcpu.deb.

* Insert a USB drive that does not contain your cold wallet, and can be used for file transfers, into your computer.

* Copy the downloaded .deb file onto the inserted USB drive.

* Eject the USB drive and remove it from your computer.

## Uninstall mwc-qt-wallet Package

You will now uninstall the mwc-qt-wallet package on your cold wallet. Uninstalling mwc-qt-wallet will not affect your cold wallet contents. The uninstallation only removes the executables that are used to run the wallet.

mwc-qt-wallet can be uninstalled using either the Ubuntu Software Install utility or from the command line. However, since you will be installing new software without a network connection during the upgrade process, the new software will need to be installed from the command line since the Software Install utility needs a network connection to install software. Therefore the directions for uninstalling mwc-qt-wallet will focus on using the command line.

### Boot Without Network Connection

To uninstall mwc-qt-wallet you first need to boot from your cold wallet USB drive but do not connect to a network.

* Shutdown your computer

* Insert the USB drive with the MWC Cold Wallet

* Turn on your computer and initiate the boot sequence (See the section *Boot Up Live Ubuntu USB Stick* above).

* *Do Not* connect to a network

* Click on the drive named EFI Boot

* Click the up-arrow that appears under the EFI Boot drive

To ensure that you are not connected to your network, click the down arrow in the top right of the desktop menu bar.

* Select Settings->Network

* Verify that you do not have any wired or wireless network connections.

### Uninstall Using Command Line

To uninstall the mwc-qt-wallet package, follow these steps:

* Open a terminal window by pressing ctrl-alt-t

* Type the command: sudo apt-get remove mwc-qt-wallet 

## Install Latest MWC Qt Wallet Software

Now it’s time to install the latest version of MWC Qt Wallet from the .deb installation file on your other USB drive.

* Insert the USB drive with the MWC Qt Wallet .deb installation file into the computer booted from your cold wallet USB stick.

* Open a terminal window by pressing ctrl-alt-t

* Type the command:

    * cp /media/ubuntu/\<USB device name\>/mwc-qt-wallet_\<version\>-linux64-standardcpu.deb .

* Type the command:

    * sudo apt install ./mwc-qt-wallet_\<version\>-linux64-standardcpu.deb

* Enter ‘Y’ when prompted.

Where \<version\> is replaced with the actual version information from the name of the installation file.

When the installation finishes, you can now open your MWC Cold Wallet.

* Type the command: mwc-qt-wallet

##   

