The purpose of this page is to explain how to claim MWC from the Airdrop once the claims period has opened. First off,
you need the latest MWC qt wallet. The latest version can be obtained here:
https://github.com/mwcproject/mwc-qt-wallet/releases

# Before you start

Before you start ensure you have the ability to sign a message from the BTC address that you registered (i.e. have your hardware wallet ready or any tools you need).

  Checklist:
  
  ( ) Latest MWC QT Wallet installed (https://mwc.mw/downloads/)
  
  ( ) Your bitcoin signing wallet ready (e.g. Trezor, Ledger, Coinomi)
  
  ( ) A quiet and private spot to claim the airdrop without distractions

# Claiming Process

Once you have obtained the wallet you will notice the airdrop tab on the top left side.

![Airdrop Tab](https://raw.githubusercontent.com/mwcproject/mwc-qt-wallet/master/DOC/airdrop_tab.png "Airdrop Tab")

If you click it before the claims period is open, you will see the message, "Claims not open yet.". Once claims has opened, you will need to close the wallet and reopen it to be able to claim.

After claims are open, you will see something like this:

![Airdrop Enabled](https://raw.githubusercontent.com/mwcproject/mwc-qt-wallet/master/DOC/airdrop_enabled.png "Airdrop Enabled")

Note: the "BTC Address to claim" field and the "Airdrop Account Password" fields. These are the Bitcoin address you registered during the registration process and the password you set during the registration process. If you forgot your password, you can reset it on this page: https://wallet.mwc.mw/registration.html?v5. You will be prompted to sign an additional message to prove you control the address that is specified.

After correctly entering this information, you click "Request Airdrop", you will be taken to a page where you sign a message to prove control of the address. The same process used to sign your message for registering will be used on this page. Please note that in the section of the wallet that is labeled "Signed Message" you need to put the signature of the message that's provided. It will look something like this, "IBn2qPtqae9kS1c/qehsJXkOnjiK3rBUQ2ncRmLPwDCppfp3UooxJiev9vPNUuCtiGJo9+4Bw01v5fFFPEnTQgk="

Note: some wallets like the Ledger include the address in front of the signature. Please take out the address part of the signature. For instance if the signature looks like this:

-----BEGIN BITCOIN SIGNED MESSAGE-----
This is an example of a signed message.
-----BEGIN SIGNATURE-----
1HZwkjkeaoZfTSaJxDw6aKkxp45agDiEzN
HNWsH/EpIj40CcJwK0o9Mndbx6ZNqi5uLWA9XOv2sT+9qpopn4wfqG99cIwQ/ws0XSAX8vdBiRle5PtrOshEmgo=
-----END BITCOIN SIGNED MESSAGE-----

You should submit "HNWsH/EpIj40CcJwK0o9Mndbx6ZNqi5uLWA9XOv2sT+9qpopn4wfqG99cIwQ/ws0XSAX8vdBiRle5PtrOshEmgo="

![Claim](https://raw.githubusercontent.com/mwcproject/mwc-qt-wallet/master/DOC/claim.png "Claim")

At this point, if successful you will see a message in the Airdrop tab that looks like the one below:

![Airdrop Awaiting](https://raw.githubusercontent.com/mwcproject/mwc-qt-wallet/master/DOC/awaiting_finalization2.png
 "Airdrop Awaiting")

Please note that it may take up to 48 hours to finalize your airdrop claim and your funds will not be available until it is finalized. Also, note that to see the full message, you may need to resize the text area and the wallet. Also note that your wallet does NOT need to be online during this time period.

Once your claim is finalized, your funds will be available and you should see a status message like the following:


![Claims Complete](https://raw.githubusercontent.com/mwcproject/mwc-qt-wallet/master/DOC/complete.png
 "Claims complete")

Congratulations! Stay tuned for details regarding the MWC HODL program which rewards hodlers with additional MWC.
